// canvas/canvas_content/content_local_file_texture/video_player.h

#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
extern "C" {
#include "ilclient.h"
}
#include <thread>

#define VIDEOPLAYER_NUM 3

namespace Beamertool {

    class VideoPlayer {

    public:
        VideoPlayer(EGLDisplay* d, EGLSurface* s, EGLContext* c);
        ~VideoPlayer();
        int startFreePlayer(std::string filename, GLuint * video_tex_ptr);
        void stopPlayer(int player_id);
        void sendStopSignal();

    private:

        bool player_is_free[VIDEOPLAYER_NUM];

        EGLDisplay display;
        EGLSurface surface;
        EGLContext context;

        GLuint vTex1;
        GLuint vTex2;
        GLuint vTex3;
        static void* eglImage1;
        static void* eglImage2;
        static void* eglImage3;
        static bool video_dec_loop1;
        static bool video_dec_loop2;
        static bool video_dec_loop3;
        static OMX_BUFFERHEADERTYPE* eglBuffer1;
        static OMX_BUFFERHEADERTYPE* eglBuffer2;
        static OMX_BUFFERHEADERTYPE* eglBuffer3;
        static COMPONENT_T* egl_render1;
        static COMPONENT_T* egl_render2;
        static COMPONENT_T* egl_render3;
        std::thread player_thread_1;
        std::thread player_thread_2;
        std::thread player_thread_3;
        std::string filename1;
        std::string filename2;
        std::string filename3;

        static void my_fill_buffer_done1(void* data, COMPONENT_T* comp);
        static void my_fill_buffer_done2(void* data, COMPONENT_T* comp);
        static void my_fill_buffer_done3(void* data, COMPONENT_T* comp);
        void video_decode_test1();
        void video_decode_test2();
        void video_decode_test3();

    };
}

#endif // VIDEO_PLAYER_H
