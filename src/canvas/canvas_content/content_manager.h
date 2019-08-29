// canvas/canvas_content/content_manager.h

#ifndef CONTENT_MANAGER_H
#define CONTENT_MANAGER_H

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#include <vector>
#include <string>
#include "content_interface.h"

namespace Beamertool {

    class ContentManager {

    public:
        /**
         * standard constructor
         * @param display   pointer to the egl display
         * @param surface   pointer to the egl surface
         * @param context   pointer to the egl context
         */
        ContentManager(EGLDisplay* display, EGLSurface* surface, EGLContext* context);

        /**
         * destructor
         */
        ~ContentManager();

        /**
         * get Content Ratio
         * @param content_type  content type string
         * @param content_data  content data
         * @param ratio         the ratio is set in here (if content type exists)
         */
        void getContentRatio(std::string content_type, int content_data, float * ratio);

        /**
         * set Content for Open GL
         * @param conten_type   content plugin name
         * @param conten_data   content data
         */
        void setContent(std::string content_type, int content_data);

    private:
        /**
         * load plugins (real plugin interface not implemented)
         */
        void loadContentPlugins();

        /**
         * add a plugin to the plugin list and its name to content type list
         * @param new_plugin    the new plugin
         */
        void addPlugin(ContentInterface * new_plugin);

        /**
         * get the id of an content Plugin
         * @param type_name the content type name
         * @return id of the content plugin to type_name, -1 if type has no plugin
         */
        int getPluginId(std::string type_name);

        std::vector<std::string> content_types;         // content type list
        std::vector<ContentInterface*> content_plugins; // plugin pointer list
        EGLDisplay* display;                            // egl display pointer
        EGLSurface* surface;                            // egl surface pointer
        EGLContext* context;                            // egl context pointer
    };
}

#endif // CONTENT_MANAGER_H
