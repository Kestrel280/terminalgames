#define PADDLE_CH '#'
#define BALL_CH 'O'

#define COLOR_PLAYER_PADDLE_BG COLOR_WHITE
#define COLOR_PLAYER_PADDLE_FG COLOR_PLAYER_PADDLE_BG
#define COLOR_OPPONENT_PADDLE_BG COLOR_RED
#define COLOR_OPPONENT_PADDLE_FG COLOR_OPPONENT_PADDLE_BG

typedef struct _paddle Paddle;
typedef struct _ball Ball;
typedef struct _game Game;
typedef struct _player Player;

const int screenW = 50;
const int screenH = 25;

enum {
    COLOR_UNUSED,
    COLOR_PLAYER_PADDLE,
    COLOR_OPPONENT_PADDLE
};

enum {
    GAME_STATE_READY,
    GAME_STATE_PLAYING,
    GAME_STATE_SCORED
};

struct _paddle {
    int width;
    int height;
    int y;
    int x;
    int vy;
    int vx;
    int colorpair;
};

struct _ball {
    int y;
    int x;
    int vy;
    int vx;
};

struct _player {
    int score;
    Paddle pad;
};

struct _game {
    bool finished;
    int state;
    Player p1;
    Player p2;
    Ball ball;
    int pauseCounter;
};

/* Setup */
void initColors();
void initGame(Game* game);
void initPlayer(Player* player, int color, int x);

/* Game logic */
void playGame();
void gameTick(Game* game);
void gameHandleInput(Game* game);

/* Drawing */
void drawGame(Game game);
void drawOverlay(const char* msg);
void drawBorder();
void drawPaddle(Paddle pad);
void drawBall(Ball ball);
