// canvas/renderer.h

#ifndef RENDERER_H
#define RENDERER_H

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <vector>
#include "canvas_classes.h"
#include "canvas_content/content_manager.h"

namespace Beamertool {

    class Renderer {

    public:
        /**
         * standard constructor
         */
        Renderer();

        /**
         * destructor
         */
        ~Renderer();

        /**
         * get screen width
         * @return screen width
         */
        int getScreenWidth();

        /**
         * get screen height
         * @return screen height
         */
        int getScreenHeight();

        /**
         * get screen ratio
         * @return screen ratio
         */
        float getScreenRatio();

        /**
         * draw all Canvases
         * @param canvases the canvas list
         */
        void drawCanvases(std::vector<Canvas*> canvases);

        /**
         * set Zoom
         * @param type set zoom type
         */
        void setZoom(int type);

        /**
         * trim an angle to range [0, 360)
         * @param angle to trim
         * @return angle in range [0, 360)
         */
        float correctAngleRange(float angle);

    private:
        /**
         * init OpenGL
         */
        void initOpenGL();

        /**
         * init projetion
         */
        void initProjection();

        ContentManager* content_manager;    // the content manager
        EGLDisplay display;                 // EGL Display
        EGLSurface surface;                 // EGL Surface
        EGLContext context;                 // EGL context
        uint32_t screen_width;              // screen width
        uint32_t screen_height;             // screen height
        float screen_ratio;                 // screen ratio
        GLfloat quadx[4*3];                 // the quad to draw a canvas
        GLfloat color[4*4];                 // the color to draw a canvas
        int zoom_type;                      // zooming type

    };
}

#endif // RENDERER_H
