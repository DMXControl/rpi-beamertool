// canvas/canvas_content/content_local_file_texture/video_player.cpp

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
extern "C" {
#include "ilclient.h"
}
#include <thread>
#include "video_player.h"

using namespace std;
using namespace Beamertool;

void* VideoPlayer::eglImage1 = 0;
void* VideoPlayer::eglImage2 = 0;
void* VideoPlayer::eglImage3 = 0;
OMX_BUFFERHEADERTYPE* VideoPlayer::eglBuffer1 = NULL;
OMX_BUFFERHEADERTYPE* VideoPlayer::eglBuffer2 = NULL;
OMX_BUFFERHEADERTYPE* VideoPlayer::eglBuffer3 = NULL;
COMPONENT_T* VideoPlayer::egl_render1 = NULL;
COMPONENT_T* VideoPlayer::egl_render2 = NULL;
COMPONENT_T* VideoPlayer::egl_render3 = NULL;
bool VideoPlayer::video_dec_loop1 = true;
bool VideoPlayer::video_dec_loop2 = true;
bool VideoPlayer::video_dec_loop3 = true;

VideoPlayer::VideoPlayer(EGLDisplay* d, EGLSurface* s, EGLContext* c) {
    this->display = (*d);
    this->surface = (*s);
    this->context = (*c);

    for (int i=0; i<VIDEOPLAYER_NUM; ++i) {
        this->player_is_free[i] = true;
    }
}

VideoPlayer::~VideoPlayer() {

}

int VideoPlayer::startFreePlayer(std::string filename, GLuint * video_tex_ptr) {

    int free_player = 0;
    for (int i=0; i<VIDEOPLAYER_NUM; ++i) {
        if (this->player_is_free[i]) {
            this->player_is_free[i] = false;
            free_player = i+1;
            break;
        }
    }

    if(free_player > 0) {
        if (free_player == 1) {
            this->eglImage1 = eglCreateImageKHR(this->display, this->context, EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)(*video_tex_ptr), 0);
            if (this->eglImage1 == EGL_NO_IMAGE_KHR) {
                printf("eglCreateImageKHR1 failed.\n");
                exit(1);
            }
            this->filename1 = filename;
            player_thread_1 = thread(&VideoPlayer::video_decode_test1, this);
        } else if (free_player == 2) {
            this->eglImage2 = eglCreateImageKHR(this->display, this->context, EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)(*video_tex_ptr), 0);
            if (this->eglImage2 == EGL_NO_IMAGE_KHR) {
                printf("eglCreateImageKHR2 failed.\n");
                exit(1);
            }
            this->filename2 = filename;
            player_thread_2 = thread(&VideoPlayer::video_decode_test2, this);
        } else if (free_player == 3) {
            this->eglImage3 = eglCreateImageKHR(this->display, this->context, EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)(*video_tex_ptr), 0);
            if (this->eglImage3 == EGL_NO_IMAGE_KHR) {
                printf("eglCreateImageKHR3 failed.\n");
                exit(1);
            }
            this->filename3 = filename;
            player_thread_3 = thread(&VideoPlayer::video_decode_test3, this);
        }
    }

    return free_player;
}

void VideoPlayer::stopPlayer(int player_id) {
    if (player_id == 1) {
        video_dec_loop1 = false;
        player_thread_1.join();
    } else if (player_id == 2) {
        video_dec_loop2 = false;
        player_thread_2.join();
    } else if (player_id == 3) {
        video_dec_loop3 = false;
        player_thread_3.join();
    }
}

void VideoPlayer::sendStopSignal() {
    video_dec_loop1 = false;
    video_dec_loop2 = false;
    video_dec_loop3 = false;
}

void VideoPlayer::my_fill_buffer_done1(void* data, COMPONENT_T* comp) {
    VideoPlayer *me = static_cast<VideoPlayer*>(data);
    if (OMX_FillThisBuffer(ilclient_get_handle(me->egl_render1), me->eglBuffer1) != OMX_ErrorNone) {
        printf("OMX_FillThisBuffer failed in callback\n");
        exit(1);
    }
}

