#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "ms.h"

#define MAX_RETRIES (1024*1024)

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

    if (GET_FLAG(*square)) {
        LOG(LOG_DEBUG, "Square was flagged");
        return 0;
    }

    if (GET_MINE(*square)) {
        LOG(LOG_DEBUG, "Square was a mine");
        return -1;
    }

    int uncovered = 0;
    if (GET_UNCOVERED(*square)) {
        LOG(LOG_DEBUG, "Square was uncovered");
        int flags = 0;
        FOR_EACH_NEIGHBOUR(x, y, width, height) {
            flags += GET_FLAG(*get_square(*grid, width, height, rx, ry));
        }

        if (flags != GET_SURROUNDING(*square))
            return 0;

        LOG(LOG_DEBUG, "Performing a chord");

        FOR_EACH_NEIGHBOUR(x, y, width, height) {
            if (GET_UNCOVERED(*get_square(*grid, width, height, rx, ry)))
                continue;
            int result = select_square(grid, width, height, rx, ry);
            if (result == -1)
                return -1;
            uncovered += result;
        }
        return uncovered;
    }
    uncovered = 1;
    SET_UNCOVERED(*square, true);
    LOG(LOG_DEBUG, "Uncovered square");
    if (GET_SURROUNDING(*square))
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
        if (!GET_UNCOVERED(grid[i]) && !GET_MINE(grid[i])) {
            return false;
        }
    }
    return true;
}

/// Initialise grid from dimensions and mines
void init_grid(Square *grid, int width, int height, bool *mine_positions) {
    LOG(LOG_DEBUG, "Initialising grid values");
    for (int i = 0; i < width * height; i++) {
        grid[i] = 0;
        SET_MINE(grid[i], mine_positions[i]);
    }
    FOR_EACH_IN_GRID(grid, width, height) {
        Square *square = get_square(grid, width, height, x, y);
        FOR_EACH_NEIGHBOUR(x, y, width, height) {
            if (GET_MINE(*get_square(grid, width, height, rx, ry)))
                SET_SURROUNDING(*square, GET_SURROUNDING(*square) + 1);
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
    for (int c = 0; c < MAX_RETRIES; c++) {
        bool *mine_positions = calloc(width * height, sizeof(bool));
        if (!mine_positions) {
            LOG(LOG_ERROR, "Failed to allocate memory for list of mine positions");
            return NULL;
        }
        for (int i = 0; i < mines; i++) {
            int m;
            do {
                m = rand() % (width * height);
            } while (mine_positions[m] ||
                    _in_safe_zone(width, y * width + x, m, safe_zone));
            mine_positions[m] = true;
        }
        init_grid(grid, width, height, mine_positions);
        free(mine_positions);
        if (!no_guess || solve_board(grid, width, height, x, y)) {
            LOG(LOG_INFO, "Valid grid found");
            break;
        }
        LOG(LOG_DEBUG, "Invalid grid. Attempt %d", c);
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
    if (!GET_MINE(*square) && !visited[y * width + x]) {
        visited[y * width + x] = true;
        if (!GET_SURROUNDING(*square)) {
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
        if (!GET_MINE(*square) && !GET_SURROUNDING(*square) && !visited[y * width + x]) {
            bbbv++;
            _3bv_flood_fill(grid, width, height, x, y, visited);
        }
    }
    FOR_EACH_IN_GRID(grid, width, height) {
        Square *square = get_square(grid, width, height, x, y);
        if (!GET_MINE(*square) && GET_SURROUNDING(*square) && !visited[y * width + x]) {
            bbbv++;
        }
    }
    free(visited);
    return bbbv;
}
