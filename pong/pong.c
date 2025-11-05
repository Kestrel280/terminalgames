#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include "pong.h"

const int screenW;
const int screenH;

int main(int argc, char* argv[]) {
    initscr(); // ncurses init
    initColors();
    curs_set(0); // hide cursor
    keypad(stdscr, TRUE); // enable capturing key_up, etc
    nodelay(stdscr, TRUE); // getch() becomes nonblocking
    playGame();

    endwin(); // ncurses end
    
    return 0;
}

/* Setup */
void initColors() {
    start_color();
    if (has_colors() == false) return;
    init_pair(COLOR_PLAYER_PADDLE, COLOR_PLAYER_PADDLE_FG, COLOR_PLAYER_PADDLE_BG);
    init_pair(COLOR_OPPONENT_PADDLE, COLOR_OPPONENT_PADDLE_FG, COLOR_OPPONENT_PADDLE_BG);
}

void initGame(Game* game) {
    game->state = GAME_STATE_READY;
    initPlayer(&game->p1, COLOR_PLAYER_PADDLE, 3);
    initPlayer(&game->p2, COLOR_OPPONENT_PADDLE, screenW - 3);
    game->finished = false;
    game->pauseCounter = 100;
}

void initPlayer(Player* player, int color, int x) {
    player->score = 0;
    player->pad.width = 1;
    player->pad.height = 5;
    player->pad.y = screenH / 2;
    player->pad.x = x;
    player->pad.vy = 0;
    player->pad.vx = 0;
    player->pad.colorpair = color;
}

/* Game logic */
void playGame() {
    Game game;
    initGame(&game);
    uint64_t dtUs;
    uint64_t timeSinceLastTickUs = 0;
    struct timeval tvnow, tvlast;
    gettimeofday(&tvlast, NULL);

    while (!game.finished) {
        gettimeofday(&tvnow, NULL);
        dtUs = 1000000 * ((uint64_t)tvnow.tv_sec - (uint64_t)tvlast.tv_sec) + ((uint64_t)tvnow.tv_usec - (uint64_t)tvlast.tv_usec);
        timeSinceLastTickUs += dtUs;
        tvlast = tvnow;
        gameHandleInput(&game);
        while (timeSinceLastTickUs > (uint64_t)(16666)) { 
            timeSinceLastTickUs -= (uint64_t)(16666); 
            switch(game.state) {
                case GAME_STATE_READY: 
                    if (--game.pauseCounter <= 0) { game.pauseCounter = 100; game.state = GAME_STATE_PLAYING; }
                    break;
                case GAME_STATE_PLAYING:
                    gameTick(&game);
                    break;
                case GAME_STATE_SCORED:
                    if (--game.pauseCounter <= 0) { game.pauseCounter = 100; game.state = GAME_STATE_READY; }
                    break;
            }
        }

        drawGame(game);
    }
}

void gameTick(Game* game) {
    // Update paddle positions
    game->p1.pad.y += game->p1.pad.vy;
    game->p2.pad.y += game->p2.pad.vy;
    game->p1.pad.x += game->p1.pad.vx;
    game->p2.pad.x += game->p2.pad.vx;
    game->p1.pad.vy = 0;
    game->p2.pad.vy = 0;
    game->p1.pad.vx = 0;
    game->p2.pad.vx = 0;
    return;
}

void gameHandleInput(Game* game) {
    if (game->state != GAME_STATE_PLAYING) { while (getch() != ERR); return; }

    int key;

    while ((key = getch()) != ERR) {
        switch (key) {
            case KEY_UP: game->p1.pad.vy -= 1; break;
            case KEY_DOWN: game->p1.pad.vy += 1; break;
            case KEY_RIGHT: game->p1.pad.vx += 1; break;
            case KEY_LEFT: game->p1.pad.vx -= 1; break;
        }
    }

    return;
}

/* Drawing */
void drawGame(Game game) {
    erase();
    drawBorder();
    drawPaddle(game.p1.pad);
    drawPaddle(game.p2.pad);
    drawBall(game.ball);
    switch (game.state) {
        case GAME_STATE_READY: drawOverlay("READY"); break;
        case GAME_STATE_SCORED: drawOverlay("SCORED"); break;
    }
    refresh();
}

void drawOverlay(const char* msg) {
    mvprintw(screenH / 2, screenW / 2 - strlen(msg) / 2, msg);
}

void drawBorder() {
    return;
}

void drawPaddle(Paddle pad) {
    attron(COLOR_PAIR(pad.colorpair));
    for (int j = 0; j < pad.width; j++) {
        for (int i = 0; i < pad.height; i++) {
            mvaddch(i + pad.y, j + pad.x, '#');
        }
    }
    attroff(COLOR_PAIR(pad.colorpair));
}

void drawBall(Ball ball) {
    return;
}
