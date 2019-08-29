// canvas/canvas_manager.h

#ifndef CANVAS_MANAGER_H
#define CANVAS_MANAGER_H

#include <vector>
#include <string>
#include "canvas_classes.h"

namespace Beamertool {

    class CanvasManager {

    public:
        /**
         * standard constructor
         */
        CanvasManager();

        /**
         * destructor
         */
        ~CanvasManager();

        /**
         * generate a group of canvases (so each input plugin can access its own canvas group)
         * @param num the number of canvases in the group
         * @param type content type of the canvases
         * @return id of the group
         */
        int generateCanvasGroup(int num, std::string type);

        /**
         * get all Canvases of a Group
         * @param group_id the id of the group
         * @return a vector of pointers to the canvases of the group with group_id
         */
        std::vector<Canvas*> getCanvasGroup(int group_id);

        /**
         * get all Canvases
         * @return a vector of pointers to all canvases
         */
        std::vector<Canvas*> getAllCanvases();

        /**
         * make a Frame with all Canvases and display on the Screen
         */
        void renderFrame();

        /**
         * get the current Frame rate
         * @return frames per second
         */
        float getFPS();

        /**
         * set zooming type of the zoom function
         */
        void setZoom(int type);

        /**
         * get screen width
         * @return the screen width
         */
        int getScreenWidth();

        /**
         * get screen height
         * @return the screen height
         */
        int getScreenHeight();

        /**
         * get screen ratio
         * @return the screen ratio
         */
        float getScreenRatio();

    private:
        std::vector< std::vector<Canvas*> > canvas_groups;  // Canvas Groups
        std::vector<Canvas*> canvases_list;                 // All canvases
        Renderer* renderer;                                 // The renderer
        FPS_Counter* fpsc;                                  // The FPS Counter
    };
}

#endif // CANVAS_MANAGER_H
