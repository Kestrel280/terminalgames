Game #2

**Controls**
Arrow keys, ENTER

**Notes**
If the game does not render, the terminal window may be too small. Try zooming out and increasing the size of the terminal. Build using `make` (ncurses required).

**Game**
There is a Red Guy ('#') trying to make it from the start of a grid to the end. Control the Player ('P') using the arrow keys and place ENTER to place down a temporary barricade to block the Red Guy's path\*, which is visualized using dots. You have a limited number of barricades, and they only last so long; try to keep the Red Guy from reaching the end for as long as possible.

\* Be careful -- you can't place a barricade in a spot that would completely block the Red Guy from reaching the exit! He must always have a valid path to reach the goal.

**Purpose**
Practice working with graphs, practice implementing a more complex game. Internally, an adjacency matrix is used for the game board, which is very space inefficient for a simple 2d grid. Pathfinding is done using BFS (adj matrix is also slow for this purpose!).

**Demo**
[![Keepaway](https://img.youtube.com/vi/gY19JTWQh0Q/0.jpg)](https://www.youtube.com/watch?v=gY19JTWQh0Q)
