// canvas/canvas.h

#ifndef CANVAS_H
#define CANVAS_H

#include <string>
#include "canvas_classes.h"

namespace Beamertool {

    class Canvas {

    public:
        /**
         * standard cosntructor
         * @param content_type string id of the content plugin used for this canvas
         */
        Canvas(std::string content_type);

        /**
         * destructor
         */
        ~Canvas();

        /**
         * calculate automoves for one frame
         */
        void frameStep();

        /**
         * trim an angle to range [0, 360)
         * @param angle the angle to trim
         * @return angle in range [0,360)
         */
        float correctAngleRange(float angle);

        /**
         * get content type
         * @return content type string
         */
        std::string getContentType();

        /**
         * get content type
         * @return content data
         */
        int getContentData();

        /**
         * get position x direction
         * @return position in x direction
         */
        float getPositionX();

        /**
         * get position y direction
         * @return position in y direction
         */
        float getPositionY();

        /**
         * get scaling in x direction
         * @return scaling in x direction
         */
        float getScalingX();

        /**
         * get scaling in y direction
         * @return scaling in y direction
         */
        float getScalingY();

        /**
         * get rotation angle
         * @return rotation angle
         */
        float getRotation();

        /**
         * get numbers of instances  to display
         * @return number
         */
        unsigned int getNumber();

        /**
         * get zoom factor of instances
         * @return zoom factor
         */
        float getZoom();

        /**
         * get rotation of all instances
         * @return scaling in x direction
         */
        float getRotationAll();

        /**
         * get red
         * @return red
         */
        float getR();

        /**
         * get green
         * @return green
         */
        float getG();

        /**
         * get blue
         * @return blue
         */
        float getB();

        /**
         * get alpha
         * @return alpha
         */
        float getAlpha();

        /**
         * get shutter state
         * @return shutter state
         */
        int getShutterState();

        /**
         * set content data
         * @param data content data (no validity test, is passed to content plugin as it is)
         */
        void setContentData(int data);

        /**
         * set position in x direction
         * @param x position in x direction
         */
        void setPositionX(float x);

        /**
         * set position in y direction
         * @param y position in y direction
         */
        void setPositionY(float y);

        /**
         * set scaling in x direction
         * @param x scaling in x direction
         */
        void setScalingX(float x);

        /**
         * set scaling in y direction
         * @param y scaling in y direction
         */
        void setScalingY(float y);

        /**
         * set rotaion
         * @param r rotaion angle
         */
        void setRotation(float r);

        /**
         * set rotaion speed
         * @param s rotation speed (in degree per frame)
         */
        void setSpeed(float s);

        /**
         * set number of instances
         * @param n number of instances
         */
        void setNumber(unsigned int n);

        /**
         * set zoom factor
         * @param z zoom factor
         */
        void setZoom (float z);

        /**
         * set rotation of instance
         * @param r rotation angle of instances
         */
        void setRotationAll(float r);

        /**
         * set rotation speed of instances
         * @param s rotation speed of instances (in degree per frame)
         */
        void setSpeedAll(float s);

        /**
         * set red
         * @param r red
         */
        void setR(float r);

        /**
         * set green
         * @param g green
         */
        void setG(float g);

        /**
         * set blue
         * @param b blue
         */
        void setB(float b);

        /**
         * set alpha
         * @param a alpha
         */
        void setAlpha(float alpha);

        /**
         * set shutter speed
         * @param s speed
         */
        void setShutterSpeed(int speed);

    private:

        std::string content_type;   // Content type
        int content_data;           // Content data
        float positionX;            // Position X-Richtung
        float positionY;            // Position Y-Richtung
        float scalingX;             // Skalierung X-Richtung    1.0 = screen_width
        float scalingY;             // Skalierung Y-Richtung    <0 = use scalingX and texture Ratio
        float rotation;             // Drehwinkel
        float r_speed;              // Drehgeschwindigkeit °/Frame
        unsigned int number;        // Anzahl der Duplikate
        float zoom;                 // Abstand der Duplikate vom Mittelpunkt
        float rotation_all;         // Drehwinkel der Anordnung der Duplikate
        float r_speed_all;          // Drehgeschwindogkeit der Anordnung der Duplikate
        float r;                    // Rot
        float g;                    // Grün
        float b;                    // Blau
        float alpha;                // Alphakanal
        int shutter_state;          // Shutter Status (0=open; 1=close)
        unsigned int shutter_time;  // Shutter Zeit seit letzter veränderung
        int shutter_speed;          // Shutter Zeit Einstellung
        //animation                 // Steuerung von Videotexturen
    };
}

#endif // CANVAS_H
