#include "plugin.hpp"
#include <algorithm>

#include <qvector.h> // NOLINT (what??)

#include "generation.hpp"

static QVector<QuickshellPlugin*> plugins; // NOLINT

void QuickshellPlugin::registerPlugin(QuickshellPlugin& plugin) { plugins.push_back(&plugin); }

void QuickshellPlugin::initPlugins() {
	plugins.erase(
	    std::remove_if(
	        plugins.begin(),
	        plugins.end(),
	        [](QuickshellPlugin* plugin) { return !plugin->applies(); }
	    ),
	    plugins.end()
	);

	for (QuickshellPlugin* plugin: plugins) {
		plugin->init();
	}

	for (QuickshellPlugin* plugin: plugins) {
		plugin->registerTypes();
	}
}

void QuickshellPlugin::runConstructGeneration(EngineGeneration& generation) {
	for (QuickshellPlugin* plugin: plugins) {
		plugin->constructGeneration(generation);
	}
}

void QuickshellPlugin::runOnReload() {
	for (QuickshellPlugin* plugin: plugins) {
		plugin->onReload();
	}
}
