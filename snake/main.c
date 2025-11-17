#include <ncurses.h>
#include <stdio.h>
#include "snake.h"
#include "menu.h"
#include "leaderboard.h"

void initColors() {
    if (has_colors() == false) { fprintf(stderr, "failed to init colors\n"); return; }
    start_color();
}

int main(int argc, char* argv[]) {
    initscr();
    initColors();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    
    MenuOption mo;
    while (mo = menu() != MENU_QUIT) {
        switch (mo) {
            case MENU_PLAY: gamePlay(); break;
            case MENU_LEADERBOARD: leaderboardDisplay(); break;
        }
    }
    
    // do cleanup here
    endwin();

    return 0;
}
