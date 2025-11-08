#include <ncurses.h>
#include <sys/time.h>
#include <stdlib.h>
#include <inttypes.h>
#include "keepaway.h"
#include "mainmenu.h"
#include "keepaway.h"

/* Helpers */
inline int gridToIdx(int row, int col, int numCols) { return (col + (row * numCols)); }

void setCell(Game* game, int row, int col, int newType) {
    int myIdx = gridToIdx(row, col, game->numCols);
    game->lvl[row][col].type = newType;
    bool passable = (newType == CELL_EMPTY);

    for (int y = row - 1; y <= row + 1; y += 2) {
        if ((y < 0) || (y >= game->numRows)) continue; // oob check
        int nidx = gridToIdx(y, col, game->numCols);
        game->adj[nidx][myIdx] = passable;
    }
    for (int x = col - 1; x <= col + 1; x+= 2) {
        if ((x < 0) || (x >= game->numCols)) continue; // oob check
        int nidx = gridToIdx(row, x, game->numCols);
        game->adj[nidx][myIdx] = passable;
    }
}

/* Game logic */
void gamePlay() {
    struct timeval tv_now, tv_last;
    uint64_t dtUs;
    gettimeofday(&tv_last, NULL);
    Game game;
    gameInit(&game, 11, 11);
    bool exit = false;

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
    game->numVerts = numRows * numCols;
    game->numRows = numRows;
    game->numCols = numCols;

    game->lvl = (Cell**)malloc(sizeof(Cell*) * numRows);
    for (int i = 0; i < numRows; i++) {
        game->lvl[i] = (Cell*)malloc(sizeof(Cell) * numCols);
        for (int j = 0; j < numCols; j++) {
            game->lvl[i][j].type = ((i*j == 0) || (i == numRows-1) || (j == numCols-1)) ? CELL_PERM_WALL : CELL_EMPTY;
            game->lvl[i][j].hp = 0;
        }
    }

    game->adj = (int**)malloc(sizeof(int*) * game->numVerts);
    for (int i = 0; i < game->numVerts; i++) {
        game->adj[i] = (int*)malloc(sizeof(int) * game->numVerts);
    }

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
    *pgame = NULL;
    return;
}

void gameTick(Game* game, uint64_t dtUs) {
    return;
}

/* Drawing */
void paintCh(char c, WINDOW* win, int row, int col, int colorpair) {
    attron(COLOR_PAIR(colorpair));
    mvwaddch(win, col, row, c);
    attroff(COLOR_PAIR(colorpair));
}

void gameDraw(Game* game) {
    werase(game->window);
    for (int row = 0; row < game->numRows; row++) {
        for (int col = 0; col < game->numCols; col++) {
            switch (game->lvl[row][col].type) {
                case CELL_EMPTY: paintCh(CHAR_CELL_EMPTY, game->window, row, col, COLOR_CELL_EMPTY); break;
                case CELL_PERM_WALL: paintCh(CHAR_CELL_PERM_WALL, game->window, row, col, COLOR_CELL_PERM_WALL); break;
                case CELL_TEMP_WALL: paintCh(CHAR_CELL_TEMP_WALL, game->window, row, col, COLOR_CELL_TEMP_WALL); break;
            }
        }
    }
    wrefresh(game->window);
    return;
}
