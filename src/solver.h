#pragma once
#include "grid.h"

typedef struct {
    int x[8];
    int y[8];
    Square *square[8];
    int n;
} Neighbours;

bool solve_board(Square *grid, int width, int height, int x, int y);
