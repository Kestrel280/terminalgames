#ifndef MENU_H
#define MENU_H

typedef int MenuOption;
enum {
    MENU_OPTION_PLAY,
    MENU_OPTION_LEADERBOARD,
    MENU_OPTION_QUIT,
    MENU_OPTION_META_NUM_OPTIONS
};

extern const char* const menuString[];
MenuOption menu();

#endif
