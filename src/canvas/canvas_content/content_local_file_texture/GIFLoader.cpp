
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GIFLoader.h"

using namespace Beamertool;

GIFLoader::GIFLoader() {
}

/**
 * readFileToBuffer
 *
 *   L�dt eine Datei in den Speicher
 *
 * Parameter
 *   - char*  fileName         Dateiname
 *   - long*  fileSizeOutput   Gr��e der Datei
 *
 * R�ckgabe
 *   unsigned char*   Zeiger auf den Dateianfang oder NULL wenn ein Fehler
 *                    aufgetreten ist
 */
unsigned char * GIFLoader::readFileToBuffer ( const char * fileName, long * fileSizeOutput ) {
   FILE * file;
   unsigned long fileSize;
   unsigned char * buffer = NULL;
   size_t result;
   
   file = fopen ( fileName, "rb" );
   if (file != NULL) {
      
      // Dateigr��e bestimmen
      fseek (file , 0 , SEEK_END);
      fileSize = ftell (file);
      rewind (file);
      
      // Speicher f�r die Datei zuteilen
      buffer = (unsigned char*) malloc (sizeof(unsigned char)*fileSize);
      
      if (buffer != NULL) {
         
         // Datei in buffer kopieren
         result = fread (buffer,1,fileSize,file);
         
         // Fehler check
         if (result != fileSize) {
            free(buffer);
            buffer = NULL;
         } else {
            *fileSizeOutput = fileSize;
         }
      }
      fclose (file);
   }
   return buffer;
}

/**
 * isGifHeader
 *
 *   testet ob der Anfang eines buffers dem Header einer gif Datei entspricht
 *
 * Parameter
 *   - unsigned char*  testMe   der zu pr�fende buffer
 *
 * R�ckgabe
 *   int   1 wenn gif Header sonst 0
 */
int GIFLoader::isGifHeader(unsigned char * testMe) {
   if ( testMe[0] == 'G'
     && testMe[1] == 'I'
     && testMe[2] == 'F'
     && testMe[3] == '8'
     && (testMe[4] == '7' || testMe[4] == '9')
     && testMe[5] == 'a')
   {
      return 1;
   } else {
      return 0;
   }
}

/**
 * getEmptyTable
 *
 *   gibt einen Leeren Tabellenkopf zur�ck
 *
 * R�ckgabe
 *   TableHead*   leeren Tabellen Kopf
 */
TableHead * GIFLoader::getEmptyTable() {
   TableHead * pTable = (TableHead *) malloc(sizeof(TableHead));
   (*pTable).length = 0;
   (*pTable).content = NULL;
   return pTable;
}

/**
 * freeTableContent
 *
 *   leert die Tabelle
 *
 * Parameter
 *   - TableHead*  pTable   zu leerende Tabelle
 */
void GIFLoader::freeTableContent(TableHead * pTable) {
   int i;
   // Inhalt der Eintr�ge freigeben
   for( i = 0; i < (*pTable).length; i++) {
      free((*pTable).content[i].entry);
   }
   // L�nge und Pointer aller Eintr�ge freigeben
   free((*pTable).content);
   (*pTable).content = NULL;
   (*pTable).length = 0;
}

/**
 * initTable
 *
 *   Initialisiert die Tabelle mit initLength Eintr�gen bei denen der Wert =
 *   der Index ist.
 *   !!! Achtung! die Tabelleneintr�ge werden als unsigned char gespeichert
 *   d.h. Werte > 255 laufen �ber!!!
 *
 * Parameter
 *   - TableHead*  pTable       Tabelle
 *   - int         initLength   Anzahl der zu initialisierenden Eintr�ge
 */
void GIFLoader::initTable(TableHead * pTable, int initLength) {
   // Tabelle leeren
   freeTableContent(pTable);
   
   // Tabelleneintr�ge anlegen
   (*pTable).length = initLength;
   (*pTable).content = (TableCell*) malloc(initLength * sizeof(TableCell));
   
   // Inhalt der Tabelleneintr�ge einf�gen
   int i;
   for ( i=0; i < initLength; i++) {
      // je ein Zeichen pro Tabelleneintrag
      (*pTable).content[i].length = 1;
      (*pTable).content[i].entry = (unsigned char*) malloc(sizeof(unsigned char));
      // Eintrag = Index setzten
      // Es kann zum �berlauf des unsigned char kommen wenn i > 255 ist.
      * ((*pTable).content[i].entry) = (unsigned char) i;
   }
}

/**
 * freeTable
 *
 *   leert die Tabelle und l�scht sie dann
 *
 * Parameter
 *   - TableHead**  pTable   Zeiger auf Zeiger auf Tabelle, Tabelleninhalt und
 *                           Tabelle weren gel�scht und der Zeiger auf die
 *                           Tabelle dann auf NULL gesetzt.
 */
