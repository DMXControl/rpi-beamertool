// canvas/renderer.cpp

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <cmath>
#include <cassert>
#include <vector>
#include "canvas.h"
#include "canvas_content/content_manager.h"
#include "renderer.h"

using namespace std;
using namespace Beamertool;

Renderer::Renderer() {
    // init quad
    this->quadx[0] = -1.f;
    this->quadx[1] = -1.f;
    this->quadx[2] =  0.f;
    this->quadx[3] =  1.f;
    this->quadx[4] = -1.f;
    this->quadx[5] =  0.f;
    this->quadx[6] = -1.f;
    this->quadx[7] =  1.f;
    this->quadx[8] =  0.f;
    this->quadx[9] =  1.f;
    this->quadx[10] =  1.f;
    this->quadx[11] =  0.f;

    // init color
    this->color[0] =  1.f;
    this->color[1] =  1.f;
    this->color[2] =  1.f;
    this->color[3] =  1.f;
    this->color[4] =  1.f;
    this->color[5] =  1.f;
    this->color[6] =  1.f;
    this->color[7] =  1.f;
    this->color[8] =  1.f;
    this->color[9] =  1.f;
    this->color[10] =  1.f;
    this->color[11] =  1.f;
    this->color[12] =  1.f;
    this->color[13] =  1.f;
    this->color[14] =  1.f;
    this->color[15] =  1.f;

    this->zoom_type = 1;

    bcm_host_init();
    initOpenGL();
    initProjection();

    this->content_manager = new ContentManager(&this->display, &this->surface, &this->context);
}

Renderer::~Renderer() {
    // clear screen
    glClear( GL_COLOR_BUFFER_BIT );
    eglSwapBuffers(this->display, this->surface);

    delete this->content_manager;

    // Release OpenGL resources
    eglMakeCurrent( this->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    eglDestroySurface( this->display, this->surface );
    eglDestroyContext( this->display, this->context );
    eglTerminate( this->display );
}

void Renderer::initOpenGL() {

    int32_t success = 0;
    EGLBoolean result;
    EGLint num_config;

    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    VC_DISPMANX_ALPHA_T alpha;
    alpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
    alpha.opacity = 255;
    alpha.mask = 0;

    static const EGLint attribute_list[] =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };

    EGLConfig config;

    // get an EGL display connection
    this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(this->display!=EGL_NO_DISPLAY);

    // initialize the EGL display connection
    result = eglInitialize(this->display, NULL, NULL);
    assert(EGL_FALSE != result);

    // get an appropriate EGL frame buffer configuration
    result = eglChooseConfig(this->display, attribute_list, &config, 1, &num_config);
    //result = eglSaneChooseConfigBRCM(this->display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);

    // create an EGL rendering context
    this->context = eglCreateContext(this->display, config, EGL_NO_CONTEXT, NULL);
    assert(this->context!=EGL_NO_CONTEXT);

    // create an EGL window surface
    success = graphics_get_display_size(0 /* LCD */, &this->screen_width, &this->screen_height);
    assert( success >= 0 );
    this->screen_ratio = ((float)this->screen_width) / ((float)this->screen_height);

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = this->screen_width;
    dst_rect.height = this->screen_height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = this->screen_width << 16;
    src_rect.height = this->screen_height << 16;

    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
    dispman_update = vc_dispmanx_update_start( 0 );

    dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
        0/*layer*/, &dst_rect, 0/*src*/,
        &src_rect, DISPMANX_PROTECTION_NONE, &alpha /*alpha*/, 0/*clamp*/, DISPMANX_NO_ROTATE/*transform*/);

    nativewindow.element = dispman_element;
    nativewindow.width = this->screen_width;
    nativewindow.height = this->screen_height;
    vc_dispmanx_update_submit_sync( dispman_update );

    this->surface = eglCreateWindowSurface( this->display, config, &nativewindow, NULL );
    assert(this->surface != EGL_NO_SURFACE);

    // connect the context to the surface
    result = eglMakeCurrent(this->display, this->surface, this->surface, this->context);
    assert(EGL_FALSE != result);

    // Set background color and clear buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT );
    glClear( GL_DEPTH_BUFFER_BIT );
    glShadeModel(GL_FLAT);

    // Enable back face culling.
    //glEnable(GL_CULL_FACE);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// obere Ebene überdeckt darunterliegende, je nach transparenz darunterliegende sichtbar
    //glBlendFunc (GL_SRC_ALPHA, GL_ONE);				// additive Farbmischung wie Licht, also wenn rot grün überlagert wird gelb
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_MODELVIEW);
}

