#include <ncurses.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "keepaway.h"
#include "mainmenu.h"
#include "keepaway.h"

/* Helpers */
static inline int gridToIdx(int row, int col, int numCols) { return (col + (row * numCols)); }
static inline int idxToRow(int idx, int numCols) { return idx / numCols; }
static inline int idxToCol(int idx, int numCols) { return idx % numCols; }

void setCell(Game* game, int row, int col, int newType, int hp) {
    int myIdx = gridToIdx(row, col, game->numCols);
    game->lvl[row][col].type = newType;
    game->lvl[row][col].hp = hp;
    bool passable = ((newType == CELL_EMPTY) || (newType == CELL_START) || (newType == CELL_END));

    // update adj matrix: my own row & col, for entries corresponding to neighbors which are also passable
    for (int y = row - 1; y <= row + 1; y += 2) {
        if ((y < 0) || (y >= game->numRows)) continue; // oob check
        int nIdx = gridToIdx(y, col, game->numCols);
        bool npassable = (game->lvl[y][col].type == CELL_EMPTY) || (game->lvl[y][col].type == CELL_START) || (game->lvl[y][col].type == CELL_END); 
        game->adj[nIdx][myIdx] = passable && npassable;
        game->adj[myIdx][nIdx] = passable && npassable;
    }
    for (int x = col - 1; x <= col + 1; x+= 2) {
        if ((x < 0) || (x >= game->numCols)) continue; // oob check
        int nIdx = gridToIdx(row, x, game->numCols);
        bool npassable = (game->lvl[row][x].type == CELL_EMPTY) || (game->lvl[row][x].type == CELL_START) || (game->lvl[row][x].type == CELL_END); 
        game->adj[nIdx][myIdx] = passable && npassable;
        game->adj[myIdx][nIdx] = passable && npassable;
    }
}

/* Game logic */
void gamePlay() {
    struct timeval tv_now, tv_last;
    uint64_t dtUs;
    gettimeofday(&tv_last, NULL);
    Game game;
    gameInit(&game, 6, 6);
    bool exit = false;
    clear();
    refresh();

    nodelay(game.window, TRUE); // disable blocking on getch()

    while (!exit) {
        gettimeofday(&tv_now, NULL);
        dtUs = ((uint64_t)1000000) * ((uint64_t)tv_now.tv_sec - (uint64_t)tv_last.tv_sec) + ((uint64_t)tv_now.tv_usec - (uint64_t)tv_last.tv_usec);
        switch (game.state) {
            case GAME_STATE_ACTIVE: gameTick(&game, dtUs); break;
            case GAME_STATE_EXIT: exit = true; break;
            default: break;
        }
        gameDraw(&game);
    }

    printw("playgame");
    getch();
    nodelay(game.window, FALSE); // re-enable blocking on getch()
    return;
}

void gameInit(Game* game, int numRows, int numCols) {
    // initialize basic board properties
    game->numVerts = numRows * numCols;
    game->numRows = numRows;
    game->numCols = numCols;
    game->startPos = gridToIdx(numRows - 1, numCols / 2, numCols);
    game->endPos = gridToIdx(0, numCols / 2, numCols);

    // initialize adj matrix
    game->adj = (bool**)malloc(sizeof(bool*) * game->numVerts);
    for (int i = 0; i < game->numVerts; i++) {
        game->adj[i] = (bool*)malloc(sizeof(bool) * game->numVerts);
        memset(game->adj[i], (char)false, sizeof(bool) * game->numVerts);
    }

    // initialize cell data, then populate
    game->lvl = (Cell**)malloc(sizeof(Cell*) * numRows);
    for (int row = 0; row < numRows; row++) { game->lvl[row] = (Cell*)malloc(sizeof(Cell) * numCols); }
    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
            setCell(game, row, col, ((row*col == 0) || (row == numRows-1) || (col == numCols-1)) ? CELL_PERM_WALL : CELL_EMPTY, 0);
        }
    }

    setCell(game, idxToRow(game->startPos, numCols), idxToCol(game->startPos, numCols), CELL_START, 0);
    setCell(game, idxToRow(game->endPos, numCols), idxToCol(game->endPos, numCols), CELL_END, 0);

    // debug: dump adj to stderr
    fprintf(stderr, "   ");
    for (int vert = 0; vert < game->numVerts; vert++) fprintf(stderr, "%3d", vert);
    fprintf(stderr, "\n");
    for (int vert = 0; vert < game->numVerts; vert++) {
        fprintf(stderr, "%3d", vert);
        for (int conn = 0; conn < game->numVerts; conn++) {
            fprintf(stderr, game->adj[vert][conn] ? "  1" : "  0");
        }
        fprintf(stderr, "\n");
    }

    // initialize RedGuy
    game->rg.curPos = game->startPos;
    game->rg.path = (int*)malloc(sizeof(game->numVerts));
    pathfind(game->adj, game->startPos, game->endPos, game->rg.path, &(game->rg.pathLen));

    // create game window
    int h, w;
    getmaxyx(stdscr, h, w);
    game->window = newwin(numRows, numCols, (h - numRows) / 2, (w - numCols) / 2);
    clear();
    return;
}

void gameDestroy(Game** pgame) {
    Game* game = *pgame;
    for (int i = 0; i < game->numRows; i++) free(game->lvl[i]);
    free(game->lvl);
    for (int i = 0; i < game->numVerts; i++) free(game->adj[i]);
    free(game->adj);
    free(game->rg.path);
    *pgame = NULL;
    return;
}

void gameTick(Game* game, uint64_t dtUs) {
    return;
}

bool pathfind(bool** adj, int startPos, int endPos, int* path, int* pathLen) {
    // Path should have capacity == # of elements in adj
    *pathLen = 3;
    for (int i = 0; i < 3; i++) path[i] = i;
    return true;
}

/* Drawing */
void paintCh(char c, WINDOW* win, int row, int col, int colorpair) {
    wattron(win, COLOR_PAIR(colorpair));
    mvwaddch(win, row, col, c);
    wattroff(win, COLOR_PAIR(colorpair));
}

void gameDraw(Game* game) {
    werase(game->window);
    
    // first draw play field as normal
    for (int row = 0; row < game->numRows; row++) {
        for (int col = 0; col < game->numCols; col++) {
            switch (game->lvl[row][col].type) {
                case CELL_EMPTY: paintCh(CHAR_CELL_EMPTY, game->window, row, col, COLOR_CELL_EMPTY); break;
                case CELL_PERM_WALL: paintCh(CHAR_CELL_PERM_WALL, game->window, row, col, COLOR_CELL_PERM_WALL); break;
                case CELL_TEMP_WALL: paintCh(CHAR_CELL_TEMP_WALL, game->window, row, col, COLOR_CELL_TEMP_WALL); break;
                case CELL_START: paintCh(CHAR_CELL_START, game->window, row, col, COLOR_CELL_START); break;
                case CELL_END: paintCh(CHAR_CELL_END, game->window, row, col, COLOR_CELL_END); break;
            }
        }
    }

    // draw red guy path
    for (int i = 0; i < game->rg.pathLen; i++) {
        wattron(game->window, A_REVERSE);
        paintCh(CHAR_CELL_EMPTY, game->window, idxToRow(game->rg.path[i], game->numCols), idxToCol(game->rg.path[i], game->numCols), COLOR_CELL_EMPTY);
        wattroff(game->window, A_REVERSE);
    }

    // draw red guy

    // draw player/cursor
    wrefresh(game->window);
    return;
}