void GIFLoader::freeTable(TableHead ** pTable) {
   freeTableContent(*pTable);
   free(*pTable);
   *pTable = NULL;
}

/**
 * addTableEntryWithOldContent
 *
 *   F�gt einen neuen Tabelleneintrag hinzu, der aus dem Inhalt eines schon
 *   vorhanden Eintrags erweitert um newChar besteht.
 *
 * Parameter
 *   - TableHead*     pTable     Tabelle
 *   - int            oldEntry   Index des schon vorhanden Eintrags
 *   - unsigned char  newChar    Zeichen das hinzugef�gt wird
 */
void GIFLoader::addTableEntryWithOldContent(TableHead * pTable, int oldEntry, unsigned char newChar) {
   // Speicher der Tabelle vergr��ern
   (*pTable).content = (TableCell *) realloc((*pTable).content, ((*pTable).length + 1) * sizeof(TableCell));
   
   // L�nge des neuen eintrags bestimmen
   int newEntrySize = (*pTable).content[oldEntry].length + 1;
   
   // L�nge des neuen Eintrags speichern
   (*pTable).content[(*pTable).length].length = newEntrySize;
   // Speicher f�r neuen Eintrag anfordern
   (*pTable).content[(*pTable).length].entry = (unsigned char*) malloc(newEntrySize * sizeof(unsigned char));
   // alten Eintrag in neuen kopieren
   int i;
   for (i=0; i < newEntrySize-1; i++) {
      (*pTable).content[(*pTable).length].entry[i] = (*pTable).content[oldEntry].entry[i];
   }
   // neues Zeichen anf�gen
   (*pTable).content[(*pTable).length].entry[newEntrySize-1] = newChar;
   // neue Tabellenl�nge speichern
   (*pTable).length++;
}

/**
 * getNextCode
 *
 *   gibt den n�chsten Code aus dem Code Stream zur�ck
 *
 * Parameter
 *   - codeStream            Code Stream
 *   - bytePos               Byte Position des Codes (wird danach auf n�chsten gesetzt)
 *   - bitPos                Bit Position des Codes (wird danach auf n�chsten gesetzt)
 *   - bitWidth              Gr��e des Codes in Bit
 *   - bitPattern            Muster entsprechend der Gr��e des Codes
 *   - maxCodeStreamLength   L�nge des Code Streams
 *   - error                 wird gesetzt wenn weiter als Ende des Codestreams gelesen wird
 *
 * R�ckgabe
 *   der n�chste Code aus dem CodeStream
 */
int GIFLoader::getNextCode(unsigned char * codeStream, int * bytePos, int * bitPos, int bitWidth, int bitPattern, int maxCodeStreamLength, int * error) {
   int CODE = 0;
   // Der Code ist nach gif Spezifikation maximal 12 Bit gro�. 12
   // zusammenh�ngende Bit k�nnen maximal auf 3 Byte verteilt sein.
   // Wenn am Ende des Strams keine 3 Byte mehr �brig sind entsprechend weniger
   // lesen.
   if ( (*bytePos) < maxCodeStreamLength - 2) {
      CODE = (codeStream[*bytePos + 2] << 16) + (codeStream[*bytePos + 1] << 8) + codeStream[*bytePos];
   } else if ( (*bytePos) < maxCodeStreamLength - 1) {
      CODE = (codeStream[*bytePos + 1] << 8) + codeStream[*bytePos];
   } else if ( (*bytePos) < maxCodeStreamLength) {
      CODE = codeStream[*bytePos];
   } else {
      (*error) = 1;
   }
   // Code auf richtige Bit Position schieben und passend zuschneiden.
   CODE = (CODE >> *bitPos) & bitPattern;
   
   // Position auf n�chsten Code setzten
   *bitPos += bitWidth;
   while (*bitPos >= 8) {
      (*bytePos)++;
      *bitPos -= 8;
   }
   
   return CODE;
}

/**
 * insertChar
 *
 * f�gt ein Zeichen in den DatenStream ein. Beachtet dabei die L�nge des
 * DatenStream und setzt error wenn eine ung�ltige Position beschrieben werden
 * soll.
 *
 * Parameter:
 *   - input        einzuf�gendes Zeichen
 *   - dataStream   Daten-Stream
 *   - pos          einf�ge-Position
 *   - max          l�nge des Daten-Stream
 *   - error        wird gesetzt wenn pos > max
 *
 */
void GIFLoader::insertChar(unsigned char input, unsigned char * dataStream, int * pos, int max, int *error) {
   if((*pos) < max && (*pos) >= 0) {
      dataStream[(*pos)] = input;
      (*pos)++;
   } else {
      (*error) = 1;
   }
}

