// canvas/canvas_content/content_example_plugin/content_example_plugin.cpp

#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "content_example_plugin.h"

using namespace std;
using namespace Beamertool;

ContentExamplePlugin::ContentExamplePlugin(EGLDisplay* d, EGLSurface* s, EGLContext* c) {

    this->texCoords[0] = 0.f;
    this->texCoords[1] = 1.f;
    this->texCoords[2] = 1.f;
    this->texCoords[3] = 1.f;
    this->texCoords[4] = 0.f;
    this->texCoords[5] = 0.f;
    this->texCoords[6] = 1.f;
    this->texCoords[7] = 0.f;

    glTexCoordPointer(2, GL_FLOAT, 0, this->texCoords);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    this->size_x = 1;
    this->size_y = 1;

    unsigned char * buffer = (unsigned char*) malloc(4);
    buffer[0] = 0xFF;
    buffer[1] = 0xFF;
    buffer[2] = 0x00;
    buffer[3] = 0xFF;  // 0x0 = transparent 0xFF = deckend

    glGenTextures(1, &this->texture);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->size_x, this->size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    free(buffer);
}

ContentExamplePlugin::~ContentExamplePlugin() {
    glDeleteTextures(1, &this->texture);
}

std::string ContentExamplePlugin::getContentTypeName() {
    return string("CONTENT_EXAMPLE");
}

void ContentExamplePlugin::getContentRatio(int content_data, float * ratio) {
    (*ratio) = (float)this->size_y / (float)this->size_x;;
}

void ContentExamplePlugin::setContent(int content_data) {
    glTexCoordPointer(2, GL_FLOAT, 0, this->texCoords);
    glBindTexture(GL_TEXTURE_2D, this->texture);
}
