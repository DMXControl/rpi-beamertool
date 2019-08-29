// console_screen.cpp

#include <csignal>
#include <cstdarg>
#include <thread>
#include <ncurses.h>
#include "console_screen.h"

using namespace std;
using namespace Beamertool;

volatile bool ConsoleScreen::quit = false;

ConsoleScreen::ConsoleScreen() {
    init(false);
}

ConsoleScreen::ConsoleScreen(bool silent) {
    init(silent);
}

void ConsoleScreen::init(bool s) {
    quit = false;
    silent = s;

    // Signal Listener
    signal(SIGINT, ConsoleScreen::signalReceiver);
    signal(SIGTERM, ConsoleScreen::signalReceiver);

    if (!silent) {
        // Start NCurses
        initNCurses();

        // Start Key Listener
        //key_listener_thread = thread(keyListener);
    }
}

void ConsoleScreen::initNCurses() {
    initscr();
    curs_set(0);            // Cursor unsichtbar
    noecho();               // Keine Ausgabe bei Tastendruck
    keypad(stdscr,TRUE);    // Sondertasten behandeln
    clear();
    //cbreak();
    timeout(-1);
}

ConsoleScreen::~ConsoleScreen() {
    if (!silent) {
        //key_listener_thread.join();
        endNCurses();
    }
}

void ConsoleScreen::endNCurses() {
    endwin();
}

void ConsoleScreen::signalReceiver(int signal) {
    quit = true;
}

void ConsoleScreen::keyListener() {
    int key;
    bool loop = true;
    while(loop) {
        key=getch();
        if (key == 'q') {
            loop = false;
            quit = 1;
        }
    }
}

bool ConsoleScreen::isQuit() {
    return quit;
}

void ConsoleScreen::showTitle() {
    if (!this->silent) {
        attrset(A_BOLD);
        printw("Raspberry Pi Beamertool | Version: "VERSION_STRING"\n");
        refresh();
        attrset(A_NORMAL);
    }
}

void ConsoleScreen::print(const char* aFormat, ...) {
    if (!silent) {
        va_list argptr;
        va_start(argptr, aFormat);
        vwprintw(stdscr, aFormat, argptr);
        refresh();
        va_end(argptr);
    }
}

void ConsoleScreen::moveTo(int x, int y) {
    if (!silent) {
        move(x, y);
    }
}

