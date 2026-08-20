#pragma once
#include "grid.h"
#include <ncurses.h>
#include <stdbool.h>

typedef struct {
    int difficulty;
    int width;
    int height;
    int mines;
    int flags;
    bool win;
    Square *grid;

    long long score;
    long long highscore;

    int cursor_x;
    int cursor_y;

    int max_x;
    int max_y;
 
    WINDOW *board;
} Game;

#define FOLDER_NAME "minesweeper-tui"
#define SCORE_FILE "score"

Game init_game();
void game_loop(Game *game);
bool end_game(Game *game);
