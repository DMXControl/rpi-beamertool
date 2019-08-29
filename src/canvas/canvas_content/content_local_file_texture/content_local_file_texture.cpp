// canvas/canvas_content/content_local_file_texture/content_local_file_texture.cpp

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "content_local_file_texture.h"
#include "texture.h"
#include "video_player.h"

using namespace std;
using namespace Beamertool;

ContentLocalFileTexture::ContentLocalFileTexture(EGLDisplay* d, EGLSurface* s, EGLContext* c) {

    this->texCoords[0] = 0.f;
    this->texCoords[1] = 1.f;
    this->texCoords[2] = 1.f;
    this->texCoords[3] = 1.f;
    this->texCoords[4] = 0.f;
    this->texCoords[5] = 0.f;
    this->texCoords[6] = 1.f;
    this->texCoords[7] = 0.f;
    this->texCoordsVid[0] = 0.f;
    this->texCoordsVid[1] = 0.f;
    this->texCoordsVid[2] = 1.f;
    this->texCoordsVid[3] = 0.f;
    this->texCoordsVid[4] = 0.f;
    this->texCoordsVid[5] = 1.f;
    this->texCoordsVid[6] = 1.f;
    this->texCoordsVid[7] = 1.f;

    glTexCoordPointer(2, GL_FLOAT, 0, this->texCoords);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    for (int i = 0; i<TEX_MAX; ++i) {
        this->textures[i] = NULL;
    }

    this->player = new VideoPlayer(d, s, c);

    for (int i=0; i < TEX_MAX; ++i) {
        this->textures[i] = new Texture(i, this->player, this->texCoords, this->texCoordsVid);
    }
}

ContentLocalFileTexture::~ContentLocalFileTexture() {
    this->player->sendStopSignal();
    for (int i=0; i < TEX_MAX; ++i) {
        delete this->textures[i];
    }
    delete this->player;
}

std::string ContentLocalFileTexture::getContentTypeName() {
    return string("CONTENT_TEXTURE_256");
}

void ContentLocalFileTexture::getContentRatio(int content_data, float * ratio) {
    if (content_data < 0 || content_data > TEX_MAX) {
        content_data = 0;
    }
    (*ratio) = this->textures[content_data]->getTextureRatio();
}

void ContentLocalFileTexture::setContent(int content_data) {
    if (content_data < 0 || content_data > TEX_MAX) {
        content_data = 0;
    }
    textures[content_data]->setTexture();
}
