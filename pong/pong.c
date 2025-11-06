#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include "pong.h"

int clamp(int in, int lo, int hi) {
    if (in < lo) return lo;
    else if (in > hi) return hi;
    else return in;
}


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
    init_pair(COLOR_BG, COLOR_BACKGROUND_FG, COLOR_BACKGROUND_BG);
    init_pair(COLOR_PLAYER_PADDLE, COLOR_PLAYER_PADDLE_FG, COLOR_PLAYER_PADDLE_BG);
    init_pair(COLOR_OPPONENT_PADDLE, COLOR_OPPONENT_PADDLE_FG, COLOR_OPPONENT_PADDLE_BG);
    init_pair(COLOR_BALL, COLOR_BALL_FG, COLOR_BALL_BG);
    init_pair(COLOR_WALL, COLOR_WALL_FG, COLOR_WALL_BG);
    init_pair(COLOR_TRIM, COLOR_TRIM_FG, COLOR_TRIM_BG);
}

void initGame(Game* game) {
    game->state = GAME_STATE_READY;
    game->lastScore = -1;
    initPlayer(&game->p1, COLOR_PLAYER_PADDLE, 3);
    initPlayer(&game->p2, COLOR_OPPONENT_PADDLE, screenW - 3);
    game->ball.subx = screenW / 2 * BALL_SUBTICKS;
    game->ball.suby = screenH / 2 * BALL_SUBTICKS;
    game->ball.x = game->ball.subx / BALL_SUBTICKS;
    game->ball.y = game->ball.suby / BALL_SUBTICKS;
    game->ball.subvx = -2 * BALL_SUBTICKS;
    game->ball.subvy = 1 * BALL_SUBTICKS;
    game->ball.colorpair = COLOR_BALL;
    game->ball.lastCol = -1;
    game->finished = false;
    game->pauseCounter = READY_TICKS;
}

void initPlayer(Player* player, int colorpair, int x) {
    player->score = 0;
    player->pad.width = 1;
    player->pad.height = 5;
    player->pad.y = screenH / 2;
    player->pad.x = x;
    player->pad.vy = 0;
    player->pad.vx = 0;
    player->pad.colorpair = colorpair;
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
        while (timeSinceLastTickUs > (uint64_t)(TICK_TIME_US)) { 
            timeSinceLastTickUs -= (uint64_t)(TICK_TIME_US); 
            switch(game.state) {
                case GAME_STATE_READY: 
                    if (--game.pauseCounter <= 0) { game.state = GAME_STATE_PLAYING; }
                    break;
                case GAME_STATE_PLAYING:
                    gameTick(&game);
                    break;
                case GAME_STATE_SCORED:
                    if (--game.pauseCounter <= 0) { game.pauseCounter = READY_TICKS; game.state = GAME_STATE_READY; }
                    break;
            }
        }

        game.frac = ((float)timeSinceLastTickUs / (float)TICK_TIME_US);

        drawGame(game);
    }
}

void score(Game* game, int player) {
    if (player == 1) game->p1.score++;
    else if (player == 2) game->p2.score++;
    if ((game->p1.score > WIN_SCORE) || (game->p2.score > WIN_SCORE)) winGame(game, player);
    game->lastScore = player;
    game->state = GAME_STATE_SCORED;
    game->pauseCounter = SCORED_TICKS;
    game->p1.pad.x = 3;
    game->p2.pad.x = screenW - 3;
    game->p1.pad.y = screenH / 2;
    game->p2.pad.y = screenH / 2;
    game->p1.pad.vy = 0;
    game->p1.pad.vx = 0;
    game->p2.pad.vy = 0;
    game->p2.pad.vx = 0;
    game->ball.lastCol = -1;
    game->ball.subx = screenW / 2 * BALL_SUBTICKS;
    game->ball.suby = screenH / 2 * BALL_SUBTICKS;
    game->ball.subvx = -2 * BALL_SUBTICKS;
    game->ball.subvy = 1 * BALL_SUBTICKS;
    game->ball.x = game->ball.subx / BALL_SUBTICKS;
    game->ball.y = game->ball.suby / BALL_SUBTICKS;
    return;
}

void winGame(Game* game, int player) {
    game->finished = true;
}

