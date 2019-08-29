// canvas/canvas_content/content_local_file_texture/texture.cpp

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include "texture.h"
#include "video_player.h"
#include "GIFLoader.h"
#include "stb_image.h"

using namespace std;
using namespace Beamertool;

Texture::Texture(int t_id, VideoPlayer * player, GLfloat texCoords_ptr[8], GLfloat texCoordsVid_ptr[8]) {
    this->texture_id = t_id;
    this->texture_type = TEX_NONE;
    this->size_x = 0;
    this->size_y = 0;
    this->player = player;
    this->texCoords_ptr = texCoords_ptr;
    this->texCoordsVid_ptr = texCoordsVid_ptr;

    load();
}

Texture::~Texture() {
    if (this->texture_type == TEX_NONE) {
        glDeleteTextures(1, &this->none_tex);
    } else if (this->texture_type == TEX_PNG) {
        glDeleteTextures(1, &this->png_tex);
    } else if (this->texture_type == TEX_GIF) {
        glDeleteTextures(this->gif_info.numberOfImages, this->gif_tex);
        delete this->gif_tex;
    } else if (this->texture_type == TEX_VIDEO) {
        this->player->stopPlayer(this->player_id);
    }
}

void Texture::load() {
    string filename(TEX_PATH);
    if (this->texture_id < 100) {
        filename += "0";
    }
    if (this->texture_id < 10) {
        filename += "0";
    }
    filename += to_string(this->texture_id);

    if(fileExist(filename+".png")) {
        filename += ".png";
        this->texture_type = TEX_PNG;
        int n = 0;
        this->png_buffer = stbi_load(filename.c_str(), &this->size_x, &this->size_y, &n, 4); // 4Byte pro Pixel erzwingen

        glGenTextures(1, &this->png_tex);
        glBindTexture(GL_TEXTURE_2D, this->png_tex);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->size_x, this->size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->png_buffer);

        stbi_image_free(this->png_buffer);
        this->png_buffer = NULL;

    } else if (fileExist(filename+".gif")) {
        filename += ".gif";
        this->texture_type = TEX_GIF;

        GIFLoader GIFL;
        this->gif_buffer = GIFL.loadAnimatedGif(filename.c_str(), &this->size_x, &this->size_y, &this->gif_info);

        struct timeval tv;
        gettimeofday (&tv, NULL);
        this->time_sec = (long) tv.tv_sec;
        this->time_usec = (long) tv.tv_usec;

        this->gif_time = 0.d;
        int timeSum = 0;
        for (int z = 0; z < this->gif_info.numberOfImages; ++z) {
            timeSum += this->gif_info.delayArray[z];
            this->gif_info.delayArray[z] = timeSum;
        }

        // TODO Fehler abfangen
        //if (images != NULL) {}

        this->gif_tex = new GLuint[this->gif_info.numberOfImages];
        glGenTextures(this->gif_info.numberOfImages, this->gif_tex);
        for(int j = 0; j < this->gif_info.numberOfImages; ++j) {
            glBindTexture(GL_TEXTURE_2D, this->gif_tex[j]);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->size_x, this->size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->gif_buffer[j]);
            free(this->gif_buffer[j]);
        }
        free(this->gif_buffer);

    } else if (fileExist(filename+".h264")) {
        filename += ".h264";
        this->texture_type = TEX_VIDEO;

        if(getVideoSize(filename, &this->size_x, &this->size_y)) {

            glGenTextures(1, &this->video_tex);
            glBindTexture(GL_TEXTURE_2D, this->video_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->size_x, this->size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            this->player_id = this->player->startFreePlayer(filename, &this->video_tex);

            if (this->player_id <= 0) {
                this->makeErrorTexture(0xFF, 0x0, 0x0, 0x3F);
            }
        } else {
            this->makeErrorTexture(0xFF, 0xFF, 0x0, 0x3F);
        }
    } else {
        this->makeErrorTexture(0xFF, 0x0, 0x0, 0x0);
    }
}

void Texture::makeErrorTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    this->texture_type = TEX_NONE;
    this->png_buffer = (unsigned char*) malloc(4);
    this->png_buffer[0] = r;
    this->png_buffer[1] = g;
    this->png_buffer[2] = b;
    this->png_buffer[3] = a;  // 0x0 = transparent 0xFF = deckend

    this->size_x = 1;
    this->size_y = 1;

    glGenTextures(1, &this->none_tex);
    glBindTexture(GL_TEXTURE_2D, this->none_tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->size_x, this->size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->png_buffer);

    free(this->png_buffer);
    this->png_buffer = NULL;
}

bool Texture::fileExist(const std::string &filename) {
    FILE *file = fopen(filename.c_str(), "r");
    if (file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}

bool Texture::getVideoSize(std::string &filename, int *size_x, int *size_y) {
    string mediainfo_call("mediainfo --Inform=\"Video;info4beamertool|%Width%|%Height%|\" ");
    mediainfo_call.append(filename);

    FILE *fp;
    char mediainfo_output[200];
    fp = popen(mediainfo_call.c_str(), "r");
    if (fp == NULL){
        return false;
    }
    fgets(mediainfo_output, 200, fp);
    string size_str(mediainfo_output);
    pclose(fp);

    if (size_str.compare(0, 16, "info4beamertool|") == 0) {
        size_str.erase(0, 16);
        string::size_type sz;
        int width = stoi(size_str, &sz);
        size_str.erase(0, sz+1);
        int height = stoi(size_str, &sz);
        if (width > 0 && height > 0) {
            (*size_x) = width;
            (*size_y) = height;
            return true;
        }
    }
    return false;
}

double Texture::getSecondsSinceLastCall() {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    long sec = tv.tv_sec - this->time_sec;
    long usec = tv.tv_usec - this->time_usec;
    this->time_sec = tv.tv_sec;
    this->time_usec = tv.tv_usec;
    return (((double) sec) + (((double) usec) / 1000000.d));
}

void Texture::setTexture() {

    glTexCoordPointer(2, GL_FLOAT, 0, this->texCoords_ptr);

    if (this->texture_type == TEX_NONE) {
        glBindTexture(GL_TEXTURE_2D, this->none_tex);
    } else if (this->texture_type == TEX_PNG) {
        glBindTexture(GL_TEXTURE_2D, this->png_tex);
    } else if (this->texture_type == TEX_GIF) {
        // add time since last Frame to gif_time
        this->gif_time += (getSecondsSinceLastCall() * 100.d);
        // make gif_time in range [0, max_delay]
        while (this->gif_time > this->gif_info.delayArray[this->gif_info.numberOfImages-1]) {
            this->gif_time -= this->gif_info.delayArray[this->gif_info.numberOfImages-1];
        }
        // find the right gif frame from gif_time
        int z = 0;
        while (this->gif_info.delayArray[z] < this->gif_time) {
            ++z;
        }
        glBindTexture(GL_TEXTURE_2D, this->gif_tex[z]);
    } else if (this->texture_type == TEX_VIDEO) {
        glTexCoordPointer(2, GL_FLOAT, 0, this->texCoordsVid_ptr);
        glBindTexture(GL_TEXTURE_2D, this->video_tex);
    }
}

float Texture::getTextureRatio() {
    if (this->size_x > 0 && this->size_y > 0) {
        return (float)this->size_y / (float)this->size_x;
    }
    return 0.f;
}
