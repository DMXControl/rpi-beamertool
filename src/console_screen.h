// console_screen.h

#ifndef CONSOLE_SCREEN_H
#define CONSOLE_SCREEN_H

#include <thread>

namespace Beamertool {

    class ConsoleScreen {

    public:
        /**
         * standard constructor
         */
        ConsoleScreen();

        /**
         * constructor, set silent mode
         * @param silent indicates silent ,ode
         */
        ConsoleScreen(bool silent);

        /**
         * destructor
         */
        ~ConsoleScreen();

        /**
         * signal receiver function
         */
        static void signalReceiver(int signal);

        /**
         * indicates if the programm is intended to quit
         */
        bool isQuit();

        /**
         * prints the title on the screen (not in silent mode)
         */
        void showTitle();

        /**
         * wrapper for ncurses printw (not in silent mode)
         */
        void print(const char* aFormat, ...);

        /**
         * wrapper for ncurses move (not in silent mode)
         */
        void moveTo(int x, int y);

    private:
        /**
         * is set to true if the program is intended to quit
         */
        static volatile bool quit;

        /**
         * is set to true if no output should appear
         */
        bool silent;

        /**
         * extra thread for key listening
         */
        std::thread key_listener_thread;

        /**
         * init and starts signal and key listeners
         * @param s indicates silent mode
         */
        void init(bool s);

        /**
         * wrapper for ncurses initialization
         */
        void initNCurses();

        /**
         * wrapper for ncurses uninitialization
         */
        void endNCurses();

        /**
         * key listener function
         */
        static void keyListener();
    };
}

#endif // CONSOLE_SCREEN_H
