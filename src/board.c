#include "ms.h"

int width_to_board_width(int width) {
    // Each cell is 2x1
    // Last one is 1x1
    // 2 for border
    return (width * WIDTH_MUL) + BOX_WIDTH + TRIM_SPACE;
}

int height_to_board_height(int height) {
    // 1 for stats
    // 2 for border
    return height + BOX_WIDTH + STATS_ROW;
}

int board_width_to_width(int board_width) {
    return (board_width - BOX_WIDTH - TRIM_SPACE) / WIDTH_MUL;
}

int board_height_to_height(int board_height) {
    return board_height - BOX_WIDTH - STATS_ROW;
}

int x_to_board_x(int x) {
    return x * WIDTH_MUL + BOX_WIDTH / 2;
}

int y_to_board_y(int y) {
    return y + BOX_WIDTH / 2 + STATS_ROW;
}

int board_x_to_x(int board_x) {
    return (board_x - BOX_WIDTH / 2) / WIDTH_MUL;
}

int board_y_to_y(int board_y) {
    return board_y - STATS_ROW - BOX_WIDTH / 2;
}
