// canvas/canvas_content/content_manager.cpp

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <vector>
#include <string>
#include "content_interface.h"
#include "content_manager.h"
// TODO implement a plugin interface
#include "content_example_plugin/content_example_plugin.h"
#include "content_local_file_texture/content_local_file_texture.h"

using namespace std;
using namespace Beamertool;

ContentManager::ContentManager(EGLDisplay* display, EGLSurface* surface, EGLContext* context) {
    this->display = display;
    this->surface = surface;
    this->context = context;
    this->content_types.clear();
    this->content_plugins.clear();

    loadContentPlugins();
}

ContentManager::~ContentManager() {
    for (unsigned int i=0; i < this->content_plugins.size(); ++i) {
        delete this->content_plugins[i];
    }
    this->content_plugins.clear();
    this->content_types.clear();
}

void ContentManager::loadContentPlugins() {

    // TODO: implemen plugin system

    // Load local example plugin
    ContentInterface* example_plugin = new ContentExamplePlugin(this->display, this->surface, this->context);
    addPlugin(example_plugin);

    // Load local file texture Plugin
    ContentInterface* local_file_texture = new ContentLocalFileTexture(this->display, this->surface, this->context);
    addPlugin(local_file_texture);
}

void ContentManager::addPlugin(ContentInterface* new_plugin) {
    this->content_plugins.push_back(new_plugin);
    this->content_types.push_back(new_plugin->getContentTypeName());
}

int ContentManager::getPluginId(string type_name) {
    for (unsigned int i=0; i < this->content_types.size(); ++i) {
        if (this->content_types[i].compare(type_name) == 0) {
            return i;
        }
    }
    return -1;
}

void ContentManager::getContentRatio(std::string content_type, int content_data, float * ratio) {
    int plugin_id = this->getPluginId(content_type);
    if (plugin_id >= 0) {
        this->content_plugins[plugin_id]->getContentRatio(content_data, ratio);
    }
}

void ContentManager::setContent(std::string content_type, int content_data) {
    int plugin_id = this->getPluginId(content_type);
    if (plugin_id >= 0) {
        this->content_plugins[plugin_id]->setContent(content_data);
    }
}
