#pragma once
#include "grid.h"
#include <ncurses.h>
#include <stdbool.h>

typedef struct {
    int width;
    int height;
    int mines;
    int flags;
    bool win;
    Square *grid;

    int cursor_x;
    int cursor_y;

    int max_x;
    int max_y;
    
    WINDOW *board;
} Game;

Game init_game();
void game_loop(Game *game);
bool end_game(Game *game);
