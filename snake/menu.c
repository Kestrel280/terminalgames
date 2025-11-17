#include <assert.h>
#include <ncurses.h>
#include <string.h>
#include "menu.h"

const char* const menuString[] = {
    [MENU_OPTION_PLAY]          = "PLAY",
    [MENU_OPTION_LEADERBOARD]   = "VIEW LEADERBOARD",
    [MENU_OPTION_QUIT]          = "QUIT"
};

static_assert(sizeof(menuString) / sizeof(const char *) == MENU_OPTION_META_NUM_OPTIONS, "incorrect # of menu string options in menu.c");

void _menuDraw(int choice) {
    int width, height;
    erase();
    getmaxyx(stdscr, height, width);
    for (int i = 0; i < MENU_OPTION_META_NUM_OPTIONS; i++) {
        const char* str = menuString[i];
        int lpos = (width - strlen(str)) / 2;
        if (i == choice) { attron(A_REVERSE); mvaddch(height / 2 + i, lpos - 1, '>'); }
        mvprintw(height / 2 + i, lpos, "%s", menuString[i]);
        if (i == choice) attroff(A_REVERSE);
    }
}

MenuOption menu() {
    int choice = 0;
    _menuDraw(choice);

    while (1) {
        switch (int ch = getch()) {
            case KEY_UP: choice--; break;
            case KEY_DOWN: choice++; break;
            case '\n': return choice;
        }
        choice = choice < 0 ? 0 : choice;
        choice = choice >= MENU_OPTION_META_NUM_OPTIONS ? MENU_OPTION_META_NUM_OPTIONS - 1 : choice;

        _menuDraw(choice);
    }
    return MENU_OPTION_PLAY;
}