void gameTick(Game* game) {
    // Update paddle positions, ensuring they stay in bounds
    game->p1.pad.y += game->p1.pad.vy;
    game->p2.pad.y += game->p2.pad.vy;
    game->p1.pad.x += game->p1.pad.vx;
    game->p2.pad.x += game->p2.pad.vx;
    game->p1.pad.y = clamp(game->p1.pad.y, BORDER_THICKNESS - 1, screenH - game->p1.pad.height);
    game->p2.pad.y = clamp(game->p2.pad.y, BORDER_THICKNESS - 1, screenH - game->p2.pad.height);
    game->p1.pad.x = clamp(game->p1.pad.x, BORDER_THICKNESS - 1, screenW / 3 - game->p1.pad.width);
    game->p2.pad.x = clamp(game->p2.pad.x, 2 * screenW / 3, screenW - game->p2.pad.width);
    game->p1.pad.vy = 0;
    game->p2.pad.vy = 0;
    game->p1.pad.vx = 0;
    game->p2.pad.vx = 0;

    // Update ball position
    game->ball.subx += game->ball.subvx;
    game->ball.suby += game->ball.subvy;
    int newX = game->ball.subx / BALL_SUBTICKS;
    int newY = game->ball.suby / BALL_SUBTICKS;

    // Paddle collisions
    if ((game->ball.lastCol != 1)
        && (newX <= (game->p1.pad.x + game->p1.pad.width))          // new x in/left-of pad 1's right edge
        && (game->ball.x >= game->p1.pad.x)                         // old x in/right-of pad 1's left edge
        && (newY >= (game->p1.pad.y))                               // new Y in/below pad 1's top edge
        && (game->ball.y <= (game->p1.pad.y + game->p1.pad.height)))// old Y in/above pad 1's bottom edge
    {
        newX = (game->p1.pad.x + game->p1.pad.width);
        game->ball.subvx *= -1;
        game->ball.lastCol = 1;
    }
    else if ((game->ball.lastCol != 1)
        && (newX >= game->p2.pad.x)                                 // new x in/right-of pad 2's left edge
        && (game->ball.x <= (game->p2.pad.x + game->p2.pad.width))  // old x in/left-of pad 2's right edge
        && (newY >= (game->p2.pad.y))                               // new Y in/below pad 2's top edge
        && (game->ball.y <= (game->p2.pad.y + game->p2.pad.height)))// old Y in/above pad 2's bottom edge
    {
        newX = (game->p1.pad.x + game->p1.pad.width);
        game->ball.subvx *= -1;
        game->ball.lastCol = 2;
    }

    // Ceiling/floor collisions
    if (newY <= BORDER_THICKNESS) (game->ball.subvy *= -1);
    else if (newY >= (screenH - BORDER_THICKNESS)) (game->ball.subvy *= -1);

    // Check for score
    if (newX <= 0) score(game, 2);
    else if (newX >= screenW) score(game, 1);
    else { game->ball.x = newX; game->ball.y = newY; }
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
    drawScore(game);
    drawTrim();
    drawPaddle(game.p1.pad, game.frac);
    drawPaddle(game.p2.pad, game.frac);
    drawBall(game.ball, game.frac);
    switch (game.state) {
        case GAME_STATE_READY: drawOverlay("READY"); break;
        case GAME_STATE_SCORED: char buf[20]; sprintf(buf, "P%d SCORED", game.lastScore); buf[19] = '\x00'; drawOverlay(buf); break;
    }
    refresh();
}

void drawOverlay(const char* msg) {
    mvprintw(screenH / 2, screenW / 2 - strlen(msg) / 2, msg);
}

void drawTrim() {
    int x = screenW / 3, y;
    attron(COLOR_PAIR(COLOR_TRIM));
    for (y = 1; y < screenH; y++) mvaddch(y, x, '|');
    x = 2 * screenW / 3;
    for (y = 1; y < screenH; y++) mvaddch(y, x, '|');
    attroff(COLOR_PAIR(COLOR_TRIM));
    return;
}

void drawBorder() {
    int x, y = 0;
    attron(COLOR_PAIR(COLOR_WALL));
    for (x = 0; x < screenW + 1; x++) mvaddch(y, x, 'x');
    y = screenH;
    for (x = 0; x < screenW + 1; x++) mvaddch(y, x, 'x');
    x = 0;
    for (y = 0; y < screenH; y++) mvaddch(y, x, 'x');
    x = screenW;
    for (y = 0; y < screenH; y++) mvaddch(y, x, 'x');
    attroff(COLOR_PAIR(COLOR_WALL));
    return;
}

void drawPaddle(Paddle pad, float frac) {
    int x = pad.x + (int)((float)pad.vx * frac);
    int y = pad.y + (int)((float)pad.vy * frac);
    attron(COLOR_PAIR(pad.colorpair));
    for (int j = 0; j < pad.width; j++) {
        for (int i = 0; i < pad.height; i++) {
            mvaddch(i + y, j + x, '#');
        }
    }
    attroff(COLOR_PAIR(pad.colorpair));
}

void drawBall(Ball ball, float frac) {
    int x = ball.x + (int)((float)ball.subvx * frac / (float)BALL_SUBTICKS);
    int y = ball.y + (int)((float)ball.subvy * frac / (float)BALL_SUBTICKS);
    attron(COLOR_PAIR(ball.colorpair));
    mvaddch(y, x, 'O');
    attroff(COLOR_PAIR(ball.colorpair));
    return;
}

void drawScore(Game game) {
    char buf[20];
    sprintf(buf, "PLAYER 1: %d", game.p1.score);
    mvprintw(3, 3, buf);
    sprintf(buf, "PLAYER 2: %d", game.p2.score);
    mvprintw(3, screenW - 3 - strlen(buf), buf);
    return;
}
