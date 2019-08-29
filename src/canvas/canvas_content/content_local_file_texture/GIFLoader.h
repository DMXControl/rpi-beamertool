/**
 * +-------+
 * | gif.c |
 * +-------+
 *
 * Stellt die Funktion loadAnimatedGif() zur verfügung, mit der man die Frames
 * eines animiertes (oder auch normales) GIF-Bild im raw RGBA Format in den
 * Speicher lesen kann.
 * z.B. nützlich um gif Bilder als OpenGL Textur verwenden zu können.
 *
 * (c) 2013 Moritz Heinemann <mo@blog-srv.net>
 *
 * Version 0.1 (beta 0.1) Build: 2013_12_16
 *
 */

#ifndef GIF_LOADER_H
#define GIF_LOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
   unsigned char signature[4];
   unsigned char version[4];
} GifHeader;

typedef struct {
   int width;
   int height;
   unsigned char packedFields;
   unsigned char globalColorTableFlag;
   unsigned char colorResolution;
   unsigned char sortFlag;
   unsigned char globalColorTableSize;
   unsigned char backgroundColorIndex;
   unsigned char pixelAspectRatio;
} GifScreenDescriptor;

typedef struct {
   int leftPosition;
   int topPosition;
   int width;
   int height;
   unsigned char packedFields;
   unsigned char localColorTableFlag;
   unsigned char interlaceFlag;
   unsigned char sortFlag;
   unsigned char localColorTableSize;
} GifImageDescriptor;

typedef struct {
   unsigned char r;
   unsigned char g;
   unsigned char b;
} RGB;

typedef struct {
   int length;
   unsigned char * entry;
} TableCell;

typedef struct {
   int length;
   TableCell* content;
} TableHead;

typedef struct {
   int numberOfImages;
   int isLoopImage;
   int loopCount;
   int * delayArray;
} GifAnimationInfo;

typedef struct {
   unsigned char disposalMethod;
   unsigned char userInputFlag;
   unsigned char transparencyFlag;
   int delayTime;
   unsigned char transparentColorIndex;
} GifGraphicControlExtension;

namespace Beamertool {

    class GIFLoader {

    public:
        GIFLoader();

    private:
        unsigned char * readFileToBuffer ( const char * fileName, long * fileSizeOutput );
        int isGifHeader(unsigned char * testMe);
        TableHead * getEmptyTable();
        void freeTableContent(TableHead * pTable);
        void initTable(TableHead * pTable, int initLength);
        void freeTable(TableHead ** pTable);
        void addTableEntryWithOldContent(TableHead * pTable, int oldEntry, unsigned char newChar);
        int getNextCode(unsigned char * codeStream, int * bytePos, int * bitPos, int bitWidth, int bitPattern, int maxCodeStreamLength, int * error);
        void insertChar(unsigned char input, unsigned char * dataStream, int * pos, int max, int *error);
        unsigned char * LZW_decompression ( unsigned char * codeStream, int codeStreamLength, int LZW_minBitLength, int imageSize);
        GifHeader loadHeaderFromFileBuffer (unsigned char * fileBuffer);
        GifScreenDescriptor loadScreenDescriptorFromFileBuffer (unsigned char * fileBuffer);
        GifImageDescriptor loadImageDescriptorFromFileBuffer (unsigned char * fileBuffer, int bufferPos);
        void deinterlace(unsigned char * codeStream, int width, int height);

    public:
        unsigned char ** loadAnimatedGif ( const char * fileName, int * width, int * height, GifAnimationInfo * animationInfo );

    };
}

#endif // GIF_LOADER_H
