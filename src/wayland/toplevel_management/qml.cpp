#include "qml.hpp"

#include <qobject.h>
#include <qtmetamacros.h>

#include "../../core/model.hpp"
#include "../../core/proxywindow.hpp"
#include "../../core/qmlscreen.hpp"
#include "../../core/windowinterface.hpp"
#include "handle.hpp"
#include "manager.hpp"

namespace qs::wayland::toplevel_management {

Toplevel::Toplevel(impl::ToplevelHandle* handle, QObject* parent): QObject(parent), handle(handle) {
	// clang-format off
	QObject::connect(handle, &impl::ToplevelHandle::closed, this, &Toplevel::onClosed);
	QObject::connect(handle, &impl::ToplevelHandle::appIdChanged, this, &Toplevel::appIdChanged);
	QObject::connect(handle, &impl::ToplevelHandle::titleChanged, this, &Toplevel::titleChanged);
	QObject::connect(handle, &impl::ToplevelHandle::parentChanged, this, &Toplevel::parentChanged);
	QObject::connect(handle, &impl::ToplevelHandle::activatedChanged, this, &Toplevel::activatedChanged);
	QObject::connect(handle, &impl::ToplevelHandle::maximizedChanged, this, &Toplevel::maximizedChanged);
	QObject::connect(handle, &impl::ToplevelHandle::minimizedChanged, this, &Toplevel::minimizedChanged);
	QObject::connect(handle, &impl::ToplevelHandle::fullscreenChanged, this, &Toplevel::fullscreenChanged);
	// clang-format on
}

void Toplevel::onClosed() {
	emit this->closed();
	delete this;
}

void Toplevel::activate() { this->handle->activate(); }

QString Toplevel::appId() const { return this->handle->appId(); }
QString Toplevel::title() const { return this->handle->title(); }

Toplevel* Toplevel::parent() const {
	return ToplevelManager::instance()->forImpl(this->handle->parent());
}

bool Toplevel::activated() const { return this->handle->activated(); }

bool Toplevel::maximized() const { return this->handle->maximized(); }
void Toplevel::setMaximized(bool maximized) { this->handle->setMaximized(maximized); }

bool Toplevel::minimized() const { return this->handle->minimized(); }
void Toplevel::setMinimized(bool minimized) { this->handle->setMinimized(minimized); }

bool Toplevel::fullscreen() const { return this->handle->fullscreen(); }
void Toplevel::setFullscreen(bool fullscreen) { this->handle->setFullscreen(fullscreen); }

void Toplevel::fullscreenOn(QuickshellScreenInfo* screen) {
	auto* qscreen = screen != nullptr ? screen->screen : nullptr;
	this->handle->fullscreenOn(qscreen);
}

void Toplevel::setRectangle(QObject* window, QRect rect) {
	auto* proxyWindow = qobject_cast<ProxyWindowBase*>(window);

	if (proxyWindow == nullptr) {
		if (auto* iface = qobject_cast<WindowInterface*>(window)) {
			proxyWindow = iface->proxyWindow();
		}
	}

	if (proxyWindow != this->rectWindow) {
		if (this->rectWindow != nullptr) {
			QObject::disconnect(this->rectWindow, nullptr, this, nullptr);
		}

		this->rectWindow = proxyWindow;

		if (proxyWindow != nullptr) {
			QObject::connect(
			    proxyWindow,
			    &QObject::destroyed,
			    this,
			    &Toplevel::onRectangleProxyDestroyed
			);

			QObject::connect(
			    proxyWindow,
			    &ProxyWindowBase::windowConnected,
			    this,
			    &Toplevel::onRectangleProxyConnected
			);
		}
	}

	this->rectangle = rect;
	this->handle->setRectangle(proxyWindow->backingWindow(), rect);
}

void Toplevel::unsetRectangle() { this->setRectangle(nullptr, QRect()); }

void Toplevel::onRectangleProxyConnected() {
	this->handle->setRectangle(this->rectWindow->backingWindow(), this->rectangle);
}

void Toplevel::onRectangleProxyDestroyed() {
	this->rectWindow = nullptr;
	this->rectangle = QRect();
}

ToplevelManager::ToplevelManager() {
	auto* manager = impl::ToplevelManager::instance();

	QObject::connect(
	    manager,
	    &impl::ToplevelManager::toplevelReady,
	    this,
	    &ToplevelManager::onToplevelReady
	);

	for (auto* handle: manager->readyToplevels()) {
		this->onToplevelReady(handle);
	}
}

Toplevel* ToplevelManager::forImpl(impl::ToplevelHandle* impl) const {
	if (impl == nullptr) return nullptr;

	for (auto* toplevel: this->mToplevels.valueList()) {
		if (toplevel->handle == impl) return toplevel;
	}

	return nullptr;
}

ObjectModel<Toplevel>* ToplevelManager::toplevels() { return &this->mToplevels; }

void ToplevelManager::onToplevelReady(impl::ToplevelHandle* handle) {
	auto* toplevel = new Toplevel(handle, this);
	QObject::connect(toplevel, &Toplevel::closed, this, &ToplevelManager::onToplevelClosed);
	this->mToplevels.insertObject(toplevel);
}

void ToplevelManager::onToplevelClosed() {
	auto* toplevel = qobject_cast<Toplevel*>(this->sender());
	this->mToplevels.removeObject(toplevel);
}

ToplevelManager* ToplevelManager::instance() {
	static auto* instance = new ToplevelManager(); // NOLINT
	return instance;
}

ObjectModel<Toplevel>* ToplevelManagerQml::toplevels() {
	return ToplevelManager::instance()->toplevels();
}

} // namespace qs::wayland::toplevel_management
