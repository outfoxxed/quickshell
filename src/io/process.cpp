#include "process.hpp"
#include <csignal> // NOLINT
#include <utility>

#include <qlist.h>
#include <qlogging.h>
#include <qobject.h>
#include <qprocess.h>
#include <qtmetamacros.h>
#include <qtypes.h>

#include "datastream.hpp"

bool Process::isRunning() const { return this->process != nullptr; }

void Process::setRunning(bool running) {
	this->targetRunning = running;
	if (running) this->startProcessIfReady();
	else if (this->isRunning()) this->process->terminate();
}

QVariant Process::pid() const {
	if (this->process == nullptr) return QVariant();
	return QVariant::fromValue(this->process->processId());
}

QList<QString> Process::command() const { return this->mCommand; }

void Process::setCommand(QList<QString> command) {
	if (this->mCommand == command) return;
	this->mCommand = std::move(command);
	emit this->commandChanged();

	this->startProcessIfReady();
}

DataStreamParser* Process::stdoutParser() const { return this->mStdoutParser; }

void Process::setStdoutParser(DataStreamParser* parser) {
	if (parser == this->mStdoutParser) return;

	if (this->mStdoutParser != nullptr) {
		QObject::disconnect(this->mStdoutParser, nullptr, this, nullptr);
	}

	this->mStdoutParser = parser;

	if (parser != nullptr) {
		QObject::connect(parser, &QObject::destroyed, this, &Process::onStdoutParserDestroyed);
	}

	emit this->stdoutParserChanged();

	if (parser != nullptr && !this->stdoutBuffer.isEmpty()) {
		parser->parseBytes(this->stdoutBuffer, this->stdoutBuffer);
	}
}

void Process::onStdoutParserDestroyed() {
	this->mStdoutParser = nullptr;
	emit this->stdoutParserChanged();
}

DataStreamParser* Process::stderrParser() const { return this->mStderrParser; }

void Process::setStderrParser(DataStreamParser* parser) {
	if (parser == this->mStderrParser) return;

	if (this->mStderrParser != nullptr) {
		QObject::disconnect(this->mStderrParser, nullptr, this, nullptr);
	}

	this->mStderrParser = parser;

	if (parser != nullptr) {
		QObject::connect(parser, &QObject::destroyed, this, &Process::onStderrParserDestroyed);
	}

	emit this->stderrParserChanged();

	if (parser != nullptr && !this->stderrBuffer.isEmpty()) {
		parser->parseBytes(this->stderrBuffer, this->stderrBuffer);
	}
}

void Process::onStderrParserDestroyed() {
	this->mStderrParser = nullptr;
	emit this->stderrParserChanged();
}

void Process::startProcessIfReady() {
	if (this->process != nullptr || !this->targetRunning || this->mCommand.isEmpty()) return;
	this->targetRunning = false;

	auto& cmd = this->mCommand.first();
	auto args = this->mCommand.sliced(1);

	this->process = new QProcess(this);

	// clang-format off
	QObject::connect(this->process, &QProcess::started, this, &Process::onStarted);
	QObject::connect(this->process, &QProcess::finished, this, &Process::onFinished);
	QObject::connect(this->process, &QProcess::errorOccurred, this, &Process::onErrorOccurred);
	QObject::connect(this->process, &QProcess::readyReadStandardOutput, this, &Process::onStdoutReadyRead);
	QObject::connect(this->process, &QProcess::readyReadStandardError, this, &Process::onStderrReadyRead);
	// clang-format on

	this->stdoutBuffer.clear();
	this->stderrBuffer.clear();

	this->process->start(cmd, args);
}

void Process::onStarted() {
	emit this->started();
	emit this->runningChanged();
}

void Process::onFinished(qint32 exitCode, QProcess::ExitStatus exitStatus) {
	this->process->deleteLater();
	this->process = nullptr;
	this->stdoutBuffer.clear();
	this->stderrBuffer.clear();

	emit this->exited(exitCode, exitStatus);
	emit this->runningChanged();
}

void Process::onErrorOccurred(QProcess::ProcessError error) {
	if (error == QProcess::FailedToStart) { // other cases should be covered by other events
		qWarning() << "Process failed to start, likely because the binary could not be found. Command:"
		           << this->mCommand;
		this->process->deleteLater();
		this->process = nullptr;
		emit this->runningChanged();
	}
}

void Process::onStdoutReadyRead() {
	if (this->mStdoutParser == nullptr) return;
	auto buf = this->process->readAllStandardOutput();
	this->mStdoutParser->parseBytes(buf, this->stdoutBuffer);
}

void Process::onStderrReadyRead() {
	if (this->mStderrParser == nullptr) return;
	auto buf = this->process->readAllStandardError();
	this->mStderrParser->parseBytes(buf, this->stderrBuffer);
}

void Process::signal(qint32 signal) {
	if (this->process == nullptr) return;
	kill(static_cast<qint32>(this->process->processId()), signal); // NOLINT
}

void Process::write(const QString& data) {
	if (this->process == nullptr) return;
	this->process->write(data.toUtf8());
}