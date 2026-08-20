#pragma once

#include <stdbool.h>

typedef struct {
    bool mine;
    int surrounding;
    bool uncovered;
    bool flag;
} Square;

#define FOR_EACH_NEIGHBOUR(x, y, width, height) \
    for (int dy = -1, ry; dy <= 1; dy++) \
        for (int dx = -1, rx; dx <= 1; dx++) \
            if ((dx || dy) && \
                (rx = (x) + dx) >= 0 && (rx) < (width) && \
                (ry = (y) + dy) >= 0 && (ry) < (height))

Square *get_square(Square *grid, int width, int height, int x, int y);
int select_square(Square **grid, int width, int height, int x, int y);
bool all_selected(Square *grid, int width, int height);
Square *create_grid(int width, int height, int mines, int x, int y, int safe_zone);
