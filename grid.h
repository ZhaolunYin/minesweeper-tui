#pragma once

#include <stdbool.h>

typedef struct {
    bool mine;
    int surrounding;
    bool uncovered;
    bool flag;
} Square;

int find(int *list, int len, int item);
int select_square(Square **grid, int width, int height, int x, int y);
bool all_selected(Square *grid, int width, int height);
Square *create_grid(int width, int height, int mines, int x, int y);