void Renderer::initProjection() {
    GLfloat left   = -1.f;
    GLfloat right  =  1.f;
    GLfloat top    =  1.f / this->screen_ratio;
    GLfloat bottom = -1.f / this->screen_ratio;
    GLfloat near   = -1.f;
    GLfloat far    =  1.f;

    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST ); // da glOrtho keine Korrektur benötigt

    glViewport(0, 0, (GLsizei)this->screen_width, (GLsizei)this->screen_height);

    // Texturen richtig ausrichten
    //glMatrixMode(GL_TEXTURE);
    //glRotatef(-90.f, 0.f, 0.f, 1.f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //glOrtho
    GLfloat ortho_matrix[16] = {
        (2.f / (right - left)), 0.f, 0.f, 0.f,
        0.f, (2.f / (top - bottom)), 0.f, 0.f,
        0.f, 0.f, (-2.f / (far - near)), 0.f,
        (-1 * (right + left) / (right - left)), (-1 * (top + bottom) / (top - bottom)), (-1 * (far + near) / (far - near)), 1.f,
    };
    glMultMatrixf(ortho_matrix);

    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 0, this->quadx );

    glEnableClientState( GL_COLOR_ARRAY );
    glColorPointer(4, GL_FLOAT, 0, this->color);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Renderer::drawCanvases(vector<Canvas*> canvases) {

    glClear( GL_COLOR_BUFFER_BIT );
    glMatrixMode(GL_MODELVIEW);

    for(int i=0; i < (int)canvases.size(); i++) {

        for (int j=0;j<4;j++) {
            this->color[4*j  ] = canvases[i]->getR();
            this->color[4*j+1] = canvases[i]->getG();
            this->color[4*j+2] = canvases[i]->getB();
            this->color[4*j+3] = canvases[i]->getAlpha();
        }

        float scal_x = canvases[i]->getScalingX();
        float scal_y = canvases[i]->getScalingY();
        if (scal_y < 0) {
            float content_ratio = 1.0f;
            this->content_manager->getContentRatio(canvases[i]->getContentType(), canvases[i]->getContentData(), &content_ratio);
            scal_y = scal_x * content_ratio;
        }
        this->quadx[ 0] = -scal_x;
        this->quadx[ 1] = -scal_y;
        this->quadx[ 2] = 0.f;
        this->quadx[ 3] = scal_x;
        this->quadx[ 4] = -scal_y;
        this->quadx[ 5] = 0.f;
        this->quadx[ 6] = -scal_x;
        this->quadx[ 7] = scal_y;
        this->quadx[ 8] = 0.f;
        this->quadx[ 9] = scal_x;
        this->quadx[10] = scal_y;
        this->quadx[11] = 0.f;

        this->content_manager->setContent(canvases[i]->getContentType(), canvases[i]->getContentData());

        glPushMatrix();

        if(canvases[i]->getShutterState() == 0) {
            if(canvases[i]->getNumber() == 0) {
                glTranslatef(canvases[i]->getPositionX(), -canvases[i]->getPositionY(), 0);
                glRotatef(canvases[i]->getRotation(), 0.f, 0.f, 1.f );
                glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
            } else {
                // Schleife läuft rückwärs, dass Zeichenreihenfolge (damit Überlagerung), wie im Original Beamertool
                for(int j=canvases[i]->getNumber()-1; j>=0; j--) {
                    glPushMatrix();

                    glTranslatef(canvases[i]->getPositionX(), -canvases[i]->getPositionY(), 0);	// zum mittelpunkt
                    if (this->zoom_type == 1) {
                        float angle = canvases[i]->getRotationAll() * M_PI / 180.0f + (2.0f * M_PI / (float) canvases[i]->getNumber()) * (float) j;
                        float trans_x = -1.0f * (float)cos(angle) * canvases[i]->getZoom() * 2.0f;
                        float trans_y = -1.0f * (float)sin(angle) * canvases[i]->getZoom() * 2.0f / this->screen_ratio;
                        glTranslatef(trans_x, trans_y, 0);
                    } else {
                        glRotatef(correctAngleRange(canvases[i]->getRotationAll() + (float) j * 360.0f / (float) canvases[i]->getNumber()), 0.f, 0.f, 1.f); // Drehe an position
                        glTranslatef(-canvases[i]->getZoom(), 0, 0); // gehe auf Kreis
                        glRotatef(correctAngleRange( -(canvases[i]->getRotationAll() + (float) j * 360.0f / (float) canvases[i]->getNumber())) , 0.f, 0.f, 1.f); // Drehe zurück
                    }
                    glRotatef(canvases[i]->getRotation(), 0.f, 0.f, 1.f );
                    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

                    glPopMatrix();
                }
            }
        }

        glPopMatrix();
    }

    eglSwapBuffers(this->display, this->surface);
}

int Renderer::getScreenWidth() {
    return this->screen_width;
}

int Renderer::getScreenHeight() {
    return this->screen_height;
}

float Renderer::getScreenRatio() {
    return this->screen_ratio;
}

void Renderer::setZoom(int type) {
    if (type == 1 || type == 2) {
        this->zoom_type = type;
    } else {
        this->zoom_type = 1;
    }
}

float Renderer::correctAngleRange(float angle) {
    while(angle >= 360.f) {
        angle -= 360.f;
    }
    while(angle < 0.f) {
        angle += 360.f;
    }
    return angle;
}
