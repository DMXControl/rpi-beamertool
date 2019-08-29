// canvas/fps_counter.cpp

#include <sys/time.h>
#include <time.h>
#include "fps_counter.h"

using namespace std;
using namespace Beamertool;

FPS_Counter::FPS_Counter() {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    this->sec = (long) tv.tv_sec;
    this->usec = (long) tv.tv_usec;
    this->list_pos = 0;
    for (int i = 0; i < FPS_DELAY_LIST_LEN; ++i) {
        delay_list[i] = 0.0f;
    }
}

void FPS_Counter::tick() {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    long sec = tv.tv_sec - this->sec;
    long usec = tv.tv_usec - this->usec;
    this->sec = tv.tv_sec;
    this->usec = tv.tv_usec;
    this->delay_list[list_pos] = (((float) sec) + (((float) usec) / 1000000.f));
    ++list_pos;
    list_pos %= FPS_DELAY_LIST_LEN;
}

float FPS_Counter::getFPS() {
    float sum = 0.0f;
    for (int i = 0; i < FPS_DELAY_LIST_LEN; ++i) {
        sum += delay_list[i];
    }
    sum /= (float) FPS_DELAY_LIST_LEN;
    return 1.0f / sum;
}
