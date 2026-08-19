#include "grid.h"
#include <stdlib.h>

/// Function to search for item in list. Returns number of occurences
int _find(int *list, int len, int item) {
    int c = 0;
    for (int i = 0; i < len; i++) {
        if (list[i] == item) {
            c++;
        }
    }
    return c;
}

bool _in_safe_zone(int width, int pos1, int pos2, int dist) {
    int x1 = pos1 % width;
    int y1 = pos1 / width;
    int x2 = pos2 % width;
    int y2 = pos2 / width;
    int xdistance = x1 - x2;
    int ydistance = y1 - y2;
    xdistance = (xdistance >= 0) ? xdistance : -xdistance;
    ydistance = (ydistance >= 0) ? ydistance : -ydistance;
    return (xdistance <= dist && ydistance <= dist);
}

/// Select a square in a grid. Returns number of squares selected or -1 if a mine was selected. Returns 0 if selection was invalid
int select_square(Square **grid, int width, int height, int x, int y) {
    if (x >= width || x < 0 || y >= height || y < 0) {
        return 0;
    }
    Square *square = &(*grid)[y * width + x];
    if (square->flag)
        return 0;
    if (square->mine)
        return -1;
    int uncovered = 0;
    if (square->uncovered) {
        int flags = 0;
        for (int ry = -1; ry <= 1; ry++) {
            for (int rx = -1; rx <= 1; rx++) {
                if (!rx && !ry)
                    continue;
                int row = y + ry;
                int col = x + rx;
                if (row >= 0 && row < height && col >= 0 && col < width) {
                    flags += (*grid)[row * width + col].flag;
                }
            }
        }
        if (flags != square->surrounding)
            return 0;
        for (int ry = -1; ry <= 1; ry++) {
            for (int rx = -1; rx <= 1; rx++) {
                if (!rx && !ry)
                    continue;
                int row = y + ry;
                int col = x + rx;
                if (row >= 0 && row < height && col >= 0 && col < width) {
                    if ((*grid)[row * width + col].uncovered)
                        continue;
                    int result = select_square(grid, width, height, col, row);
                    if (result == -1)
                        return -1;
                    uncovered += result;
                }
            }
        }
        return uncovered;
    }
    uncovered = 1;
    square->uncovered = true;
    if (square->surrounding)
        return uncovered;
    for (int ry = -1; ry <= 1; ry++) {
        for (int rx = -1; rx <= 1; rx++) {
            if (!rx && !ry)
                continue;
            int row = y + ry;
            int col = x + rx;
            if (row >= 0 && row < height && col >= 0 && col < width) {
                int result = select_square(grid, width, height, col, row);
                if (result == -1)
                    return -1;
                uncovered += result;
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
    for (int i = 0; i < mines; i++)
        mine_positions[i] = -1;
    for (int i = 0; i < mines; i++) {
        do {
            mine_positions[i] = rand() % (width * height);
        } while (_find(mine_positions, mines, mine_positions[i]) > 1 ||
                _in_safe_zone(width, y * width + x, mine_positions[i], 1));
    }
    for (int i = 0; i < width * height; i++) {
        grid[i].mine = false;
        grid[i].surrounding = 0;
        grid[i].uncovered = false;
        grid[i].flag = false;
        grid[i].mine = (bool) _find(mine_positions, mines, i);
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
    free(mine_positions);
    if (select_square(&grid, width, height, x, y) == -1)
        return NULL;
    return grid;
}
