#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include "snake.h"
#include "leaderboard.h"

char gfxChars[CG_META_NUM_CGS];

static inline int gridToIdx(int row, int col, int numCols) { return (row * numCols + col); }
static inline int idxToRow(int idx, int numCols) { return idx / numCols; }
static inline int idxToCol(int idx, int numCols) { return idx % numCols; }

static inline int snakePush(Snake* q, int item) {
    if (q->size + 1 > q->capacity) return -1;
    q->size++;
    q->head = (q->head + 1) % q->capacity;
    q->data[q->head] = item;
    bitPackSet(q->bp, item, true);
    return 1;
}
static inline int snakePop(Snake* q) {
    if (q->size == 0) return -1;
    q->size--;
    int item = q->data[q->tail];
    q->tail = (q->tail + 1) % q->capacity;
    bitPackSet(q->bp, item, false);
    return item;
}

int gamePlay() {
    clear();
    refresh();
    Game game;

    game.width = GAME_WIDTH;
    game.height = GAME_HEIGHT;
    
    int screenMaxWidth, screenMaxHeight;
    getmaxyx(stdscr, screenMaxHeight, screenMaxWidth);
    game.window = newwin(game.height, game.width, (screenMaxHeight - game.height) / 2, (screenMaxWidth - game.width) / 2);
    game.statsWindow = newwin(2, game.width, (screenMaxHeight - game.height) / 2 + game.height, (screenMaxWidth - game.width) / 2);
    keypad(game.window, TRUE);
    nodelay(game.window, TRUE); // disable blocking on getch()
    game.needsDraw = true;

    game.state = GAME_STATE_ACTIVE;
    game.timeToNextTickUs = SNAKE_COOLDOWN;
    game.curDir = SOUTH;
    game.score = 0;
    game.snake.data = (int*)malloc(sizeof(int) * game.width * game.height);
    game.snake.size = 5;
    game.snake.capacity = game.width * game.height;
    game.snake.head = game.snake.size - 1;
    game.snake.tail = 0;
    game.snake.data[game.snake.head] = gridToIdx(game.height / 2, game.width / 2, game.width);
    for (int i = game.snake.head - 1; i >= 0; i--) game.snake.data[i] = game.snake.data[i + 1] - game.width;
    game.snake.bp.bitsCapacity = game.snake.capacity; // initialize snake pitback
    game.snake.bp._numEls = (game.snake.capacity + (sizeof(bitpack_el) - 1)) / sizeof(bitpack_el); // # of game cells divided by sizeof(bitpack_el), rounded up
    game.snake.bp._data = (bitpack_el*)malloc(sizeof(bitpack_el) * game.snake.bp._numEls); // allocate bitpack data
    for (int i = 0; i < game.snake.bp._numEls; i++) game.snake.bp._data[i] = (bitpack_el)0; // initialize all bits to 0 (false)
    for (int i = game.snake.tail; i <= game.snake.head; i++) bitPackSet(game.snake.bp, game.snake.data[i], true); // set all bits of snake

    game.baseTimeToDecayUs = (int64_t)400000;
    game.timeToNextDecayUs = (int64_t)0;

    struct timeval tv_prev, tv_now;
    gettimeofday(&tv_prev, NULL);
    srand(tv_prev.tv_sec * tv_prev.tv_usec);
    gameCreatePickup(&game);
    while (game.state != GAME_STATE_FINISHED) {
        gettimeofday(&tv_now, NULL);
        uint64_t dtUs = 1000000ul * ((uint64_t)tv_now.tv_sec - (uint64_t)tv_prev.tv_sec) + ((uint64_t)tv_now.tv_usec - (uint64_t)tv_prev.tv_usec);
        tv_prev = tv_now;

        gameHandleInput(&game);

        switch (game.state) {
            case GAME_STATE_ACTIVE: gameTick(&game, dtUs); break;
            case GAME_STATE_DYING: gameOver(&game, dtUs); break;
        }
        if (game.needsDraw) gameDraw(&game);
    }

    nodelay(game.window, FALSE); // disable blocking on getch()
    leaderboardSubmitScore(gameName, game.score);

    free(game.snake.data);
    free(game.snake.bp._data);
    return 0;
}

void gameTick(Game* game, uint64_t dtUs) {
    game->timeToNextTickUs -= (int64_t)dtUs;
    if (game->timeToNextTickUs >= 0l) return;
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
        case COLLISION_WALL: // fallthrough
        case COLLISION_SELF: game->state = GAME_STATE_DYING; game->needsDraw = true; return;
        case COLLISION_PICKUP: { // manually duplicate tail
            game->snake.size++;
            int origTail = game->snake.tail;
            game->snake.tail = origTail - 1;
            if (game->snake.tail < 0) game->snake.tail = game->width * game->height - 1;
            game->snake.data[game->snake.tail] = game->snake.data[origTail];
            gameCreatePickup(game);
            break;
        }
    }

    snakePush(&game->snake, snakeHeadNextPos);
    snakePop(&game->snake);

    game->score += (game->snake.size * game->snake.size);

    game->needsDraw = true;

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

