#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>
#include "snake.h"

char gfxChars[CG_META_NUM_CGS];

static inline int gridToIdx(int row, int col, int numCols) { return (row * numCols + col); }
static inline int idxToRow(int idx, int numCols) { return idx / numCols; }
static inline int idxToCol(int idx, int numCols) { return idx % numCols; }

static inline int snakePush(Snake* q, int item) {
    if (q->size + 1 > q->capacity) return -1;
    q->size++;
    q->head = (q->head + 1) % q->size;
    q->data[q->head] = item;
    return 1;
}
static inline int snakePop(Snake* q) {
    if (q->size == 0) return -1;
    q->size--;
    return q->data[q->tail++];
}

int gamePlay() {
    clear();
    refresh();
    Game game;

    game.width = 15;
    game.height = 15;
    
    int screenMaxWidth, screenMaxHeight;
    getmaxyx(stdscr, screenMaxHeight, screenMaxWidth);
    game.window = newwin(game.height, game.width, (screenMaxHeight - game.height) / 2, (screenMaxWidth - game.width) / 2);
    keypad(game.window, TRUE);
    nodelay(game.window, TRUE); // disable blocking on getch()
    game.needsDraw = true;

    game.state = GAME_STATE_ACTIVE;
    game.timeToNextTickUs = SNAKE_COOLDOWN;
    game.curDir = SOUTH;
    game.snake.data = (int*)malloc(sizeof(int) * game.width * game.height);
    game.snake.size = 1;
    game.snake.capacity = game.width * game.height;
    game.snake.head = 0;
    game.snake.tail = 0;
    game.snake.data[0] = gridToIdx(game.height / 2, game.width / 2, game.width);

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

        if (game.needsDraw) gameDraw(&game);
    }

    free(game.snake.data);
    return 0;
}

void gameTick(Game* game, uint64_t dtUs) {
    game->timeToNextTickUs -= dtUs;
    if (game->timeToNextTickUs >= 0) return;
    
    game->timeToNextTickUs += SNAKE_COOLDOWN;
    int snakeHeadCurRow = idxToRow(game->snake.data[game->snake.head], game->width);
    int snakeHeadCurCol = idxToCol(game->snake.data[game->snake.head], game->width);
    int snakeHeadNextPos;
    switch (game->curDir) {
        case NORTH: snakeHeadNextPos = gridToIdx(snakeHeadCurRow - 1, snakeHeadCurCol, game->width); break;
        case SOUTH: snakeHeadNextPos = gridToIdx(snakeHeadCurRow + 1, snakeHeadCurCol, game->width); break;
        case EAST: snakeHeadNextPos = gridToIdx(snakeHeadCurRow, snakeHeadCurCol + 1, game->width); break;
        case WEST: snakeHeadNextPos = gridToIdx(snakeHeadCurRow, snakeHeadCurCol - 1, game->width); break;
    }

    Collision col = gameTryCollision(game, snakeHeadNextPos);
    switch (col) {
        case COLLISION_NONE: break;
        case COLLISION_WALL: break;
        case COLLISION_SELF: break;
        case COLLISION_PICKUP: break;
    }

    snakePush(&game->snake, snakeHeadNextPos);
    snakePop(&game->snake);

    return;
}

void gameHandleInput(Game* game) {
    int key = wgetch(game->window);
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

Collision gameTryCollision(Game* game, int pos) {
    return COLLISION_NONE;
}

static inline void paintCh(WINDOW* window, char c, int cg, int row, int col) {
    wattron(window, COLOR_PAIR(cg));
    mvwaddch(window, row, col, c);
    wattroff(window, COLOR_PAIR(cg));
}

void gameDraw(Game* game) {
    game->needsDraw = false;
    werase(game->window);

    // draw walls
    for (int i = 0; i <= game->width; i++) paintCh(game->window, gfxChars[CG_WALL], CG_WALL, 0, i);
    for (int i = 0; i <= game->width; i++) paintCh(game->window, gfxChars[CG_WALL], CG_WALL, game->height - 1, i);
    for (int i = 0; i <= game->height; i++) paintCh(game->window, gfxChars[CG_WALL], CG_WALL, i, 0);
    for (int i = 0; i <= game->height; i++) paintCh(game->window, gfxChars[CG_WALL], CG_WALL, i, game->width - 1);

    wrefresh(game->window);
    return;
}
