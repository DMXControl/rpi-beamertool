// canvas/canvas_manager.cpp

#include "canvas_manager.h"
#include "canvas.h"
#include "renderer.h"
#include "fps_counter.h"

using namespace std;
using namespace Beamertool;

CanvasManager::CanvasManager() {

    // start subclasses
    this->renderer = new Renderer();
    this->fpsc = new FPS_Counter();

    // reset lists
    this->canvases_list.clear();
    this->canvas_groups.clear();
}

CanvasManager::~CanvasManager() {
    // delete canvases and canvas lists
    for (int i=0; i<(int)this->canvases_list.size(); ++i) {
        delete this->canvases_list[i];
    }
    this->canvases_list.clear();
    for(unsigned int i=0; i < this->canvas_groups.size(); ++i) {
        this->canvas_groups[i].clear();
    }
    this->canvas_groups.clear();

    // delete subclasses
    delete this->renderer;
    delete this->fpsc;
}

int CanvasManager::generateCanvasGroup(int num, string type) {
    // generate Group
    vector<Canvas*> group_list;
    group_list.clear();
    if (num > 0) {
        for (int i=0; i<num; ++i) {
            Canvas* c = new Canvas(type);
            group_list.push_back(c);
            this->canvases_list.push_back(c);
        }
    }
    this->canvas_groups.push_back(group_list);

    // TODO: reuse deletet ID's to keep canvas_groups as small as possible

    // return Group ID (musst be the last entry while there is no reuse of ID's)
    return this->canvas_groups.size() - 1;
}

vector<Canvas*> CanvasManager::getCanvasGroup(int group_id) {
    return this->canvas_groups[group_id];
}

vector<Canvas*> CanvasManager::getAllCanvases() {
    return this->canvases_list;
}

void CanvasManager::renderFrame() {
    this->fpsc->tick();

    for (unsigned int i=0; i < this->canvases_list.size(); ++i) {
        this->canvases_list[i]->frameStep();
    }

    renderer->drawCanvases(this->getAllCanvases());
}

float CanvasManager::getFPS() {
    return this->fpsc->getFPS();
}

void CanvasManager::setZoom(int type) {
    this->renderer->setZoom(type);
}

int CanvasManager::getScreenWidth() {
    return this->renderer->getScreenWidth();
}

int CanvasManager::getScreenHeight() {
    return this->renderer->getScreenHeight();
}

float CanvasManager::getScreenRatio() {
    return this->renderer->getScreenRatio();
}
