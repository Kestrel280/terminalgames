#include <stdio.h>
#include <ncurses.h>
#include "pong.h"

const int screenW;
const int screenH;

int main(int argc, char* argv[]) {
    initscr();
    initColors();
    playGame();

    getch();
    endwin();
    
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
    player->pad.vx = 0;
    player->pad.vy = 0;
    player->pad.colorpair = color;
}

/* Game logic */
void playGame() {
    Game game;
    initGame(&game);
    game.state = GAME_STATE_READY;

    while (!game.finished) {
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

        drawGame(game);
    }
}

void gameTick(Game* game) {
    return;
}


/* Drawing */
void drawPaddle(Paddle pad) {
    attron(COLOR_PAIR(pad.colorpair));
    for (int j = 0; j < pad.width; j++) {
        for (int i = 0; i < pad.height; i++) {
            mvaddch(i + pad.y, j + pad.x, '#');
        }
    }
    attroff(COLOR_PAIR(pad.colorpair));
}

void drawBorder() {
    return;
}

void drawGame(Game game) {
    drawBorder();
    drawPaddle(game.p1.pad);
    drawPaddle(game.p2.pad);
    refresh();
}
