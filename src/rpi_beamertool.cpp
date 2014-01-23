/**
 * Raspberry Pi Beamertool
 * Copyright (c) 2013-2014 Moritz Heinemann <mo@blog-srv.net>
 * for additional copyright notices, read the LICENSE file!
 */

// Includes
#include <cstring>
#include <iostream>
#include "console_screen.h"
#include "config_loader.h"
#include "canvas/canvas_manager.h"
#include "artnet_receiver.h"

using namespace std;
using namespace Beamertool;

int main (int argc, char **argv) {

    // Console Screen
    ConsoleScreen * c_screen = NULL;

    // Parse argv
    for(int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            cout << "Raspberry Pi Beamertool | Version: "VERSION_STRING << endl;
            return 0;
        }
        if (strcmp(argv[i], "--silent") == 0) {
            // Start Console Screen silent
            c_screen = new ConsoleScreen(true);
        }
    }

    if (c_screen == NULL) {
        // If Console Screen was not startet above, start it here not silent.
        c_screen = new ConsoleScreen();
    }

    // Title
    c_screen->showTitle();

    // Load Config
    ConfigLoader * config = new ConfigLoader();
    c_screen->print("Config: Subnet:%i Universe:%i Num:%i DMX:%i Zoom:%i\n", config->getArtnetSubnet(), config->getArtnetUniverse(), config->getLasersimNum(), config->getLasersimDMXStart(), config->getZoomType());

    // Init Canvas Manager
    CanvasManager * screen = new CanvasManager();
    screen->setZoom(config->getZoomType());
    c_screen->print("Screen: %i x %i\n", screen->getScreenWidth(), screen->getScreenHeight());

    // Init Lasersim on ArtnetReceiver
    int lasers_id = screen->generateCanvasGroup(config->getLasersimNum(), "CONTENT_TEXTURE_256");
    ArtnetReceiver * dmxr = new ArtnetReceiver(screen, lasers_id, config->getArtnetUniverse(), config->getArtnetSubnet(), config->getLasersimDMXStart(), config->getScalingMultiplier(), config->getLedGpioPin());

    c_screen->print("Init Done! Starting main loop... Press 'q' to quit! \n");

    // Main Loop
    while (!c_screen->isQuit()) {
        screen->renderFrame();
        c_screen->moveTo(4, 0);
        c_screen->print(" FPS: %5.2f ", screen->getFPS());
    }

    delete dmxr;
    delete screen;
    delete config;
    delete c_screen;
    return 0;
}
