#pragma once

#define BOX_WIDTH 2
#define STATS_ROW 1
#define WIDTH_MUL 2
#define TRIM_SPACE -1

#define width_to_board_width(width) \
    (((width) * WIDTH_MUL) + BOX_WIDTH + TRIM_SPACE)

#define height_to_board_height(height) \
    ((height) + BOX_WIDTH + STATS_ROW)

#define board_width_to_width(board_width) \
    ((board_width) - BOX_WIDTH - TRIM_SPACE) / WIDTH_MUL

#define board_height_to_height(board_height) \
    ((board_height) - BOX_WIDTH - STATS_ROW)

#define x_to_board_x(x) \
    (((x) * WIDTH_MUL) + BOX_WIDTH / 2)

#define y_to_board_y(y) \
    ((y) + BOX_WIDTH / 2 + STATS_ROW)

#define board_x_to_x(board_x) \
    ((board_x) - BOX_WIDTH / 2) / WIDTH_MUL

#define board_y_to_y(board_y) \
    ((board_y) - STATS_ROW - BOX_WIDTH / 2)
