#ifndef KEEPAWAY_H
#define KEEPAWAY_H

#define PATHFIND pathfind_bfs

#define CHAR_CELL_EMPTY '.'
#define COLOR_CELL_EMPTY_BG COLOR_BLACK
#define COLOR_CELL_EMPTY_FG COLOR_WHITE
#define CHAR_CELL_PERM_WALL 'X'
#define COLOR_CELL_PERM_WALL_BG COLOR_WHITE
#define COLOR_CELL_PERM_WALL_FG COLOR_WHITE
#define CHAR_CELL_TEMP_WALL 'O'
#define COLOR_CELL_TEMP_WALL_BG COLOR_WHITE
#define COLOR_CELL_TEMP_WALL_FG COLOR_WHITE
#define CHAR_CELL_START 'S'
#define COLOR_CELL_START_FG COLOR_BLUE
#define COLOR_CELL_START_BG COLOR_CELL_START_FG
#define CHAR_CELL_END 'E'
#define COLOR_CELL_END_FG COLOR_YELLOW
#define COLOR_CELL_END_BG COLOR_CELL_END_FG

typedef struct _cell Cell;
typedef struct _game Game;
typedef struct _redguy RedGuy;

enum {
    GAME_STATE_UNUSED,
    GAME_STATE_ACTIVE,
    GAME_STATE_EXIT
};

enum {
    CELL_EMPTY,
    CELL_PERM_WALL,
    CELL_TEMP_WALL,
    CELL_START,
    CELL_END
};

enum {
    COLOR_UNUSED,
    COLOR_CELL_EMPTY,
    COLOR_CELL_PERM_WALL,
    COLOR_CELL_TEMP_WALL,
    COLOR_CELL_START,
    COLOR_CELL_END
};

struct _cell {
    int type;
    int hp;
};

struct _redguy {
    int curPos;
    int* path;
    int pathLen;
};

struct _game {
    int numRows, numCols, numVerts;
    Cell** lvl;
    int startPos, endPos;
    RedGuy rg;
    bool** adj;
    int state;
    WINDOW* window;
};

void setCell(Game* game, int row, int col, int newType, int hp);
void gamePlay();
void gameInit(Game* game, int numRows, int numCols);
void gameDestroy(Game** pgame);
void gameTick(Game* game, uint64_t dtUs);
void gameDraw(Game* game);

int* pathfind_bfs(bool** adj, int startPos, int endPos, int numVerts, int* pathLen);

#endif