void VideoPlayer::my_fill_buffer_done2(void* data, COMPONENT_T* comp) {
    VideoPlayer *me = static_cast<VideoPlayer*>(data);
    if (OMX_FillThisBuffer(ilclient_get_handle(me->egl_render2), me->eglBuffer2) != OMX_ErrorNone) {
        printf("OMX_FillThisBuffer failed in callback\n");
        exit(1);
    }
}

void VideoPlayer::my_fill_buffer_done3(void* data, COMPONENT_T* comp) {
    VideoPlayer *me = static_cast<VideoPlayer*>(data);
    if (OMX_FillThisBuffer(ilclient_get_handle(me->egl_render3), me->eglBuffer3) != OMX_ErrorNone) {
        printf("OMX_FillThisBuffer failed in callback\n");
        exit(1);
    }
}

void VideoPlayer::video_decode_test1() {
   //const char* filename = "/opt/vc/src/hello_pi/hello_video/test.h264";
   const char* filename = this->filename1.c_str();

   if (eglImage1 == 0)
   {
      printf("eglImage1 is null.\n");
      exit(1);
   }

   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
   COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *clock = NULL;
   COMPONENT_T *list[5];
   TUNNEL_T tunnel[4];
   ILCLIENT_T *client;
   FILE *in;
   int status = 0;
   unsigned int data_len = 0;
   int packet_size = 80<<10;

   memset(list, 0, sizeof(list));
   memset(tunnel, 0, sizeof(tunnel));

   if((in = fopen(filename, "rb")) == NULL){
      /*return (void *)-2;*/}

   if((client = ilclient_init()) == NULL)
   {
      fclose(in);
      //return (void *)-3;
   }

   if(OMX_Init() != OMX_ErrorNone)
   {
      ilclient_destroy(client);
      fclose(in);
      //return (void *)-4;
   }

   // callback
   ilclient_set_fill_buffer_done_callback(client, my_fill_buffer_done1, this);

   // create video_decode
   if(ilclient_create_component(client, &video_decode, "video_decode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
      status = -14;
   list[0] = video_decode;

   // create egl_render1
   if(status == 0 && ilclient_create_component(client, &egl_render1, "egl_render", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_OUTPUT_BUFFERS)) != 0)
      status = -14;
   list[1] = egl_render1;

   // create clock
   if(status == 0 && ilclient_create_component(client, &clock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[2] = clock;

   memset(&cstate, 0, sizeof(cstate));
   cstate.nSize = sizeof(cstate);
   cstate.nVersion.nVersion = OMX_VERSION;
   cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
   cstate.nWaitMask = 1;
   if(clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
      status = -13;

   // create video_scheduler
   if(status == 0 && ilclient_create_component(client, &video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[3] = video_scheduler;

   set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
   set_tunnel(tunnel+1, video_scheduler, 11, egl_render1, 220);
   set_tunnel(tunnel+2, clock, 80, video_scheduler, 12);

   // setup clock tunnel first
   if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
      status = -15;
   else
      ilclient_change_component_state(clock, OMX_StateExecuting);

   if(status == 0)
      ilclient_change_component_state(video_decode, OMX_StateIdle);

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   if(status == 0 &&
      OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
      ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
   {
      OMX_BUFFERHEADERTYPE *buf;
      int port_settings_changed = 0;
      int first_packet = 1;

      ilclient_change_component_state(video_decode, OMX_StateExecuting);

      while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL && video_dec_loop1)
      {
         // feed data and wait until we get port settings changed
         unsigned char *dest = buf->pBuffer;

         // loop if at end
         if (feof(in))
            rewind(in);

         data_len += fread(dest, 1, packet_size-data_len, in);

         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            if(ilclient_setup_tunnel(tunnel, 0, 0) != 0)
            {
               status = -7;
               break;
            }

            ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

            // now setup tunnel to egl_render1
            if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
            {
               status = -12;
               break;
            }

            // Set egl_render1 to idle
            ilclient_change_component_state(egl_render1, OMX_StateIdle);

            // Enable the output port and tell egl_render1 to use the texture as a buffer
            //ilclient_enable_port(egl_render1, 221); THIS BLOCKS SO CANT BE USED
            if (OMX_SendCommand(ILC_GET_HANDLE(egl_render1), OMX_CommandPortEnable, 221, NULL) != OMX_ErrorNone)
            {
               printf("OMX_CommandPortEnable failed.\n");
               exit(1);
            }

            if (OMX_UseEGLImage(ILC_GET_HANDLE(egl_render1), &eglBuffer1, 221, NULL, eglImage1) != OMX_ErrorNone)
            {
               printf("OMX_UseEGLImage failed.\n");
               exit(1);
            }

            // Set egl_render1 to executing
            ilclient_change_component_state(egl_render1, OMX_StateExecuting);


            // Request egl_render1 to write data to the texture buffer
            if(OMX_FillThisBuffer(ILC_GET_HANDLE(egl_render1), eglBuffer1) != OMX_ErrorNone)
            {
               printf("OMX_FillThisBuffer failed.\n");
               exit(1);
            }
         }
         if(!data_len)
            break;

         buf->nFilledLen = data_len;
         data_len = 0;

         buf->nOffset = 0;
         if(first_packet)
         {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

         if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         {
            status = -6;
            break;
         }
      }

      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         status = -20;

      // need to flush the renderer to allow video_decode to disable its input port
      ilclient_flush_tunnels(tunnel, 0);

      ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
   }

   fclose(in);

   ilclient_disable_tunnel(tunnel);
   ilclient_disable_tunnel(tunnel+1);
   ilclient_disable_tunnel(tunnel+2);
   ilclient_teardown_tunnels(tunnel);

   // ##### TODO ### stürtzt ab # warum???
   //ilclient_state_transition(list, OMX_StateIdle);
   //ilclient_state_transition(list, OMX_StateLoaded);

   ilclient_cleanup_components(list);

   OMX_Deinit();

   ilclient_destroy(client);
   //return (void *)status;
}

void VideoPlayer::video_decode_test2() {
   //const char* filename = "/opt/vc/src/hello_pi/hello_video/test.h264";
   const char* filename = this->filename2.c_str();

   if (eglImage2 == 0)
   {
      printf("eglImage2 is null.\n");
      exit(1);
   }

   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
   COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *clock = NULL;
   COMPONENT_T *list[5];
   TUNNEL_T tunnel[4];
   ILCLIENT_T *client;
   FILE *in;
   int status = 0;
   unsigned int data_len = 0;
   int packet_size = 80<<10;

   memset(list, 0, sizeof(list));
   memset(tunnel, 0, sizeof(tunnel));

   if((in = fopen(filename, "rb")) == NULL){
      /*return (void *)-2;*/}

   if((client = ilclient_init()) == NULL)
   {
      fclose(in);
      //return (void *)-3;
   }

   if(OMX_Init() != OMX_ErrorNone)
   {
      ilclient_destroy(client);
      fclose(in);
      //return (void *)-4;
   }


   // callback
   ilclient_set_fill_buffer_done_callback(client, my_fill_buffer_done2, this);

   // create video_decode
   if(ilclient_create_component(client, &video_decode, "video_decode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
      status = -14;
   list[0] = video_decode;

   // create egl_render2
   if(status == 0 && ilclient_create_component(client, &egl_render2, "egl_render", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_OUTPUT_BUFFERS)) != 0)
      status = -14;
   list[1] = egl_render2;

   // create clock
   if(status == 0 && ilclient_create_component(client, &clock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[2] = clock;

   memset(&cstate, 0, sizeof(cstate));
   cstate.nSize = sizeof(cstate);
   cstate.nVersion.nVersion = OMX_VERSION;
   cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
   cstate.nWaitMask = 1;
   if(clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
      status = -13;

   // create video_scheduler
   if(status == 0 && ilclient_create_component(client, &video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[3] = video_scheduler;

   set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
   set_tunnel(tunnel+1, video_scheduler, 11, egl_render2, 220);
   set_tunnel(tunnel+2, clock, 80, video_scheduler, 12);

   // setup clock tunnel first
   if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
      status = -15;
   else
      ilclient_change_component_state(clock, OMX_StateExecuting);

   if(status == 0)
      ilclient_change_component_state(video_decode, OMX_StateIdle);

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   if(status == 0 &&
      OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
      ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
   {
      OMX_BUFFERHEADERTYPE *buf;
      int port_settings_changed = 0;
      int first_packet = 1;

      ilclient_change_component_state(video_decode, OMX_StateExecuting);

      while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL && video_dec_loop2)
      {
         // feed data and wait until we get port settings changed
         unsigned char *dest = buf->pBuffer;

         // loop if at end
         if (feof(in))
            rewind(in);

         data_len += fread(dest, 1, packet_size-data_len, in);

         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            if(ilclient_setup_tunnel(tunnel, 0, 0) != 0)
            {
               status = -7;
               break;
            }

            ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

            // now setup tunnel to egl_render2
            if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
            {
               status = -12;
               break;
            }

            // Set egl_render2 to idle
            ilclient_change_component_state(egl_render2, OMX_StateIdle);

            // Enable the output port and tell egl_render2 to use the texture as a buffer
            //ilclient_enable_port(egl_render2, 221); THIS BLOCKS SO CANT BE USED
            if (OMX_SendCommand(ILC_GET_HANDLE(egl_render2), OMX_CommandPortEnable, 221, NULL) != OMX_ErrorNone)
            {
               printf("OMX_CommandPortEnable failed.\n");
               exit(1);
            }

            if (OMX_UseEGLImage(ILC_GET_HANDLE(egl_render2), &eglBuffer2, 221, NULL, eglImage2) != OMX_ErrorNone)
            {
               printf("OMX_UseEGLImage failed.\n");
               exit(1);
            }

            // Set egl_render2 to executing
            ilclient_change_component_state(egl_render2, OMX_StateExecuting);


            // Request egl_render2 to write data to the texture buffer
            if(OMX_FillThisBuffer(ILC_GET_HANDLE(egl_render2), eglBuffer2) != OMX_ErrorNone)
            {
               printf("OMX_FillThisBuffer failed.\n");
               exit(1);
            }
         }
         if(!data_len)
            break;

         buf->nFilledLen = data_len;
         data_len = 0;

         buf->nOffset = 0;
         if(first_packet)
         {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

         if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         {
            status = -6;
            break;
         }
      }

      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         status = -20;

      // need to flush the renderer to allow video_decode to disable its input port
      ilclient_flush_tunnels(tunnel, 0);

      ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
   }

   fclose(in);

   ilclient_disable_tunnel(tunnel);
   ilclient_disable_tunnel(tunnel+1);
   ilclient_disable_tunnel(tunnel+2);
   ilclient_teardown_tunnels(tunnel);

   // ##### TODO ### stürtzt ab # warum???
   //ilclient_state_transition(list, OMX_StateIdle);
   //ilclient_state_transition(list, OMX_StateLoaded);

   ilclient_cleanup_components(list);

   OMX_Deinit();

   ilclient_destroy(client);
   //return (void *)status;
}

void VideoPlayer::video_decode_test3() {
   //const char* filename = "/opt/vc/src/hello_pi/hello_video/test.h264";
   const char* filename = this->filename3.c_str();

   if (eglImage3 == 0)
   {
      printf("eglImage3 is null.\n");
      exit(1);
   }

   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
   COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *clock = NULL;
   COMPONENT_T *list[5];
   TUNNEL_T tunnel[4];
   ILCLIENT_T *client;
   FILE *in;
   int status = 0;
   unsigned int data_len = 0;
   int packet_size = 80<<10;

   memset(list, 0, sizeof(list));
   memset(tunnel, 0, sizeof(tunnel));

   if((in = fopen(filename, "rb")) == NULL){
      /*return (void *)-2;*/}

   if((client = ilclient_init()) == NULL)
   {
      fclose(in);
      //return (void *)-3;
   }

   if(OMX_Init() != OMX_ErrorNone)
   {
      ilclient_destroy(client);
      fclose(in);
      //return (void *)-4;
   }


   // callback
   ilclient_set_fill_buffer_done_callback(client, my_fill_buffer_done3, this);

   // create video_decode
   if(ilclient_create_component(client, &video_decode, "video_decode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
      status = -14;
   list[0] = video_decode;

   // create egl_render3
   if(status == 0 && ilclient_create_component(client, &egl_render3, "egl_render", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_OUTPUT_BUFFERS)) != 0)
      status = -14;
   list[1] = egl_render3;

   // create clock
   if(status == 0 && ilclient_create_component(client, &clock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[2] = clock;

   memset(&cstate, 0, sizeof(cstate));
   cstate.nSize = sizeof(cstate);
   cstate.nVersion.nVersion = OMX_VERSION;
   cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
   cstate.nWaitMask = 1;
   if(clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
      status = -13;

   // create video_scheduler
   if(status == 0 && ilclient_create_component(client, &video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[3] = video_scheduler;

   set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
   set_tunnel(tunnel+1, video_scheduler, 11, egl_render3, 220);
   set_tunnel(tunnel+2, clock, 80, video_scheduler, 12);

   // setup clock tunnel first
   if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
      status = -15;
   else
      ilclient_change_component_state(clock, OMX_StateExecuting);

   if(status == 0)
      ilclient_change_component_state(video_decode, OMX_StateIdle);

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   if(status == 0 &&
      OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
      ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
   {
      OMX_BUFFERHEADERTYPE *buf;
      int port_settings_changed = 0;
      int first_packet = 1;

      ilclient_change_component_state(video_decode, OMX_StateExecuting);

      while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL && video_dec_loop3)
      {
         // feed data and wait until we get port settings changed
         unsigned char *dest = buf->pBuffer;

         // loop if at end
         if (feof(in))
            rewind(in);

         data_len += fread(dest, 1, packet_size-data_len, in);

         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            if(ilclient_setup_tunnel(tunnel, 0, 0) != 0)
            {
               status = -7;
               break;
            }

            ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

            // now setup tunnel to egl_render3
            if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
            {
               status = -12;
               break;
            }

            // Set egl_render3 to idle
            ilclient_change_component_state(egl_render3, OMX_StateIdle);

            // Enable the output port and tell egl_render3 to use the texture as a buffer
            //ilclient_enable_port(egl_render3, 221); THIS BLOCKS SO CANT BE USED
            if (OMX_SendCommand(ILC_GET_HANDLE(egl_render3), OMX_CommandPortEnable, 221, NULL) != OMX_ErrorNone)
            {
               printf("OMX_CommandPortEnable failed.\n");
               exit(1);
            }

            if (OMX_UseEGLImage(ILC_GET_HANDLE(egl_render3), &eglBuffer3, 221, NULL, eglImage3) != OMX_ErrorNone)
            {
               printf("OMX_UseEGLImage failed.\n");
               exit(1);
            }

            // Set egl_render3 to executing
            ilclient_change_component_state(egl_render3, OMX_StateExecuting);


            // Request egl_render3 to write data to the texture buffer
            if(OMX_FillThisBuffer(ILC_GET_HANDLE(egl_render3), eglBuffer3) != OMX_ErrorNone)
            {
               printf("OMX_FillThisBuffer failed.\n");
               exit(1);
            }
         }
         if(!data_len)
            break;

         buf->nFilledLen = data_len;
         data_len = 0;

         buf->nOffset = 0;
         if(first_packet)
         {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

         if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         {
            status = -6;
            break;
         }
      }

      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         status = -20;

      // need to flush the renderer to allow video_decode to disable its input port
      ilclient_flush_tunnels(tunnel, 0);

      ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
   }

   fclose(in);

   ilclient_disable_tunnel(tunnel);
   ilclient_disable_tunnel(tunnel+1);
   ilclient_disable_tunnel(tunnel+2);
   ilclient_teardown_tunnels(tunnel);

   // ##### TODO ### stürtzt ab # warum???
   //ilclient_state_transition(list, OMX_StateIdle);
   //ilclient_state_transition(list, OMX_StateLoaded);

   ilclient_cleanup_components(list);

   OMX_Deinit();

   ilclient_destroy(client);
   //return (void *)status;
}