/**
 * LZW_decompression
 *
 * Dekomprimiert einen LZW komprimierten CodeStream
 *
 * Parameter
 *   - codeStream         komprimierter Code Stream
 *   - codeStreamLength   l�nge des Code Streams
 *   - LZW_MinBitLength   minimale Bitzahl um decodierte Daten darzustellen
 *   - imageSize          Bildgr��e (soll l�nge der dekomprimierten Daten)
 *
 * R�ckgabe
 *   Zeiger auf die dekomprimierten Daten, wenn Dekomprimierung erfolgreich und
 *   l�nger der Daten = imageSize, sonst NULL
 */
unsigned char * GIFLoader::LZW_decompression ( unsigned char * codeStream, int codeStreamLength, int LZW_minBitLength, int imageSize) {
   
   int error = 0; // Error Flag
   int loop = 1;  // Loop Flag
   
   unsigned char * outData = (unsigned char*) malloc (imageSize); // dekomprimierte Daten
   int outDataPos = 0;                           // aktuelle Position auf den dekomprimierten Daten
   int bitWidth = LZW_minBitLength + 1;          // Breite der Codes in Bit
   int bitPattern = (1 << bitWidth) - 1;         // bitWidth bin�re 1sen
   int bytePos = 0;                              // aktuelle Position im Code Stream
   int bitPos = 0;                               // aktuelle Position im Code Stream
   int CLR = 1 << LZW_minBitLength;              // Clear Code gleichzeitig auch Startgr��e der Tabelle
   int EOD = CLR + 1;                            // End od Data Code
   
   TableHead * pTable = getEmptyTable();         // Pointer auf LZW Tabelle
   initTable(pTable, CLR + 2);                   // Tabelle initialisieren
   
   // aktueller Code
   int CODE = getNextCode(codeStream, &bytePos, &bitPos, bitWidth, bitPattern, codeStreamLength, &error);
   int CODE_1; // vorheriger Code
   
   if(CODE == CLR) {
      // CLR Code an Anfang ignorieren, da Tabelle schon initialisiert ist.
      // nimm n�chsten CODE
      CODE_1 = CODE;
      CODE = getNextCode(codeStream, &bytePos, &bitPos, bitWidth, bitPattern, codeStreamLength, &error);
   }
   if (CODE == EOD) {
      // codeStream beginnt mit Ende zeichen => Sinnlos aber falls Eingabedatei defekt abfangen
      error = 1;
   }
   if (CODE < CLR) {
      insertChar((unsigned char) CODE, outData, &outDataPos, imageSize, &error);
   }
   
   while(loop && !error) {
      CODE_1 = CODE;
      CODE = getNextCode(codeStream, &bytePos, &bitPos, bitWidth, bitPattern, codeStreamLength, &error);
      
      if (CODE == CLR) {
         // Clear Table
         initTable(pTable, CLR + 2);
         bitWidth = LZW_minBitLength + 1;
         bitPattern = (1 << bitWidth) - 1;
         
         // n�chsten CODE laden. CODE_1 wird nicht ben�tigt und beim n�chsten Schleifendurchlauf neu gesetzt
         CODE = getNextCode(codeStream, &bytePos, &bitPos, bitWidth, bitPattern, codeStreamLength, &error);
         
         // n�chsten Code ausgeben
         insertChar((unsigned char) CODE, outData, &outDataPos, imageSize, &error);
         
      } else if(CODE == EOD) {
         loop = 0;
      } else if(CODE < (*pTable).length) { // Fall: CODE ist in Tabelle
         // Tabellenwert von CODE zur Ausgabe hinzuf�gen
         int i;
         for (i=0; i < (*pTable).content[CODE].length; i++) {
            insertChar((*pTable).content[CODE].entry[i], outData, &outDataPos, imageSize, &error);
         }
         // sei K das erste Zeichen des Wertes zum Index CODE
         unsigned char k = (*pTable).content[CODE].entry[0];
         // H�nge K an den Wert zum Index CODE_1 an und mach daraus neuen Tabelleneintrag
         addTableEntryWithOldContent(pTable, CODE_1, k);
      } else { // Fall: CODE ist nicht in Tabelle
         // sei K das erste Zeichen des Wertes zum Index CODE_1
         unsigned char k = (*pTable).content[CODE_1].entry[0];
         // Tabellenwert von CODE_1 und K zur Ausgabe hinzuf�gen
         int i;
         for (i=0; i < (*pTable).content[CODE_1].length; i++) {
            insertChar( (*pTable).content[CODE_1].entry[i], outData, &outDataPos, imageSize, &error);
         }
         insertChar(k, outData, &outDataPos, imageSize, &error);
         // H�nge K an den Wert zum Index CODE_1 an und mach daraus neuen Tabelleneintrag
         addTableEntryWithOldContent(pTable, CODE_1, k);
      }
      
      // Wenn Anzahl der Tabelleneintr�ge gr��er ist, als mit aktuellem
      // Bitmuster darstellbar w�ren, erh�he Bitmuster um eins
      if ((*pTable).length > (1 << bitWidth) - 1) {
         bitWidth++;
         // nach GIF Spezifikation maximale Bitmusterbreite von 12 Bit
         if (bitWidth > 12) {
            bitWidth = 12;
         }
         bitPattern = (1 << bitWidth) - 1;
      }
   }
   
   // Tabelle l�schen
   freeTable(&pTable);
   
   // Fehlertest
   if (outDataPos != imageSize) {
      error = 1;
   }
   
   if (error) {
      free (outData);
      return NULL;
   } else {
      return outData;
   }
}

