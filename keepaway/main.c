#if defined(__CYGWIN__) || defined(_WIN64)
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include "keepaway.h"
#include "mainmenu.h"

void initColors() {
    if (has_colors() == false) { fprintf(stderr, "failed to init colors\n"); return; }
    start_color();
    init_pair(COLOR_CELL_EMPTY, COLOR_CELL_EMPTY_FG, COLOR_CELL_EMPTY_BG);
    init_pair(COLOR_CELL_PERM_WALL, COLOR_CELL_PERM_WALL_FG, COLOR_CELL_PERM_WALL_BG);
    init_pair(COLOR_CELL_TEMP_WALL, COLOR_CELL_TEMP_WALL_FG, COLOR_CELL_TEMP_WALL_BG);
    init_pair(COLOR_CELL_START, COLOR_CELL_START_FG, COLOR_CELL_START_BG);
    init_pair(COLOR_CELL_END, COLOR_CELL_END_FG, COLOR_CELL_END_BG);
    init_pair(COLOR_REDGUY, COLOR_REDGUY_FG, COLOR_REDGUY_BG);
    init_pair(COLOR_PLAYER, COLOR_PLAYER_FG, COLOR_PLAYER_BG);
}

int main() {
    initscr(); // start ncurses
    initColors();
    noecho(); // ncurses disable echo
    keypad(stdscr, TRUE);
    cbreak();
    curs_set(0); // invisible cursor

    while (1) {
        if (doMainMenu()) break;
    }

    endwin();   // end ncurses
    return 0;
}
