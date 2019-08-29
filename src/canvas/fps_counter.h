// canvas/fps_counter.h

#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#define FPS_DELAY_LIST_LEN 10

namespace Beamertool {

    class FPS_Counter {

    public:
        /**
         * standard constructor
         */
        FPS_Counter();

        /**
         * call once for every generatet Frame
         */
        void tick();

        /**
         * get the calculated Framerate
         */
        float getFPS();

    private:
        long sec;                               // seconds since last frame
        long usec;                              // useconds since last frame
        float delay_list[FPS_DELAY_LIST_LEN];   // list of the delay betwen the last frames
        int list_pos;                           // save position in the list to use as a circular buffer
    };
}

#endif // FPS_COUNTER_H
