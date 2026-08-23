#pragma once
#include "grid.h"
#include <ncurses.h>
#include <stdbool.h>

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

    int cursor_x;
    int cursor_y;

    int max_x;
    int max_y;
 
    WINDOW *board;
} Game;

#define FOLDER_NAME "minesweeper-tui"
#define SCORE_FILE "score"

void show_highscores();
Game init_game(int width, int height, int mines, char *difficulty, bool no_guess);
void game_loop(Game *game);
bool end_game(Game *game, bool save_highscores);
