#ifndef MENU_H
#define MENU_H

typedef int MenuOption;
enum {
    MENU_PLAY,
    MENU_LEADERBOARD,
    MENU_QUIT
};

MenuOption menu();

#endif
