#include <ncurses.h>
#include <stdio.h>
#include "snake.h"
#include "menu.h"
#include "leaderboard.h"

#define REGISTER_PIXEL(id, fg, bg, c) init_pair(id, fg, bg); gfxChars[id] = c;
extern char gfxChars[];
void initColors() {
    if (has_colors() == false) { fprintf(stderr, "failed to init colors\n"); return; }
    start_color();
    REGISTER_PIXEL(CG_SPACE, COLOR_WHITE, COLOR_WHITE, ' ');
    REGISTER_PIXEL(CG_WALL, COLOR_RED, COLOR_GREEN, '#');
    REGISTER_PIXEL(CG_SNAKE, COLOR_CYAN, COLOR_BLACK, '.');
    REGISTER_PIXEL(CG_SNAKEHEAD, COLOR_CYAN, COLOR_BLACK, 'o');
    REGISTER_PIXEL(CG_PICKUP, COLOR_WHITE, COLOR_BLACK, '*');
}

int main(int argc, char* argv[]) {
    initscr();
    initColors();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    MenuOption mo;
    while ((mo = menu()) != MENU_OPTION_QUIT) {
        switch (mo) {
            case MENU_OPTION_PLAY: gamePlay(); break;
            case MENU_OPTION_LEADERBOARD: leaderboardDisplay(); break;
        }
    }
    
    // do cleanup here
    endwin();

    return 0;
}
