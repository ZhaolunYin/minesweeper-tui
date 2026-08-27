#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "ms.h"
#include "ms/log.h"

#define MAX_RETRIES (1024*1024)

/// Function to search for item in list. Returns number of occurences
static int _find(int *list, int len, int item) {
    int c = 0;
    for (int i = 0; i < len; i++) {
        if (list[i] == item) {
            c++;
        }
    }
    return c;
}

/// Returns true if two squares are within `dist` cells horizontally and vertically.
static bool _in_safe_zone(int width, int pos1, int pos2, int dist) {
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

/// Returns pointer to square in grid
Square *get_square(Square *grid, int width, int height, int x, int y) {
    if (y >= 0 && y < height && x >= 0 && x < width)
        return &grid[y * width + x];
    else
        return NULL;
}

/// Select a square in a grid. Returns number of squares selected or -1 if a mine was selected. Returns 0 if selection was invalid
int select_square(Square **grid, int width, int height, int x, int y) {
    LOG(LOG_DEBUG, "Selecting square (%d, %d)", x, y);
    Square *square = get_square(*grid, width, height, x, y);
    if (!square) {
        LOG(LOG_ERROR, "Failed to select square");
        return 0;
    }

    if (square->flag) {
        LOG(LOG_DEBUG, "Square was flagged");
        return 0;
    }

    if (square->mine) {
        LOG(LOG_DEBUG, "Square was a mine");
        return -1;
    }

    int uncovered = 0;
    if (square->uncovered) {
        LOG(LOG_DEBUG, "Square was uncovered");
        int flags = 0;
        FOR_EACH_NEIGHBOUR(x, y, width, height) {
            flags += get_square(*grid, width, height, rx, ry)->flag;
        }

        if (flags != square->surrounding)
            return 0;

        LOG(LOG_DEBUG, "Performing a chord");

        FOR_EACH_NEIGHBOUR(x, y, width, height) {
            if (get_square(*grid, width, height, rx, ry)->uncovered)
                continue;
            int result = select_square(grid, width, height, rx, ry);
            if (result == -1)
                return -1;
            uncovered += result;
        }
        return uncovered;
    }
    uncovered = 1;
    square->uncovered = true;
    LOG(LOG_DEBUG, "Uncovered square");
    if (square->surrounding)
        return uncovered;

    LOG(LOG_DEBUG, "Square had no neighbouring mines. Uncovering neighbouring squares");
    FOR_EACH_NEIGHBOUR(x, y, width, height) {
        int result = select_square(grid, width, height, rx, ry);
        if (result == -1)
            return -1;
        uncovered += result;
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

/// Initialise grid from dimensions and mines
void init_grid(Square *grid, int width, int height, int mines, int *mine_positions) {
    LOG(LOG_DEBUG, "Initialising grid values");
    for (int i = 0; i < width * height; i++) {
        grid[i].surrounding = 0;
        grid[i].uncovered = false;
        grid[i].flag = false;
        grid[i].mine = (bool) _find(mine_positions, mines, i);
    }
    FOR_EACH_IN_GRID(grid, width, height) {
        FOR_EACH_NEIGHBOUR(x, y, width, height) {
            if (get_square(grid, width, height, rx, ry)->mine)
                get_square(grid, width, height, x, y)->surrounding++;
        }
    }
}

/// Returns grid as a Square *. Square (x, y) of grid can be accessed using the notation grid[y * width + x]
Square *create_grid(int width, int height, int mines, int x, int y, int safe_zone, bool no_guess) {
    LOG(LOG_INFO, "Generating grid");
    Square *grid = (Square *) malloc((size_t) width * height * sizeof(Square));
    if (!grid) {
        LOG(LOG_ERROR, "Failed to allocate memory for grid");
        return NULL;
    }
    for (int i = 0; i < MAX_RETRIES; i++) {
        int *mine_positions = (int *) malloc((size_t) mines * sizeof(int));
        if (!mine_positions) {
            LOG(LOG_ERROR, "Failed to allocate memory for list of mine positions");
            return NULL;
        }
        for (int i = 0; i < mines; i++)
            mine_positions[i] = -1;
        for (int i = 0; i < mines; i++) {
            do {
                mine_positions[i] = rand() % (width * height);
            } while (_find(mine_positions, mines, mine_positions[i]) > 1 ||
                    _in_safe_zone(width, y * width + x, mine_positions[i], safe_zone));
        }
        init_grid(grid, width, height, mines, mine_positions);
        free(mine_positions);
        if (!no_guess || solve_board(grid, width, height, x, y)) {
            LOG(LOG_INFO, "Valid grid found");
            break;
        }
        LOG(LOG_DEBUG, "Invalid grid. Attempt %d", i);
    }
    if (select_square(&grid, width, height, x, y) == -1) {
        LOG(LOG_ERROR, "First square was a mine despite safety zone.");
        free(grid);
        return NULL;
    }
    return grid;
}

/// Flood-fills from (x, y) marking every connected empty, unmined square as visited.
static void _3bv_flood_fill(Square *grid, int width, int height, int x, int y, bool *visited) {
    Square *square = get_square(grid, width, height, x, y);
    if (!square->mine && !visited[y * width + x]) {
        visited[y * width + x] = true;
        if (!square->surrounding) {
            LOG(LOG_INFO, "Found zero while calculating 3BV. Flood filling");
            FOR_EACH_NEIGHBOUR(x, y, width, height) {
                _3bv_flood_fill(grid, width, height, rx, ry, visited);
            }
        }
    }
}

/// Estimates the 3BV (three-button visits) score of the grid.
int get_bbbv(Square *grid, int width, int height) {
    LOG(LOG_INFO, "Calculating 3BV");
    int bbbv = 0;

    bool *visited = calloc((size_t) width * height, sizeof(bool));
    if (!visited) {
        LOG(LOG_ERROR, "Failed to allocate memory for list of visited squares");
        return -1;
    }
    FOR_EACH_IN_GRID(grid, width, height) {
        Square *square = get_square(grid, width, height, x, y);
        if (!square->mine && !square->surrounding && !visited[y * width + x]) {
            bbbv++;
            _3bv_flood_fill(grid, width, height, x, y, visited);
        }
    }
    FOR_EACH_IN_GRID(grid, width, height) {
        Square *square = get_square(grid, width, height, x, y);
        if (!square->mine && square->surrounding && !visited[y * width + x]) {
            bbbv++;
        }
    }
    free(visited);
    return bbbv;
}