/**
 * loadHeaderFromFileBuffer
 *
 *   speichert den Anfang eines FileBuffers in ein GifHeader struct.
 *
 * Parameter
 *   - unsigned char *  fileBuffer   fileBuffer der Header enth�lt
 *
 * R�ckgabe
 *   GifHeader   enth�lt den Gif Header
 */
GifHeader GIFLoader::loadHeaderFromFileBuffer (unsigned char * fileBuffer) {
   GifHeader header;
   
   // Load Header
   header.signature[0] = fileBuffer[0];
   header.signature[1] = fileBuffer[1];
   header.signature[2] = fileBuffer[2];
   header.signature[3] = 0; // Terminate string
   
   header.version[0] = fileBuffer[3];
   header.version[1] = fileBuffer[4];
   header.version[2] = fileBuffer[5];
   header.version[3] = 0; // Terminate string
   
   return header;
}

/**
 * loadScreenDescriptorFromFileBuffer
 *
 *   l�d den Screen Descriptor aus einen FileBuffer
 *
 * Parameter
 *   - unsigned char*  fileBuffer   fileBuffer der gif Datei
 *
 * R�ckgabe
 *   GifScreenDescriptor   Screen Descriptor
 */
GifScreenDescriptor GIFLoader::loadScreenDescriptorFromFileBuffer (unsigned char * fileBuffer) {
   GifScreenDescriptor screen_descriptor;
   
   // Load Screen Descriptor
   screen_descriptor.width = (((int) fileBuffer[7]) << 8) +((int) fileBuffer[6]);
   screen_descriptor.height = (((int) fileBuffer[9]) << 8) +((int) fileBuffer[8]);
   screen_descriptor.packedFields = fileBuffer[10];
   screen_descriptor.globalColorTableFlag = (fileBuffer[10] & 0x80) >> 7;
   screen_descriptor.colorResolution = (fileBuffer[10] & 0x70) >> 4;
   screen_descriptor.sortFlag = (fileBuffer[10] & 0x8) >> 3;
   screen_descriptor.globalColorTableSize = (fileBuffer[10] & 0x7);
   screen_descriptor.backgroundColorIndex = fileBuffer[11];
   screen_descriptor.pixelAspectRatio = fileBuffer[12];
   
   return screen_descriptor;
}

/**
 * loadImageDescriptorFromFileBuffer
 *
 *   l�d den Image Descriptor aus einem fileBuffer
 *
 * Parameter
 *   - unsigned char*  fileBuffer   FileBuffer der gif Datei
 *   - int             bufferPos    Position des Image Descriptors
 *
 * R�ckgabe
 *   GifImageDescriptor   Image Descriptor
 */
GifImageDescriptor GIFLoader::loadImageDescriptorFromFileBuffer (unsigned char * fileBuffer, int bufferPos) {
   GifImageDescriptor image_descriptor;
   image_descriptor.leftPosition = (((int) fileBuffer[bufferPos+2]) << 8) +((int) fileBuffer[bufferPos+1]);
   image_descriptor.topPosition = (((int) fileBuffer[bufferPos+4]) << 8) +((int) fileBuffer[bufferPos+3]);
   image_descriptor.width = (((int) fileBuffer[bufferPos+6]) << 8) +((int) fileBuffer[bufferPos+5]);
   image_descriptor.height = (((int) fileBuffer[bufferPos+8]) << 8) +((int) fileBuffer[bufferPos+7]);
   image_descriptor.packedFields = fileBuffer[bufferPos+9];
   image_descriptor.localColorTableFlag = (fileBuffer[bufferPos+9] & 0x80) >> 7;
   image_descriptor.interlaceFlag = (fileBuffer[bufferPos+9] & 0x40) >> 6;
   image_descriptor.sortFlag = (fileBuffer[bufferPos+9] & 0x20) >> 5;
   image_descriptor.localColorTableSize = (fileBuffer[bufferPos+9] & 0x7);

   return image_descriptor;
}