// do not allow snake to double back on itself. it can turn into an older segment of itself or a wall,
//  but it cannot turn 180 degrees
void gameTrySetDir(Game* game, Direction newDir) {
    if (game->snake.size == 1) { game->curDir = newDir; return; }
    int snakePenultimateHead = game->snake.data[(game->snake.head - 1 + game->snake.capacity) % game->snake.capacity];
    int attemptedNextPos;
    switch (newDir) { // calculating attemptedNextPos this way LOOKS like it could "wrap" the side of the board inappropriately; but, because snake is inner-bounded by the walls, this can't happen
                      // it also doesn't actually MATTER, because this value is ONLY used for checking equality with snakePenultimateHead; we're not actually going to try and move to this position
        case NORTH: attemptedNextPos = game->snake.data[game->snake.head] - game->width; break;
        case SOUTH: attemptedNextPos = game->snake.data[game->snake.head] + game->width; break;
        case EAST: attemptedNextPos = game->snake.data[game->snake.head] + 1; break;
        case WEST: attemptedNextPos = game->snake.data[game->snake.head] - 1; break;
    }
    if (attemptedNextPos == snakePenultimateHead) return;
    else game->curDir = newDir;
    return;
}

Collision gameTryCollision(Game* game, int pos) {
    if (pos / game->width == 0) return COLLISION_WALL; // top edge
    if (pos / game->width == game->height - 1) return COLLISION_WALL; // bottom edge
    if (pos % game->width == 0) return COLLISION_WALL; // left edge
    if ((pos + 1) % game->width == 0) return COLLISION_WALL; // right edge

    if (bitPackGet(game->snake.bp, pos)) return COLLISION_SELF;

    if (pos == game->pickupPos) return COLLISION_PICKUP;

    return COLLISION_NONE;
}

static inline void paintCh(WINDOW* window, char c, int cg, int row, int col) {
    if (window == NULL) return;
    wattron(window, COLOR_PAIR(cg));
    mvwaddch(window, row, col, c);
    wattroff(window, COLOR_PAIR(cg));
}

void gameDraw(Game* game) {
    game->needsDraw = false;
    werase(game->window);
    werase(game->statsWindow);

    // draw walls
    for (int i = 0; i <= game->width; i++) paintCh(game->window, gfxChars[CG_WALL], CG_WALL, 0, i);
    for (int i = 0; i <= game->width; i++) paintCh(game->window, gfxChars[CG_WALL], CG_WALL, game->height - 1, i);
    for (int i = 0; i <= game->height; i++) paintCh(game->window, gfxChars[CG_WALL], CG_WALL, i, 0);
    for (int i = 0; i <= game->height; i++) paintCh(game->window, gfxChars[CG_WALL], CG_WALL, i, game->width - 1);

    // draw snake
    for (int i = game->snake.tail; i != game->snake.head; i = (i + 1) % game->snake.capacity) {
        paintCh(game->window, gfxChars[CG_SNAKE], game->state == GAME_STATE_DYING ? CG_SNAKEDEAD : CG_SNAKE, idxToRow(game->snake.data[i], game->width), idxToCol(game->snake.data[i], game->width));
    }
    paintCh(game->window, gfxChars[CG_SNAKEHEAD], CG_SNAKEHEAD, idxToRow(game->snake.data[game->snake.head], game->width), idxToCol(game->snake.data[game->snake.head], game->width));

    // draw pickup
    paintCh(game->window, gfxChars[CG_PICKUP], CG_PICKUP, idxToRow(game->pickupPos, game->width), idxToCol(game->pickupPos, game->width));

    char buf[128];
    sprintf(buf, "%llu", game->score);
    mvwprintw(game->statsWindow, 0, (game->width - strlen("-- SCORE --")) / 2, "-- SCORE --");
    mvwprintw(game->statsWindow, 1, (game->width - strlen(buf)) / 2, buf);

    wrefresh(game->window);
    wrefresh(game->statsWindow);
    return;
}

void gameCreatePickup(Game* game) {
    int idx;
    while (1) {
        idx = rand() % (game->height * game->width);
        if ((idx / game->width == 0) || (idx % game->width == 0) || (idx / game->width == game->height - 1) || ((idx + 1) % game->width == 0)) continue; // don't spawn in wall
        if (bitPackGet(game->snake.bp, idx)) continue; // don't spawn inside snake
        break;
    }
    game->pickupPos = idx;
    return;
}

void gameOver(Game* game, uint64_t dtUs) {
    game->timeToNextDecayUs -= (int64_t)dtUs;
    if (game->timeToNextDecayUs < 0) {
        if (game->snake.size <= 1) { game->state = GAME_STATE_FINISHED; return; }
        game->timeToNextDecayUs += game->baseTimeToDecayUs;
        game->baseTimeToDecayUs = game->baseTimeToDecayUs - (game->baseTimeToDecayUs / 10);
        game->needsDraw = true;
    }
}
