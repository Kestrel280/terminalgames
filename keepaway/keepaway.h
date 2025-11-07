typedef struct _cell Cell;

struct _cell {
    bool open; // Whether the cell can be passed through
};

int gridToIdx(int row, int col);
void setCell(Cell** lvl, int** graph, int row, int col, bool newState);
