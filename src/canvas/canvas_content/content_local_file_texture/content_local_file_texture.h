// canvas/canvas_content/content_local_file_texture/content_local_file_texture.h

#ifndef CONTENT_LOCAL_FILE_TEXTURE_H
#define CONTENT_LOCAL_FILE_TEXTURE_H

#include "../content_interface.h"
#include "texture.h"
#include "video_player.h"

#define TEX_PATH "./textures/"  // Verzeichnis in dem die Texturen sind
#define TEX_MAX 256             // Limitierung der geladenen Texturen
                                // NICHT ÄNDERN!!! kann zum Aufruf nicht existenter Variablen führen

namespace Beamertool {

    class ContentLocalFileTexture : public ContentInterface {

    public:
        ContentLocalFileTexture(EGLDisplay* d, EGLSurface* s, EGLContext* c);
        ~ContentLocalFileTexture();
        std::string getContentTypeName();
        void getContentRatio(int content_data, float * ratio);
        void setContent(int content_data);

    private:
        Texture * textures[TEX_MAX];
        VideoPlayer * player;
        GLfloat texCoords[4*2];
        GLfloat texCoordsVid[4*2];
    };
}

#endif // CONTENT_LOCAL_FILE_TEXTURE_H
