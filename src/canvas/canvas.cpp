// canvas/canvas.cpp

#include "canvas.h"

using namespace std;
using namespace Beamertool;

Canvas::Canvas(string content_type) {
    this->content_type = content_type;
    content_data = 0;
    positionX = 0.0f;
    positionY = 0.0f;
    scalingX = 1.0f;
    scalingY = 1.0f;
    rotation = 0.0f;
    r_speed = 0.0f;
    number = 0;
    zoom = 0.0f;
    rotation_all = 0.0f;
    r_speed_all = 0.0f;
    r = 1.0f;
    g = 1.0f;
    b = 1.0f;
    alpha = 1.0f;
    shutter_state = 0;
    shutter_time = 0;
    shutter_speed = 0;
}

Canvas::~Canvas() {

}

void Canvas::frameStep() {

    this->rotation = correctAngleRange(this->rotation + this->r_speed);
    this->rotation_all = correctAngleRange(this->rotation_all + this->r_speed_all);

    //Shutter
    if (this->shutter_speed > 0) {
        this->shutter_time++;
        if (this->shutter_time > (64 - this->shutter_speed/4)) {
            if (this->shutter_state == 0) {
                this->shutter_state = 1;
            } else {
                this->shutter_state = 0;
            }
            this->shutter_time = 0;
        }
    }
}

float Canvas::correctAngleRange(float angle) {
    while(angle >= 360.f) {
        angle -= 360.f;
    }
    while(angle < 0.f) {
        angle += 360.f;
    }
    return angle;
}

string Canvas::getContentType() {
    return this->content_type;
}

int Canvas::getContentData() {
    return this->content_data;
}

float Canvas::getPositionX() {
    return this->positionX;
}

float Canvas::getPositionY() {
    return this->positionY;
}

float Canvas::getScalingX() {
    return this->scalingX;
}

float Canvas::getScalingY() {
    return this->scalingY;
}

float Canvas::getRotation() {
    return this->rotation;
}

unsigned int Canvas::getNumber() {
    return this->number;
}

float Canvas::getZoom() {
    return this->zoom;
}

float Canvas::getRotationAll() {
    return this->rotation_all;
}

float Canvas::getR() {
    return this->r;
}

float Canvas::getG() {
    return this->g;
}

float Canvas::getB() {
    return this->b;
}

float Canvas::getAlpha() {
    return this->alpha;
}

int Canvas::getShutterState() {
    if (this->shutter_speed == 0) {
        return 0;
    } else {
        return this->shutter_state;
    }
}

void Canvas::setContentData(int data) {
    this->content_data = data;
}

void Canvas::setPositionX(float x) {
    this->positionX = x;
}

void Canvas::setPositionY(float y) {
    this->positionY = y;
}

void Canvas::setScalingX(float x) {
    this->scalingX = x;
}

void Canvas::setScalingY(float y) {
    this->scalingY = y;
}

void Canvas::setRotation(float r) {
    this->rotation = correctAngleRange(r);
}

void Canvas::setSpeed(float s) {
    this->r_speed = s;
}

void Canvas::setNumber(unsigned int n) {
    this->number = n;
}

void Canvas::setZoom(float z) {
    this->zoom = z;
}

void Canvas::setRotationAll(float r) {
    this->rotation_all = correctAngleRange(r);
}

void Canvas::setSpeedAll(float s) {
    this->r_speed_all = s;
}

void Canvas::setR(float r) {
    if (r < 0.f) {
        this->r = 0.f;
    } else if (r > 1.f) {
        this->r = 1.f;
    } else {
        this->r = r;
    }
}

void Canvas::setG(float g) {
    if (g < 0.f) {
        this->g = 0.f;
    } else if (g > 1.f) {
        this->g = 1.f;
    } else {
        this->g = g;
    }
}

void Canvas::setB(float b) {
    if (b < 0.f) {
        this->b = 0.f;
    } else if (b > 1.f) {
        this->b = 1.f;
    } else {
        this->b = b;
    }
}

void Canvas::setAlpha(float alpha) {
    if (alpha < 0.f) {
        this->alpha = 0.f;
    } else if (alpha > 1.f) {
        this->alpha = 1.f;
    } else {
        this->alpha = alpha;
    }
}

void Canvas::setShutterSpeed(int speed) {
    if (speed > 0) {
        this->shutter_speed = speed;
    } else {
        this->shutter_speed = 0;
    }
}
