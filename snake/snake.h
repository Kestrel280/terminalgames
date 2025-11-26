#ifndef SNAKE_H
#define SNAKE_H

#include <ncurses.h>
#include "bitpack.h"

#define SNAKE_COOLDOWN 125000l

typedef int GameState;
typedef int Direction;
typedef int Collision;
typedef int ColorGroup;
typedef struct _snake Snake;
typedef struct _game Game;

// GameState
enum {
    GAME_STATE_ACTIVE,
    GAME_STATE_DYING,
    GAME_STATE_FINISHED,
    GAME_STATE_META_NUM_STATES
};

// Direction
enum {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

// Collision
enum {
    COLLISION_NONE,
    COLLISION_WALL,
    COLLISION_SELF,
    COLLISION_PICKUP
};

// Color Groups
enum {
    CG_SPACE,
    CG_WALL,
    CG_SNAKE,
    CG_SNAKEDEAD,
    CG_SNAKEHEAD,
    CG_PICKUP,
    CG_META_NUM_CGS
};

struct _snake {
    int* data;
    int size, capacity;
    int head, tail;
    BitPack bp;
};

// Game
struct _game {
    GameState state;
    Direction curDir;
    int height, width;
    int64_t timeToNextTickUs;
    int64_t baseTimeToDecayUs, timeToNextDecayUs;
    Snake snake;
    WINDOW* window;
    int pickupPos;
    bool needsDraw;
};

int gamePlay();
void gameTick(Game* game, uint64_t dtUs);
void gameHandleInput(Game* game);
void gameTrySetDir(Game* game, Direction newDir);
Collision gameTryCollision(Game* game, int pos);
void gameDraw(Game* game);
void gameCreatePickup(Game* game);
void gameOver(Game* game, uint64_t dtUs);

#endif
