#define PADDLE_CH '#'
#define BALL_CH 'O'

#define COLOR_BACKGROUND_BG COLOR_CYAN
#define COLOR_BACKGROUND_FG COLOR_BACKGROUND_BG
#define COLOR_BALL_BG COLOR_YELLOW
#define COLOR_BALL_FG COLOR_BALL_BG
#define COLOR_PLAYER_PADDLE_BG COLOR_GREEN
#define COLOR_PLAYER_PADDLE_FG COLOR_PLAYER_PADDLE_BG
#define COLOR_OPPONENT_PADDLE_BG COLOR_RED
#define COLOR_OPPONENT_PADDLE_FG COLOR_OPPONENT_PADDLE_BG
#define COLOR_WALL_BG COLOR_CYAN
#define COLOR_WALL_FG COLOR_WALL_BG
#define COLOR_TRIM_BG COLOR_WHITE
#define COLOR_TRIM_FG COLOR_TRIM_BG

typedef struct _paddle Paddle;
typedef struct _ball Ball;
typedef struct _game Game;
typedef struct _player Player;

const int screenW = 160;
const int screenH = 35;
const uint64_t TICK_TIME_US = 100000;
const int BORDER_THICKNESS = 2;
const int READY_TICKS = 10;
const int SCORED_TICKS = 20;
const int BALL_SUBTICKS = 10;
const int WIN_SCORE = 5;

enum {
    COLOR_UNUSED,
    COLOR_BG,
    COLOR_PLAYER_PADDLE,
    COLOR_OPPONENT_PADDLE,
    COLOR_BALL,
    COLOR_WALL,
    COLOR_TRIM
};

enum {
    GAME_STATE_READY,
    GAME_STATE_PLAYING,
    GAME_STATE_SCORED,
    GAME_STATE_GAMEOVER
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
    int subx;
    int suby;
    int y;
    int x;
    int subvy;
    int subvx;
    int colorpair;
    int lastCol;
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
    int lastScore;
    float frac; // percentage to next tick, 0-1
};

/* Setup */
void initColors();
void initGame(Game* game);
void initPlayer(Player* player, int color, int x);

/* Game logic */
void playGame();
void score(Game* game, int player);
void winGame(Game* game, int player);
void doAi(Game* game);
void gameTick(Game* game);
void gameHandleInput(Game* game);

/* Drawing */
void drawGame(Game game);
void drawOverlay(const char* msg);
void drawTrim();
void drawBorder();
void drawPaddle(Paddle pad, float frac);
void drawBall(Ball ball, float frac);
void drawScore(Game game);
