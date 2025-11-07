#include <ncurses.h>
#include "keepaway.h"
#include "mainmenu.h"
#include "keepaway.h"

int main() {
    initscr(); // start ncurses
    noecho(); // ncurses disable echo
    keypad(stdscr, TRUE); // allow ncurses to capture arrowkeys
    cbreak();
    curs_set(0); // invisible cursor

    while (1) {
        if (doMainMenu()) break;
    }

    endwin();   // end ncurses
    return 0;
}
