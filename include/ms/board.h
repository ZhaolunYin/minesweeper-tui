#pragma once

#define BOX_WIDTH 2
#define STATS_ROW 1
#define WIDTH_MUL 2
#define TRIM_SPACE -1

int width_to_board_width(int width);
int height_to_board_height(int height);
int board_width_to_width(int board_width);
int board_height_to_height(int board_height);

int x_to_board_x(int x);
int y_to_board_y(int y);
int board_x_to_x(int board_x);
int board_y_to_y(int board_y);
