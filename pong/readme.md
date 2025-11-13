Game #1

**Controls** Arrow keys

**Notes**
This game uses a fixed window size. If it does not render properly, try zooming out/increasing the size of the terminal window. Build using `make normal` or `make interp` (see Extras) (ncurses required).

**Game**
A basic implementation of Pong, with a couple small twists: paddles can be moved horizontally (but not in the inner third of the board), and the player can impart momentum on the ball with the paddle (but not the cpu).

**Purpose**
Get up to speed with ncurses.

**Extras**
As an exercise, the rendering and the "physics" are decoupled: physics "ticks" run at a fixed rate while rendering is unlimited. The game can be built using "interp" mode which tries to predict ball/paddle motion in between physics "ticks". This can cause some odd behavior around the edges of the board and may cause some misleading hits/misses.

**Demo**
(Note: Demo uses "interp" build of the game, hence some rendering issues as noted in Extras)
[![Pong](https://img.youtube.com/vi/Pjk9j0MbQY4/0.jpg)](https://www.youtube.com/watch?v=Pjk9j0MbQY4)

