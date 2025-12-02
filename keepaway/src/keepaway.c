#include <ncurses.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "keepaway.h"
#include "mainmenu.h"
#include "keepaway.h"
#include "leaderboard.h"

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

const char* gameName = "keepaway";

/* Helpers */
static inline int gridToIdx(int row, int col, int numCols) { return (col + (row * numCols)); }
static inline int idxToRow(int idx, int numCols) { return idx / numCols; }
static inline int idxToCol(int idx, int numCols) { return idx % numCols; }

void setCell(Game* game, int row, int col, int newType, int64_t hp) {
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
void gamePlay(int nrows, int ncols, int difficulty) {
    struct timeval tv_now, tv_last;
    uint64_t dtUs;
    gettimeofday(&tv_last, NULL);
    Game game;
    gameInit(&game, nrows, ncols, difficulty);
    bool exit = false;
    clear();
    refresh();

    nodelay(game.window, TRUE); // disable blocking on getch()

    while (!exit) {
        gameHandleInput(&game);
        gettimeofday(&tv_now, NULL);
        dtUs = ((uint64_t)1000000) * ((uint64_t)tv_now.tv_sec - (uint64_t)tv_last.tv_sec) + ((uint64_t)tv_now.tv_usec - (uint64_t)tv_last.tv_usec);
        tv_last = tv_now;
        switch (game.state) {
            case GAME_STATE_ACTIVE: gameTick(&game, dtUs); break;
            case GAME_STATE_EXIT: exit = true; break;
            default: break;
        }
        if (game.needsDraw) gameDraw(&game);
    }

    // clean up
    nodelay(game.window, FALSE); // re-enable blocking on getch()
    char buf[128];
    sprintf(buf, "GAME OVER | SCORE = %d | PRESS ANY KEY", game.score);
    drawOverlay(buf);
    getch();
    leaderboardSubmitScore(gameName, game.score);
    gameDestroy(&game);
    return;
}

void gameHandleInput(Game* game) {
    int key = wgetch(game->window);
    switch (key) {
        case ERR: return;
        case KEY_UP: game->player.curPos -= game->player.curPos < game->numCols ? 0 : game->numCols; break;
        case KEY_DOWN: game->player.curPos += game->player.curPos >= (game->numCols * (game->numRows - 1)) ? 0 : game->numCols; break;
        case KEY_RIGHT: game->player.curPos += ((game->player.curPos + 1) % game->numCols) == 0 ? 0 : 1; break;
        case KEY_LEFT: game->player.curPos -= ((game->player.curPos) % game->numCols) == 0 ? 0 : 1; break;
        case (int)' ': gameTryPlaceBarricade(game);
    }
    game->needsDraw = true;
}

bool gameTryPlaceBarricade(Game* game) {
    int pos = game->player.curPos;
    int row = idxToRow(pos, game->numCols);
    int col = idxToCol(pos, game->numCols);

    if (game->player.numRemainingBarricades <= 0) return false; // no barricades to place
    if (pos == game->rg.curPos) return false; // can't place on top of red guy
    if (game->lvl[row][col].type != CELL_EMPTY) return false; // can only place on empty spaces
    
    setCell(game, row, col, CELL_TEMP_WALL, 12500000l - (game->difficulty * 2500000l));
    int pl;
    int* newPath = PATHFIND(game->adj, game->rg.curPos, game->endPos, game->numVerts, &pl);
    if (newPath == NULL) { setCell(game, row, col, CELL_EMPTY, 0l); return false; } 
    else { // can place a barricade: red guy still has path to exit. swap out his path for the new one
        free(game->rg.path);
        game->rg.path = newPath;
        game->rg.pathProgress = 0;
        game->rg.pathLen = pl;
        game->player.numRemainingBarricades--;
        game->player.barricades[game->player.bEnd] = pos;
        game->player.bEnd = (game->player.bEnd + 1) % game->player.numBarricades;
    }
    game->needsDraw = true;
    return true;
}


void gameInit(Game* game, int numRows, int numCols, int difficulty) {
    // initialize basic board properties
    game->state = GAME_STATE_ACTIVE;
    game->score = 0;
    game->numVerts = numRows * numCols;
    game->numRows = numRows;
    game->numCols = numCols;
    game->startPos = gridToIdx(numRows - 1, numCols / 2, numCols);
    game->endPos = gridToIdx(0, numCols / 2, numCols);
    game->needsDraw = true;
    game->difficulty = difficulty;

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
    /*
    DEBUG_PRINT("   ");
    for (int vert = 0; vert < game->numVerts; vert++) DEBUG_PRINT("%3d", vert);
    DEBUG_PRINT("\n");
    for (int vert = 0; vert < game->numVerts; vert++) {
        DEBUG_PRINT("%3d", vert);
        for (int conn = 0; conn < game->numVerts; conn++) {
            DEBUG_PRINT(game->adj[vert][conn] ? "  1" : "  0");
        }
        DEBUG_PRINT("\n");
    }
    */

    // initialize RedGuy
    game->rg.basePatience = 1500000l - (difficulty * 250000l);
    game->rg.patience = game->rg.basePatience;
    game->rg.curPos = game->startPos;
    game->rg.path = PATHFIND(game->adj, game->startPos, game->endPos, game->numVerts, &(game->rg.pathLen));
    game->rg.pathProgress = 0;
    DEBUG_PRINT("redguy initial path has length %d\n", game->rg.pathLen);

    // initialize Player
    game->player.curPos = gridToIdx(game->numRows / 2, game->numCols / 2, game->numCols);
    game->player.numBarricades = (4 - difficulty) * (game->numVerts / 200);
    game->player.numRemainingBarricades = game->player.numBarricades;
    game->player.barricades = (int*)malloc(sizeof(int) * game->player.numBarricades);
    game->player.bStart = 0;
    game->player.bEnd = 0;

    // create game window
    int h, w;
    getmaxyx(stdscr, h, w);
    game->window = newwin(numRows + 4, numCols, (h - numRows) / 2, (w - numCols) / 2);
    keypad(game->window, TRUE);

    // create stats window
    game->statswindow = newwin(4, 50, (h + numRows) / 2, (w - numCols) / 2);
    clear();
    return;
}

void gameDestroy(Game* game) {
    for (int i = 0; i < game->numRows; i++) free(game->lvl[i]);
    free(game->lvl);
    for (int i = 0; i < game->numVerts; i++) free(game->adj[i]);
    free(game->adj);
    free(game->rg.path);
    free(game->player.barricades);
    return;
}

void gameTick(Game* game, uint64_t dtUs) {
    if ((game->rg.patience -= (int64_t)dtUs) < 0) {
        game->rg.curPos = game->rg.path[game->rg.pathProgress++];
        if (game->rg.curPos == game->endPos) gameOver(game);
        game->rg.basePatience -= game->rg.basePatience / (70 - (game->difficulty * 20));
        game->rg.basePatience = game->rg.basePatience < 125000 ? 125000 : game->rg.basePatience; // cap rg speed at 5 moves per second
        game->rg.patience += game->rg.basePatience;
        game->score += 10000000l / game->rg.basePatience * (game->difficulty + 1);
        game->needsDraw = true;
    }
    
    for (int bufIdx = game->player.bStart, dum = game->player.numBarricades; dum > game->player.numRemainingBarricades; dum--) {
        int cellIdx = game->player.barricades[bufIdx];
        int row = idxToRow(cellIdx, game->numCols);
        int col = idxToCol(cellIdx, game->numCols);
        int64_t oldHp = game->lvl[row][col].hp;
        game->lvl[row][col].hp -= (int64_t)dtUs;
        if ((oldHp / 1000000l) > (game->lvl[row][col].hp / 1000000l)) game->needsDraw = true; // to redraw timer on barrier
        if (game->lvl[row][col].hp < 0l) { // wall expired
            setCell(game, row, col, CELL_EMPTY, 0);
            game->player.bStart = (game->player.bStart + 1) % game->player.numBarricades;
            game->player.numRemainingBarricades++;
            game->needsDraw = true;

            // recalc rg path
            int pl;
            int* newPath = PATHFIND(game->adj, game->rg.curPos, game->endPos, game->numVerts, &pl);
            free(game->rg.path);
            game->rg.path = newPath;
            game->rg.pathProgress = 0;
            game->rg.pathLen = pl;
        }
        bufIdx = (bufIdx + 1) % game->player.numBarricades;
    }
    return;
}

int* pathfind_bfs2(bool** adj, int startPos, int endPos, int numVerts, int* pathLen) {
    int parents[numVerts];
    int q[numVerts], qs = 0, qe = 0;
    for (int i = 0; i < numVerts; i++) parents[i] = -1;
    
    q[qe] = startPos;
    qe++;

    while (qs < qe) {
        int cur = q[qs];
        qs++;
        DEBUG_PRINT("pathfind_bfs2 exploring node %d (tgt = %d)\n", cur, endPos);
        for (int i = 0; i < numVerts; i++) { // check cur's entry in adj
            if (i == cur) continue;
            if (adj[cur][i] && (parents[i] == -1)) { // for each of cur's neighbors, check if it's been explored yet
                q[qe] = i; // if not, add to queue and set its parent
                qe++;
                parents[i] = cur;
                if (i == endPos) { // found target node
                    int sz = 1;
                    int parent = parents[i];
                    while (parent != startPos) { sz++; parent = parents[parent]; }
                    int* out = (int*)malloc(sizeof(int) * sz);
                    for (int j = sz - 1; j >= 0; j--) {
                        out[j] = i;
                        i = parents[i];
                    }
                    *pathLen = sz;
                    return out;
                }
            }
        }
    }

    *pathLen = 0;
    return NULL;
}

void gameOver(Game* game) {
    game->state = GAME_STATE_EXIT;
    return;
}

/* Pathfinding algorithms */
int* pathfind_bfs(bool** adj, int startVert, int endVert, int numVerts, int* pathLen) {
    int parent[numVerts];
    int queue[numVerts];
    int queueStart = -1;
    int queueEnd = 0;
    int curVert;

    // haven't explored anything yet
    for (int i = 0; i < numVerts; i++) parent[i] = -1;

    // push the starting node into the queue and mark it as its own parent
    queue[++queueStart] = startVert;
    parent[startVert] = startVert;

    while (queueStart <= queueEnd) {
        curVert = queue[queueStart++];
        DEBUG_PRINT("pathfind_bfs exploring vert %d\n", curVert);
        for (int neighborVert = 0; neighborVert < numVerts; neighborVert++) {
            // found target
            if (adj[curVert][neighborVert] && (neighborVert == endVert)) {
                parent[neighborVert] = curVert;
                int sz = 1, n = curVert; // initialize backtrackVert to neighborVert; sz to 1, to include starting vert
                while (n != startVert) { sz++; n = parent[n]; } // determine size of path
                *pathLen = sz;
                int* out = (int*)malloc(sizeof(int) * sz);
                n = curVert; // backtrack once more to store the path
                out[--sz] = endVert;
                while (n != startVert) { out[--sz] = n; n = parent[n]; }
                return out;
            }

            // not the target yet
            if (adj[curVert][neighborVert] && (parent[neighborVert] == -1)) { // found a new node: enqueue it and set its parent
                queue[++queueEnd] = neighborVert;
                parent[neighborVert] = curVert;
            }
        }
    }

    // target not found
    *pathLen = 0;
    return NULL;
}

/* Drawing */
void paintCh(char c, WINDOW* win, int row, int col, int colorpair) {
    wattron(win, COLOR_PAIR(colorpair));
    mvwaddch(win, row, col, c);
    wattroff(win, COLOR_PAIR(colorpair));
}

void gameDraw(Game* game) {
    game->needsDraw = false;
    werase(game->window);
    werase(game->statswindow);
    
    // first draw play field
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

    // draw red guy path, excluding start/end
    for (int i = game->rg.pathProgress; i < game->rg.pathLen - 1; i++) {
        paintCh(CHAR_REDGUY_PATH, game->window, idxToRow(game->rg.path[i], game->numCols), idxToCol(game->rg.path[i], game->numCols), COLOR_CELL_EMPTY);
    }

    // draw red guy
    paintCh(CHAR_REDGUY, game->window, idxToRow(game->rg.curPos, game->numCols), idxToCol(game->rg.curPos, game->numCols), COLOR_REDGUY);

    // draw temp wall hps
    for (int bufIdx = game->player.bStart, dum = game->player.numBarricades; dum > game->player.numRemainingBarricades; dum--) {
        int cellIdx = game->player.barricades[bufIdx];
        int row = idxToRow(cellIdx, game->numCols);
        int col = idxToCol(cellIdx, game->numCols);
        if (game->lvl[row][col].hp > 10000000l) continue; // >10s remaining, don't draw any char
        paintCh('0' + (game->lvl[row][col].hp / 1000000l), game->window, row, col, COLOR_CELL_TEMP_WALL);
        bufIdx = (bufIdx + 1) % game->player.numBarricades;
    }

    // draw player/cursor
    paintCh(CHAR_PLAYER, game->window, idxToRow(game->player.curPos, game->numCols), idxToCol(game->player.curPos, game->numCols), COLOR_PLAYER);

    // draw stats/score/etc; we've assigned 4 lines below the board for these (in gameInit)
    mvwprintw(game->statswindow, 0, 0, "SCORE: %d", game->score);
    mvwprintw(game->statswindow, 1, 0, "#BARRIERS: %d", game->player.numRemainingBarricades);
    mvwprintw(game->statswindow, 2, 0, "SPEED: %5li", (10000000l) / game->rg.basePatience);

    wrefresh(game->window);
    wrefresh(game->statswindow); 
    return;
}

void drawOverlay(const char* msg) {
    int h, w;
    getmaxyx(stdscr, h, w);
    mvprintw(h / 2, (w - strlen(msg)) / 2, msg);
    refresh();
    return;
}