/**
 * deinterlace
 *
 *   Deinterlaced einen gif Indexstream mit der Breite width und H�he heigth
 *   gem�� gif Spezifikation. der Index Stream muss Breite*H�he Eintr�ge haben.
 *
 * Prameter
 *   - unsigned char*  codeStream   Index Stream (wird �berschrieben)
 *   - int             width        Breite
 *   - int             height       H�he
 */
void GIFLoader::deinterlace(unsigned char * codeStream, int width, int height) {
   
   unsigned char * deCodeStream = (unsigned char*) malloc(width * height * sizeof(unsigned char));
   int i = 0;
   int j = 0;
   int c = 0;
   
   // Pass 1
   for ( i = 0; i < height; i += 8 ) {
      for( j = 0; j < width; j++ ) {
         deCodeStream[i * width + j] = codeStream[c * width + j];
      }
      c++;
   }
   // Pass 2
   for ( i = 4; i < height; i += 8 ) {
      for( j = 0; j < width; j++ ) {
         deCodeStream[i * width + j] = codeStream[c * width + j];
      }
      c++;
   }
   // Pass 3
   for ( i = 2; i < height; i += 4 ) {
      for( j = 0; j < width; j++ ) {
         deCodeStream[i * width + j] = codeStream[c * width + j];
      }
      c++;
   }
   // Pass 4
   for ( i = 1; i < height; i += 2) {
      for( j = 0; j < width; j++ ) {
         deCodeStream[i * width + j] = codeStream[c * width + j];
      }
      c++;
   }
   
   // Zur�ckkopieren
   memcpy(codeStream, deCodeStream, width * height * sizeof(unsigned char));
   free(deCodeStream);
}

/**
 * loadAnimatedGif
 *
 *   L�dt eine (animierte) GIF Datei und gibt die einzelnen Frames als RGBA
 *   Pixel Array zur�ck
 *
 * Parameter:
 *   - char*             filename       Dateiname der zu ladenden gif Datei
 *   - int*              width          R�ckgabe der Breite des geladenen gif's
 *   - int*              height         R�ckgabe der H�he des geladenen gif's
 *   - GifAnimationInfo* animationInfo  Animations Info (siehe unten)
 *
 * Animation Info
 *   struct aus:
 *   int  numberOfImages  Anzahl der Frames
 *   int  isLoopImage     0=Frames einmal durchlaufen | 1=Frames wiederholen
 *   int  loopCount       Anzahl wie oft alle Frames durchlaufen werden wenn
 *                        loop Image. 0=endlos Schleife
 *   int* delayArray      Array mit den Delay Zeiten wie lange nach dem
 *                        Frame gewartet werden soll. (in 1/100 Sekunden).
 *                        Enth�lt numberOfImages Eintr�ge.
 *
 * R�ckgabe:
 *   Zeiger auf ein (unsigned char*) Array mit numberOfImages Eintr�gen. Jeder
 *   (unsigned char*) Eintrag Zeigt auf einen Speicherbereich der Gr��e
 *   width*height*4 Bytes in dem jeweils ein Frame des Bildes gespeichert ist.
 *   Jedes Pixel belegt 4 Byte (RGBA), Das Bild ist Zeilenweise abgespeichert
 *   ohne Marker oder Abstand zwischen den einzelnen Zeilen.
 *   Die einzlenen Frames k�nnen z.B. direkt f�r eine OpenGL Textur verwendet
 *   werden.
 */
