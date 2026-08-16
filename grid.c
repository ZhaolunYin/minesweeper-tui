#include "grid.h"
#include <stdlib.h>

/// Function to search for item in list. Returns number of occurences
int find(int *list, int len, int item) {
    int c = 0;
    for (int i = 0; i < len; i++) {
        if (list[i] == item) {
            c++;
        }
    }
    return c;
}

/// Select a square in a grid. Returns number of squares selected or -1 if a mine was selected. Returns 0 if selection was invalid
int select_square(Square **grid, int width, int height, int x, int y) {
    if (x >= width || x < 0 || y >= height || y < 0) {
        return 0;
    }
    Square *square = &(*grid)[y * width + x];
    if (square->flag || square->uncovered)
        return 0;
    if (square->mine)
        return -1;
    square->uncovered = true;
    int uncovered = 1;
    if (square->surrounding)
        return uncovered;
    for (int ry = -1; ry <= 1; ry++) {
        for (int rx = -1; rx <= 1; rx++) {
            if (!rx && !ry)
                continue;
            int row = y + ry;
            int col = x + rx;
            if (row >= 0 && row < height && col >= 0 && col < width) {
                uncovered += select_square(grid, width, height, col, row);
            }
        }
    }
    return uncovered;
}

/// Returns true if all non-mine squares have been selected.
bool all_selected(Square *grid, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        if (!grid[i].uncovered && !grid[i].mine) {
            return false;
        }
    }
    return true;
}

/// Returns grid as a Square *. Square (x, y) of grid can be accessed using the notation grid[y * height + x]
Square *create_grid(int width, int height, int mines, int x, int y) {
    Square *grid = (Square *) malloc(width * height * sizeof(Square));
    int *mine_positions = (int *) malloc(mines * sizeof(int));
    for (int i = 0; i < mines; i++) {
        do {
            mine_positions[i] = rand() % (width * height);
        } while (mine_positions[i] == y * width + x || find(mine_positions, mines, mine_positions[i]) > 1);
    }
    for (int i = 0; i < width * height; i++) {
        grid[i].mine = false;
        grid[i].surrounding = 0;
        grid[i].uncovered = false;
        grid[i].flag = false;
        grid[i].mine = (bool) find(mine_positions, mines, i);
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int ry = -1; ry <= 1; ry++) {
                for (int rx = -1; rx <= 1; rx++) {
                    if (!rx && !ry)
                        continue;
                    int row = i + ry;
                    int col = j + rx;
                    if (row >= 0 && row < height && col >= 0 && col < width) {
                        if (grid[row * width + col].mine)
                            grid[i * width + j].surrounding++;
                    }
                }
            }
        }
    }
    if (select_square(&grid, width, height, x, y) == -1)
        return NULL;
    free(mine_positions);
    return grid;
}
