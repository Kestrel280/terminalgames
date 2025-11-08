#ifndef KEEPAWAY_H
#define KEEPAWAY_H

#define CHAR_CELL_EMPTY '.'
#define COLOR_CELL_EMPTY_BG COLOR_BLACK
#define COLOR_CELL_EMPTY_FG COLOR_WHITE
#define CHAR_CELL_PERM_WALL 'X'
#define COLOR_CELL_PERM_WALL_BG COLOR_WHITE
#define COLOR_CELL_PERM_WALL_FG COLOR_WHITE
#define CHAR_CELL_TEMP_WALL 'O'
#define COLOR_CELL_TEMP_WALL_BG COLOR_WHITE
#define COLOR_CELL_TEMP_WALL_FG COLOR_WHITE

typedef struct _cell Cell;
typedef struct _game Game;

enum {
    GAME_STATE_UNUSED,
    GAME_STATE_ACTIVE,
    GAME_STATE_EXIT
};

enum {
    CELL_EMPTY,
    CELL_PERM_WALL,
    CELL_TEMP_WALL
};

enum {
    COLOR_UNUSED,
    COLOR_CELL_EMPTY,
    COLOR_CELL_PERM_WALL,
    COLOR_CELL_TEMP_WALL
};

struct _cell {
    int type;
    int hp;
};

struct _game {
    int numRows, numCols, numVerts;
    Cell** lvl;
    int** adj;
    int state;
    WINDOW* window;
};

int gridToIdx(int row, int col, int numCols);
void setCell(Game* game, int row, int col, int newType);
void gamePlay();
void gameInit(Game* game, int numRows, int numCols);
void gameDestroy(Game** pgame);
void gameTick(Game* game, uint64_t dtUs);
void gameDraw(Game* game);

#endif