unsigned char ** GIFLoader::loadAnimatedGif ( const char * fileName, int * width, int * height, GifAnimationInfo * animationInfo ) {
   
   // Datei laden
   long fileSize = 0;
   unsigned char * gifFileBuffer = readFileToBuffer(fileName, &fileSize);
   long bufferPosition = 0;
   
   // Fehlerkontrolle
   if(gifFileBuffer == NULL) {
      //printf("Bad file!\n");
      free(gifFileBuffer);
      return NULL;
   }
   if (fileSize <= 13 || !isGifHeader(gifFileBuffer)) {
      //printf("Bad Header!\n");
      free(gifFileBuffer);
      return NULL;
   }
   
   // Header und Screen Descriptor auslesen
   //GifHeader header = loadHeaderFromFileBuffer(gifFileBuffer); // aktuell unbenutzt
   GifScreenDescriptor screen_descriptor = loadScreenDescriptorFromFileBuffer(gifFileBuffer);
   int colors_in_global_map = 1 << ( screen_descriptor.globalColorTableSize + 1 ); // 2^(var + 1)
   
   // Infos - nur zum Entwickeln gedacht
   //printf("%s: Filesize: %i\n", fileName, (int) fileSize);
   //printf("Version: %s\n", header.version);
   //printf("Width: %i  Height: %i\n",screen_descriptor.width,screen_descriptor.height);
   //printf("Colors in Map: %i\n",colors_in_global_map);
   
   // Buffer hinter Header setzten
   bufferPosition = 13;
   
   // Fehlerkontrolle
   if (fileSize <= colors_in_global_map * 3 + 13) {
      //printf("Bad FileSize!\n");
      free(gifFileBuffer);
      return NULL;
   }
   
   // Globale Farb Tabelle auslesen wenn Flag gesetzt
   RGB * globalColorMap = NULL;
   if (screen_descriptor.globalColorTableFlag) {
      globalColorMap = (RGB*) malloc(colors_in_global_map * sizeof(RGB));
      int i;
      for ( i = 0; i < colors_in_global_map; i++ ) {
         globalColorMap[i].r = gifFileBuffer[bufferPosition + 3*i];
         globalColorMap[i].g = gifFileBuffer[bufferPosition + 3*i + 1];
         globalColorMap[i].b = gifFileBuffer[bufferPosition + 3*i + 2];
      }
      bufferPosition += 3 * colors_in_global_map;
   }
   
   // Bild Array anlegen
   unsigned char ** images = (unsigned char**) malloc(sizeof(unsigned char*));
   images[0] = (unsigned char*) calloc(screen_descriptor.width*screen_descriptor.height, 4);
   
   // Hintergrundfarbe setzten
   if (screen_descriptor.globalColorTableFlag) { // Steht auch f�r Background Color
      int i;
      for(i=0; i<screen_descriptor.width*screen_descriptor.height; i++) {
         images[0][4*i    ] = globalColorMap[screen_descriptor.backgroundColorIndex].r;
         images[0][4*i + 1] = globalColorMap[screen_descriptor.backgroundColorIndex].g;
         images[0][4*i + 2] = globalColorMap[screen_descriptor.backgroundColorIndex].b;
         images[0][4*i + 3] = 0xFF;
      }
   }
   
   // Variablen f�r Animation Informationen
   GifAnimationInfo animInfo;
   animInfo.numberOfImages = 0;
   animInfo.isLoopImage = 0;
   animInfo.loopCount = 0;
   animInfo.delayArray = (int*) calloc(1, sizeof(int));
   
   // Loop kontrolle
   char reachedTrailer = 0;
   char loop = 1;
   
   // Framedetails
   GifGraphicControlExtension GCE;   // Inhalt der GraphicControlExtension
   char useGCE = 0;                  // zeigt an, dass die Graphic Control Extension gesetzt wurde
   char newImage = 0;                // Zeigt an, dass delay>0 war und somit ein neues Frame begonnen werden soll
   
   // Parse Gif Content
   while ( loop && bufferPosition <= fileSize ) {
      // Gif Bl�cke unterscheiden
      switch (gifFileBuffer[bufferPosition]) {
         case ';' :
            // Gif Trailer
            reachedTrailer = 1;
            loop = 0;
            break;
         case ',' : { // Klammern um Block zu erzeugen da in switch keine Variablen erzeugt werden d�rfen.
            // Bildblock
            
            // Wenn delay bei vorherigem Frame: neues Beginnen
            if(newImage) {
               // Image array vergr��ern
               images = (unsigned char**) realloc(images, (animInfo.numberOfImages + 2) * sizeof(char*));
               animInfo.delayArray = (int*) realloc(animInfo.delayArray, (animInfo.numberOfImages + 2) * sizeof(int));
               // Speicher f�r neus Frame
               images[animInfo.numberOfImages+1] = (unsigned char*) calloc(screen_descriptor.width*screen_descriptor.height, 4);
               // altes in neues Frame kopieren (notwendig falls nur ein Teil des Bilder �berschieben wird)
               memcpy(images[animInfo.numberOfImages+1], images[animInfo.numberOfImages], screen_descriptor.width*screen_descriptor.height*4);
               // Z�hler erh�hen
               animInfo.numberOfImages++;
               newImage = 0;
            }
            
            // Image Descriptor des Bild Blocks auslesen
            GifImageDescriptor image_descriptor = loadImageDescriptorFromFileBuffer (gifFileBuffer, bufferPosition);
            int colors_in_local_map = 1 << ( image_descriptor.localColorTableSize + 1 ); // 2^(var + 1)
            // Buffer Position hinter Image Descriptor schieben
            bufferPosition += 10;
            
            // Lokale Farb Tabelle auslesen
            RGB * localColorMap = NULL;
            if (image_descriptor.localColorTableFlag) {
               localColorMap = (RGB *) malloc(colors_in_local_map * sizeof(RGB));
               int i;
               for ( i = 0; i < colors_in_local_map; i++ ) {
                  localColorMap[i].r = gifFileBuffer[bufferPosition + 3*i];
                  localColorMap[i].g = gifFileBuffer[bufferPosition + 3*i + 1];
                  localColorMap[i].b = gifFileBuffer[bufferPosition + 3*i + 2];
               }
               bufferPosition += 3 * colors_in_local_map;
            }
            
            // min Code Size auslesen
            int LZW_minCodeSize = gifFileBuffer[bufferPosition];
            
            // Code Size bestimmen
            int code_size = 0;
            
            bufferPosition++;
            int bufPos = bufferPosition; // Kopie anlegen beide Zeigen auf L�nge des ersten Datenblocks
            
            while (gifFileBuffer[bufPos] != 0) {
               code_size += gifFileBuffer[bufPos];
               bufPos += gifFileBuffer[bufPos] + 1;
            }
            
            // Komprimierte Daten in neuen Buffer kopieren
            unsigned char compressed_data[code_size];
            int dataPos = 0;
            while (gifFileBuffer[bufferPosition] != 0) {
               int i;
               for (i=0; i<gifFileBuffer[bufferPosition]; i++) {
                  compressed_data[dataPos] = gifFileBuffer[bufferPosition + 1 + i];
                  dataPos++;
               }
               bufferPosition += gifFileBuffer[bufferPosition] + 1;
            }
            
            // Stream Decodieren
            int pixels = image_descriptor.width*image_descriptor.height;
            unsigned char * decompressed_data = LZW_decompression(compressed_data, code_size, LZW_minCodeSize, pixels);
            
            if(decompressed_data == NULL) {
               loop = 0;
               free(localColorMap);
               free(decompressed_data);
               break;
            }
            
            // Deinterlace
            if (image_descriptor.interlaceFlag) {
               deinterlace(decompressed_data, image_descriptor.width, image_descriptor.height);
            }
            
            // Farb Tabelle ausw�hlen
            RGB * colorMap;
            if (image_descriptor.localColorTableFlag == 0) { // Use global Color Table
               colorMap = globalColorMap;
            } else {
               colorMap = localColorMap;
            }
            
            // Farben f�r den jeweiligen Index einsetzten
            int i,j;
            for (i=0; i<image_descriptor.height; i++) {
               for (j=0; j<image_descriptor.width; j++) {
                  int screenPos = 4 * ((image_descriptor.topPosition + i) * screen_descriptor.width + image_descriptor.leftPosition + j);
                  int imgDataPos = i * image_descriptor.width + j;
                  // RGB
                  images[animInfo.numberOfImages][ screenPos     ] = colorMap[decompressed_data[imgDataPos]].r;
                  images[animInfo.numberOfImages][ screenPos + 1 ] = colorMap[decompressed_data[imgDataPos]].g;
                  images[animInfo.numberOfImages][ screenPos + 2 ] = colorMap[decompressed_data[imgDataPos]].b;
                  // Transparenz
                  if(useGCE && GCE.transparencyFlag && decompressed_data[imgDataPos] == GCE.transparentColorIndex) {
                     images[animInfo.numberOfImages][ screenPos + 3 ] = 0x00;
                  } else {
                     images[animInfo.numberOfImages][ screenPos + 3 ] = 0xFF;
                  }
               }
            }
            
            // Delay Zeit speichern
            if(useGCE && GCE.delayTime > 0) {
               animInfo.delayArray[animInfo.numberOfImages] = GCE.delayTime;
               newImage = 1;
            }
            useGCE = 0;
            
            free(localColorMap);
            free(decompressed_data);
            
            bufferPosition++; // auf n�chsten Block zeigen
            }
            break;
         case '!' :
            bufferPosition++; // Zeigt auf Function Code
            switch (gifFileBuffer[bufferPosition]) {
               case 0x01:
                  //printf("Ex:   Plain Text Extension");
                  // is not supported by this Programm
                  // perhaps a future TODO
                  break;
               case 0xF9:
                  //printf("Ex:   Graphic Control Extension\n");
                  
                  useGCE = 1;
                  
                  // Block Size
                  if(gifFileBuffer[bufferPosition+1] != 4) {
                     loop = 0;
                  }
                  
                  // Graphic Control Extension auslesen
                  GCE.disposalMethod = (gifFileBuffer[bufferPosition+2] & 0x1C) >> 2;
                  GCE.userInputFlag = (gifFileBuffer[bufferPosition+2] & 0x02) >> 1;
                  GCE.transparencyFlag = (gifFileBuffer[bufferPosition+2] & 0x01);
                  GCE.delayTime = (((int) gifFileBuffer[bufferPosition+4]) << 8) +((int) gifFileBuffer[bufferPosition+3]);
                  GCE.transparentColorIndex = gifFileBuffer[bufferPosition+5];
                  
                  break;
               case 0xFE:
                  //printf("Ex:   Comment\n");
                  // Unwichtig f�r Bildausgabe
                  break;
               case 0xFF:
                  //printf("Ex:   Application\n");
                  // nur NETSCAPE2.0 Application wird Unterst�tzt
                  if (    ( gifFileBuffer [ bufferPosition + 1 ] == 0x0B )
                       && ( gifFileBuffer [ bufferPosition + 2 ] == 'N' )
                       && ( gifFileBuffer [ bufferPosition + 3 ] == 'E' )
                       && ( gifFileBuffer [ bufferPosition + 4 ] == 'T' )
                       && ( gifFileBuffer [ bufferPosition + 5 ] == 'S' )
                       && ( gifFileBuffer [ bufferPosition + 6 ] == 'C' )
                       && ( gifFileBuffer [ bufferPosition + 7 ] == 'A' )
                       && ( gifFileBuffer [ bufferPosition + 8 ] == 'P' )
                       && ( gifFileBuffer [ bufferPosition + 9 ] == 'E' )
                       && ( gifFileBuffer [ bufferPosition + 10 ] == '2' )
                       && ( gifFileBuffer [ bufferPosition + 11 ] == '.' )
                       && ( gifFileBuffer [ bufferPosition + 12 ] == '0' )
                       && ( gifFileBuffer [ bufferPosition + 13 ] == 0x03 )
                       && ( gifFileBuffer [ bufferPosition + 14 ] == 0x01 )) {
                     animInfo.isLoopImage = 1;
                     animInfo.loopCount = (((int) gifFileBuffer[bufferPosition + 16]) << 8) +((int) gifFileBuffer[bufferPosition + 15]);
                  }
                  break;
               default:
                  //printf("Ex:   unbekannter Block mit ID: %i\n", gifFileBuffer[bufferPosition]);
                  
                  break;
            }
            // Block �berspringen (wird hier gemacht, dass auch unbekannte Bl�cke �bersprungen werden)
            bufferPosition++; // Zeigt auf Byte Count
            // Inhalt �berspringen
            while (gifFileBuffer[bufferPosition] != 0) {
               bufferPosition += gifFileBuffer[bufferPosition] + 1;
            }
            bufferPosition++; // Zeigt auf n�chsten Block Marker
            break;
         default:
            //printf("ERROR! Unbekanter GIF Block!\n");
            loop = 0;
            break;
      }
   }

   // terminate
   free (gifFileBuffer);
   free (globalColorMap);
   
   if (reachedTrailer) {
      animInfo.numberOfImages++; // Number of Images ist die ArrayPosition des aktuellen Bildes. Da bei 0 begonnen wird +1
      *width = screen_descriptor.width;
      *height = screen_descriptor.height;
      *animationInfo = animInfo;
      return images;
   }
   
   // Wenn Schleife abgebrichen wurde ohne, dass der gif Trailer erreicht wurde
   // Speicher aufr�umen und NULL zur�ckgeben.
   int i;
   for(i=0; i < animInfo.numberOfImages; i++) {
      free(images[i]);
   }
   free (images);
   return NULL;
}



