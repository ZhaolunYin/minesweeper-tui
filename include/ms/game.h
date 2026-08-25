#pragma once
#include <ncurses.h>
#include <stdbool.h>

#include "grid.h"
#include "arguments.h"

typedef struct {
    int difficulty;
    int no_guess;
    int width;
    int height;
    int mines;
    int flags;
    bool win;
    Square *grid;

    long long score;
    long long highscore;
    int clicks;

    int cursor_x;
    int cursor_y;
    int start_x;
    int start_y;

    int max_x;
    int max_y;
 
    WINDOW *board;
    struct arguments* args;
} Game;

Game *init_game(struct arguments *args);
void game_loop(Game *game);
bool end_game(Game *game);
