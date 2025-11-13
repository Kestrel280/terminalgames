#ifndef KEEPAWAY_H
#define KEEPAWAY_H

#define PATHFIND pathfind_bfs

#define CHAR_CELL_EMPTY ' '
#define COLOR_CELL_EMPTY_BG COLOR_BLACK
#define COLOR_CELL_EMPTY_FG COLOR_WHITE
#define CHAR_CELL_PERM_WALL 'X'
#define COLOR_CELL_PERM_WALL_BG COLOR_WHITE
#define COLOR_CELL_PERM_WALL_FG COLOR_WHITE
#define CHAR_CELL_TEMP_WALL ' '
#define COLOR_CELL_TEMP_WALL_BG COLOR_CYAN
#define COLOR_CELL_TEMP_WALL_FG COLOR_BLACK
#define CHAR_CELL_START 'S'
#define COLOR_CELL_START_FG COLOR_BLUE
#define COLOR_CELL_START_BG COLOR_CELL_START_FG
#define CHAR_CELL_END 'E'
#define COLOR_CELL_END_FG COLOR_YELLOW
#define COLOR_CELL_END_BG COLOR_CELL_END_FG
#define CHAR_REDGUY '#'
#define CHAR_REDGUY_PATH '.'
#define COLOR_REDGUY_FG COLOR_RED
#define COLOR_REDGUY_BG COLOR_BLACK
#define CHAR_PLAYER 'P'
#define COLOR_PLAYER_FG COLOR_CYAN
#define COLOR_PLAYER_BG COLOR_BLACK

typedef struct _cell Cell;
typedef struct _game Game;
typedef struct _redguy RedGuy;
typedef struct _player Player;

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
    COLOR_CELL_END,
    COLOR_REDGUY,
    COLOR_PLAYER
};

struct _cell {
    int type;
    int64_t hp;
};

struct _redguy {
    int curPos;
    int* path;
    int pathProgress;
    int pathLen;
    int64_t basePatience;
    int64_t patience;
};

struct _player {
    int curPos;
    int numBarricades;
    int numRemainingBarricades;
    int* barricades;
    int bStart;
    int bEnd;
};

struct _game {
    int state; //GAME_STATE_... enum
    int score;
    int numRows, numCols, numVerts; // lvl metadata
    int startPos, endPos; // lvl start/end locations
    Cell** lvl; // lvl data
    bool** adj; // lvl adjacency matrix
    bool needsDraw; // whether game needs to be painted
    RedGuy rg; // red guy
    Player player; // player
    WINDOW* window; // ncurses window
    WINDOW* statswindow;
};

void setCell(Game* game, int row, int col, int newType, int64_t hp);
void gamePlay();
void gameInit(Game* game, int numRows, int numCols);
void gameDestroy(Game* game);
void gameTick(Game* game, uint64_t dtUs);
void gameDraw(Game* game);
void gameOver(Game* game);
void gameHandleInput(Game* game);
bool gameTryPlaceBarricade(Game* game);
void drawOverlay(const char* msg);

int* pathfind_bfs(bool** adj, int startPos, int endPos, int numVerts, int* pathLen);

#endif
