#if defined(_WIN32) || defined(__CYGWIN__)
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

#include <stdio.h>
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
    player->pad.width = 2;
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
                case GAME_STATE_GAMEOVER:
                    if (--game.pauseCounter <= 0) game.finished = true;
            }
        }

        game.frac = ((float)timeSinceLastTickUs / (float)TICK_TIME_US);

        drawGame(game);
    }
}

void score(Game* game, int player) {
    if (player == 1) game->p1.score++;
    else if (player == 2) game->p2.score++;
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
    game->ball.subvx = -2 * BALL_SUBTICKS * (player == 1 ? 1 : -1);
    //game->ball.subvy = 1 * BALL_SUBTICKS; // Don't reset subvy, adds a little variety
    game->ball.x = game->ball.subx / BALL_SUBTICKS;
    game->ball.y = game->ball.suby / BALL_SUBTICKS;
    if ((game->p1.score >= WIN_SCORE) || (game->p2.score >= WIN_SCORE)) winGame(game, player);
    return;
}

void winGame(Game* game, int player) {
    game->state = GAME_STATE_GAMEOVER;
}

void doAi(Game* game) {
    int adjPadY = game->p2.pad.y + game->p2.pad.height / 2;

    int absDy = abs(adjPadY - game->ball.y);
    if (absDy < 1) game->p2.pad.vx -= 2;
    else {
        game->p2.pad.vy += (adjPadY - game->ball.y < 0 ? 2 : -2);
        if (absDy > 1) game->p2.pad.vx += 2;
    }
    return;
}

void gameTick(Game* game) {
    doAi(game);

    // Calculate new paddle positions, but don't apply them yet; will influence ball collision
    // Update paddle positions, ensuring they stay in bounds
    // Don't update velocity yet: will influence ball velocity if there's contact
    int pad1NewY = clamp(game->p1.pad.y + game->p1.pad.vy, BORDER_THICKNESS - 1, screenH - game->p1.pad.height);
    int pad2NewY = clamp(game->p2.pad.y + game->p2.pad.vy, BORDER_THICKNESS - 1, screenH - game->p2.pad.height);
    int pad1NewX = clamp(game->p1.pad.x + game->p1.pad.vx, BORDER_THICKNESS - 1, screenW / 3 - game->p1.pad.width);
    int pad2NewX = clamp(game->p2.pad.x + game->p2.pad.vx, 2 * screenW / 3, screenW - game->p2.pad.width);

    // Update ball position
    game->ball.subx += game->ball.subvx;
    game->ball.suby += game->ball.subvy;
    int newX = game->ball.subx / BALL_SUBTICKS;
    int newY = game->ball.suby / BALL_SUBTICKS;

    // Paddle collisions
    if ((game->ball.lastCol != 1)
        && (newX <= (pad1NewX + game->p1.pad.width))            // new x in/left-of pad 1's new right edge
        && (game->ball.x >= game->p1.pad.x)                     // old x in/right-of pad 1's old left edge
        && (newY >= pad1NewY)                                   // new Y in/below pad 1's new top edge
        && (game->ball.y <= (pad1NewY + game->p1.pad.height)))  // old Y in/above pad 1's new bottom edge
    {
        newX = (game->p1.pad.x + game->p1.pad.width);
        game->ball.subvx *= -1;
        game->ball.subvx = clamp(game->ball.subvx + (game->p1.pad.vx * BALL_SUBTICKS / 3), 10, 40);
        game->ball.subvy += game->p1.pad.vy * BALL_SUBTICKS / 3;
        game->ball.lastCol = 1;
    }
    else if ((game->ball.lastCol != 2)
        && (newX >= pad2NewX)                                       // new x in/right-of pad 2's new left edge
        && (game->ball.x <= (game->p2.pad.x + game->p2.pad.width))  // old x in/left-of pad 2's old right edge
        && (newY >= pad2NewY)                                       // new Y in/below pad 2's new top edge
        && (game->ball.y <= (pad2NewY + game->p2.pad.height)))      // old Y in/above pad 2's new bottom edge
    {
        newX = game->p2.pad.x;
        game->ball.subvx *= -1;
        game->ball.lastCol = 2;
    }

    // Apply new paddle values now
    game->p1.pad.vy = 0;
    game->p2.pad.vy = 0;
    game->p1.pad.vx = 0;
    game->p2.pad.vx = 0;
    game->p1.pad.y = pad1NewY;
    game->p2.pad.y = pad2NewY;
    game->p1.pad.x = pad1NewX;
    game->p2.pad.x = pad2NewX;

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
            case KEY_UP: game->p1.pad.vy -= 2; break;
            case KEY_DOWN: game->p1.pad.vy += 2; break;
            case KEY_RIGHT: game->p1.pad.vx += 3; break;
            case KEY_LEFT: game->p1.pad.vx -= 3; break;
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
    char buf[20];
    switch (game.state) {
        case GAME_STATE_READY: drawOverlay("READY"); break;
        case GAME_STATE_SCORED: sprintf(buf, "P%d SCORED", game.lastScore); buf[19] = '\x00'; drawOverlay(buf); break;
        case GAME_STATE_GAMEOVER: sprintf(buf, "P%d WIN", game.lastScore); buf[19] = '\x00'; drawOverlay(buf); break;
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
#ifdef INTERPOLATE
    int x = pad.x + (int)((float)pad.vx * frac);
    int y = pad.y + (int)((float)pad.vy * frac);
#else
    int x = pad.x;
    int y = pad.y;
#endif
    attron(COLOR_PAIR(pad.colorpair));
    for (int j = 0; j < pad.width; j++) {
        for (int i = 0; i < pad.height; i++) {
            mvaddch(i + y, j + x, '#');
        }
    }
    attroff(COLOR_PAIR(pad.colorpair));
}

void drawBall(Ball ball, float frac) {
#ifdef INTERPOLATE
    int x = ball.x + (int)((float)ball.subvx * frac / (float)BALL_SUBTICKS);
    int y = ball.y + (int)((float)ball.subvy * frac / (float)BALL_SUBTICKS);
#else
    int x = ball.x;
    int y = ball.y;
#endif
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
