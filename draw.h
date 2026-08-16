#pragma once

#include <stdbool.h>
#include <ncurses.h>

#include "grid.h"

void draw_grid(WINDOW *board, Square *grid, int board_width, int board_height, bool show_all);
bool move_cursor(WINDOW *win, Square *grid, int width, int height, int *cursor_x, int *cursor_y, int *flag_n);
