#include "keepaway.h"

// Globals
int numRows, numCols;
int numCells;

int gridToIdx(int row, int col) { return (col + (row * numCols)); }

void setCell(Cell** lvl, int** graph, int row, int col, bool newState) {
    int myIdx = gridToIdx(row, col);
    lvl[row][col].open = newState;

    for (int y = row - 1; y <= row + 1; y += 2) {
        if ((y < 0) || (y >= numRows)) continue; // oob check
        int nidx = gridToIdx(y, col);
        graph[nidx][myIdx] = newState;
    }
    for (int x = col - 1; x <= col + 1; x+= 2) {
        if ((x < 0) || (x >= numCols)) continue; // oob check
        int nidx = gridToIdx(row, x);
        graph[nidx][myIdx] = newState;
    }
}

int main() {
    return 0;
}
