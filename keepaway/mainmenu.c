#include <ncurses.h>
#include "mainmenu.h"
#include "keepaway.h"
#include "string.h"

bool doMainMenu() {
    char* difficulties[] = {
        "EASY",
        "NORMAL",
        "HARD"
    };

    char* sizes[] = {
        "SMALL",
        "MEDIUM",
        "LARGE"
    };

    char* choices[] = {
        "PLAY",
        "DIFFICULTY",
        "SIZE",
        "LAYOUT",
        "QUIT"
    };

    int nchoices = sizeof(choices) / sizeof(char*);
    int ndifficulties = sizeof(difficulties) / sizeof(char*);
    int nsizes = sizeof(sizes) / sizeof(char*);

    int choice = 0;
    int difficulty = 0;
    int size = 0;
    bool portrait = false;

    int key;
    bool quit = false;
    int height, width;
    getmaxyx(stdscr, height, width);

    while (!quit) {
        int y = height / 2;
        erase();
        for (int i = 0; i < nchoices; i++, y++) {
            if (i == choice) {
                attron(A_REVERSE);
                mvaddch(y, (width - strlen(choices[i])) / 2 - 1, '>');
                mvprintw(y, (width - strlen(choices[i])) / 2, "%s", choices[i]);
                attroff(A_REVERSE);
            } else { mvprintw(y, (width - strlen(choices[i])) / 2, "%s", choices[i]); }
        }

        ++y;
        mvprintw(++y, (width - strlen("--- GAME SETTINGS ---")) / 2, "%s", "--- GAME SETTINGS ---");
        mvprintw(++y, (width - strlen(difficulties[difficulty])) / 2, "%s", difficulties[difficulty]);
        mvprintw(++y, (width - strlen(sizes[size])) / 2, "%s", sizes[size]);
        mvprintw(++y, (width - 8) / 2, "%s", portrait ? "PORTRAIT" : "LANDSCAPE");

        key = getch();

        switch (key) {
            case KEY_UP: choice--; break;
            case KEY_DOWN: choice++; break;
            case '\n': {
                switch (choice) {
                    case 0: {
                        int nrows = GAME_BASE_SIZE * (size + 1);
                        int ncols = nrows * 3;
                        if (portrait) { int tmp = nrows; nrows = ncols * 3 / 5; ncols = tmp; }
                        if (nrows > (height - 10)) nrows = height - 10;
                        if (ncols > (width - 10)) ncols = width - 10;
                        gamePlay(nrows, ncols, difficulty);
                        break;
                    }
                    case 1: difficulty = (difficulty + 1) % ndifficulties; break;
                    case 2: size = (size + 1) % nsizes; break;
                    case 3: portrait = !portrait; break;
                    case 4: quit = true; break;
                }
            }
        }
        choice = (choice < 0 ? 0 : (choice >= nchoices ? nchoices - 1 : choice));
    };

    return true;
}
