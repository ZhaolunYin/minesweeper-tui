#pragma once

#include <stdbool.h>

typedef struct {
    bool mine;
    int surrounding;
    bool uncovered;
    bool flag;
} Square;

#define FOR_EACH_IN_GRID(grid, width, height) \
    for (int y = 0; y < height; y++) \
        for (int x = 0; x < width; x++)

#define FOR_EACH_NEIGHBOUR(x, y, width, height) \
    for (int dy = -1, ry; dy <= 1; dy++) \
        for (int dx = -1, rx; dx <= 1; dx++) \
            if ((dx || dy) && \
                (rx = (x) + dx) >= 0 && (rx) < (width) && \
                (ry = (y) + dy) >= 0 && (ry) < (height))

Square *get_square(Square *grid, int width, int height, int x, int y);
int select_square(Square **grid, int width, int height, int x, int y);
bool all_selected(Square *grid, int width, int height);
void init_grid(Square *grid, int width, int height, int mines, int *mine_positions);
Square *create_grid(int width, int height, int mines, int x, int y, int safe_zone, bool no_guess);
int get_bbbv(Square *grid, int width, int height);
