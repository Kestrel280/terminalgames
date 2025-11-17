#include <ncurses.h>
#include <stdio.h>
#include <sys/time.h>
#include <inttypes.h>
#include "snake.h"

int gamePlay() {
    nodelay(stdscr, TRUE); // disable blocking on getch()

    Game game;
    game.state = GAME_STATE_ACTIVE;
    game.curDir = SOUTH;
    game.width = 15;
    game.height = 15;

    struct timeval tv_prev, tv_now;
    gettimeofday(&tv_prev, NULL);
    while (game.state != GAME_STATE_STOPPED) {
        gettimeofday(&tv_now, NULL);
        uint64_t dtUs = 1000000ul * ((uint64_t)tv_now.tv_usec - (uint64_t)tv_prev.tv_usec) + ((uint64_t)tv_now.tv_sec - (uint64_t)tv_prev.tv_sec);
        tv_prev = tv_now;

        gameHandleInput(&game);

        switch (game.state) {
            case GAME_STATE_ACTIVE: gameTick(&game, dtUs); break;
        }
    }

    return 0;
}

void gameTick(Game* game, uint64_t dtUs) {
    game->state = GAME_STATE_STOPPED;
    return;
}

void gameHandleInput(Game* game) {
    int key = getch();
    switch (key) {
        case ERR: return;
        case KEY_UP: gameTrySetDir(game, NORTH); break;
        case KEY_DOWN: gameTrySetDir(game, SOUTH); break;
        case KEY_RIGHT: gameTrySetDir(game, EAST); break;
        case KEY_LEFT: gameTrySetDir(game, WEST); break;
    }
    return;
}

void gameTrySetDir(Game* game, Direction newDir) {
    return;
}
