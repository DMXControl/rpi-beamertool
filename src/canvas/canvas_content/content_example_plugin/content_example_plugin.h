// canvas/canvas_content/content_example_plugin/content_example_plugin.h

#ifndef CONTENT_EXAMPLE_PLUGIN_H
#define CONTENT_EXAMPLE_PLUGIN_H

#include "../content_interface.h"

namespace Beamertool {

    class ContentExamplePlugin : public ContentInterface {

    public:
        ContentExamplePlugin(EGLDisplay* d, EGLSurface* s, EGLContext* c);
        ~ContentExamplePlugin();
        std::string getContentTypeName();
        void getContentRatio(int content_data, float * ratio);
        void setContent(int content_data);

    private:
        GLfloat texCoords[4*2];
        GLuint texture;
        int size_x;
        int size_y;
    };
}

#endif // CONTENT_EXAMPLE_PLUGIN_H
