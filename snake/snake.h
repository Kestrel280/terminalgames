#ifndef SNAKE_H
#define SNAKE_H

typedef int GameState;
typedef int Direction;
typedef struct _game Game;

// GameState
enum {
    GAME_STATE_ACTIVE,
    GAME_STATE_STOPPED,
    GAME_STATE_META_NUM_STATES
};

// Direction
enum {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

// Game
struct _game {
    GameState state;
    Direction curDir;
    int height, width;
};

int gamePlay();
void gameTick(Game* game, uint64_t dtUs);
void gameHandleInput(Game* game);
void gameTrySetDir(Game* game, Direction newDir);

#endif
