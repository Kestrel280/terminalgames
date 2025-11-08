#include <ncurses.h>
#include "mainmenu.h"
#include "keepaway.h"
#include "string.h"

bool doMainMenu() {
    char* choices[] = {
        "PLAY",
        "QUIT"
    };

    int nchoices = sizeof(choices) / sizeof(char*);
    int choice = 0;
    int key;
    bool quit = false;
    int height, width;
    getmaxyx(stdscr, height, width);

    while (!quit) {
        int y = height / 2;
        clear();
        for (int i = 0; i < nchoices; i++, y++) {
            if (i == choice) {
                attron(A_REVERSE);
                mvaddch(y, (width - strlen(choices[i])) / 2 - 1, '>');
                mvprintw(y, (width - strlen(choices[i])) / 2, "%s", choices[i]);
                attroff(A_REVERSE);
            } else { mvprintw(y, (width - strlen(choices[i])) / 2, "%s", choices[i]); }
        }

        key = getch();

        switch (key) {
            case KEY_UP: choice--; break;
            case KEY_DOWN: choice++; break;
            case '\n': {
                switch (choice) {
                    case 0: gamePlay(); break;
                    case 1: quit = true; break;
                }
            }
        }
        choice = (choice < 0 ? 0 : (choice >= nchoices ? nchoices - 1 : choice));
    };

    return true;
}
