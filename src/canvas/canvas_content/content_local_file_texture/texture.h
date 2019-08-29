// canvas/canvas_content/content_local_file_texture/texture.h

#ifndef TEXTURE_H
#define TEXTURE_H

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "video_player.h"
#include "GIFLoader.h"

#define TEX_PATH "./textures/"

namespace Beamertool {

    enum TEXTURE_TYPE {TEX_NONE, TEX_PNG, TEX_GIF, TEX_VIDEO};

    class Texture {

    public:
        Texture(int t_id, VideoPlayer * player, GLfloat texCoords_ptr[8], GLfloat texCoordsVid_ptr[8]);
        ~Texture();
        void setTexture();
        float getTextureRatio();

    private:
        void load();
        void makeErrorTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
        bool fileExist(const std::string &filename);
        bool getVideoSize(std::string &filename, int *size_x, int *size_y);
        double getSecondsSinceLastCall();

        GLfloat* texCoords_ptr;
        GLfloat* texCoordsVid_ptr;

        int texture_id;
        TEXTURE_TYPE texture_type;
        int size_x;
        int size_y;

        GLuint none_tex;

        unsigned char *png_buffer;
        GLuint png_tex;

        unsigned char **gif_buffer;
        GifAnimationInfo gif_info;
        double gif_time;
        unsigned long time_sec;
        unsigned long time_usec;

        GLuint *gif_tex;

        VideoPlayer * player;
        int player_id;
        GLuint video_tex;

    };
}

#endif // TEXTURE_H