/**
 * main
 * nur zum Testen!
 */
/*
int main (int argc, const char* argv[]) {
   
   char fileName[] = "./img/sample_2_animation.gif";
   unsigned char ** images;
   int width, height;
   GifAnimationInfo animationInfo;
   
   if(argc == 2) {
      images = loadAnimatedGif ( (char*)argv[1], &width, &height, &animationInfo );
   } else {
      images = loadAnimatedGif ( fileName, &width, &height, &animationInfo );
   }
   
   if (images != NULL) {
      printf("---[GIF]--------------------------------\n");
      printf("Width:      %i\n", width);
      printf("Height:     %i\n", height);
      printf("imgNum:     %i\n", animationInfo.numberOfImages);
      printf("is LoopImg: %i\n", animationInfo.isLoopImage);
      printf("loopCount:  %i\n", animationInfo.loopCount);
      
      int i;
      for(i=0; i<animationInfo.numberOfImages; i++) {
         printf("| %i ", animationInfo.delayArray[i]);
      }
      printf("\n");
      
      for(i=0; i<animationInfo.numberOfImages; i++) {
         
         char filename_buf[20] = "000.raw";
         if (i<10) {
            sprintf (filename_buf, "00%d.raw", i);
         } else if (i<100) {
            sprintf (filename_buf, "0%d.raw", i);
         } else {
            sprintf (filename_buf, "%d.raw", i);
         }
         
         FILE * pFile;
         pFile = fopen (filename_buf,"wb");
         if (pFile!=NULL)
         {
            fwrite (images[i] , 1, width*height*4, pFile);
            fclose (pFile);
         }
         
      }
   }
   
   printf("\n");
   return 0;
}
//*/


