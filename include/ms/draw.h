#pragma once

#include <stdbool.h>
#include <ncurses.h>

#include "grid.h"

typedef enum {
    CLICK, FLAG, MOVE, NONE
} Action;

void init_color_pairs(void);
void draw_grid(WINDOW *board, Square *grid, int board_width, int board_height, int difficulty, bool show_all);
Action move_cursor(WINDOW *win, Square *grid, int width, int height, int *cursor_x, int *cursor_y, int *flag_n, int mine_n);
void draw_stats(WINDOW *board, int mine_total, int flags, int time);
