#include "board.h"

int width_to_board_width(int width) {
    // Each cell is 2x1
    // Last one is 1x1
    // 2 for border
    return (width * 2) + 1;
}

int height_to_board_height(int height) {
    // 1 for stats
    // 2 for border
    return height + 3;
}

int board_width_to_width(int board_width) {
    return (board_width - 1) / 2;
}

int board_height_to_height(int board_height) {
    return board_height - 3;
}
