#pragma once

#include <stdbool.h>

// 1 bit mine
// 1 bit uncovered
// 1 bit flag
// 5 bits surrounding

typedef char Square;
#define GET_MINE(s)         ((bool)(((s) & 0x80) != 0))
#define GET_UNCOVERED(s)    ((bool)(((s) & 0x40) != 0))
#define GET_FLAG(s)         ((bool)(((s) & 0x20) != 0))
#define GET_SURROUNDING(s)  ((int)((s) & 0x1F))

/*
 * The square is read once, modified in a temporary and written back last, so
 * `v` may safely read the same square (e.g. SET_FLAG(*s, !GET_FLAG(*s))).
 */
#define SET_MINE(s, v)          do { Square _set_sq = (s); _set_sq = (_set_sq & ~0x80) | (Square)((v) ? 0x80 : 0x00); (s) = _set_sq; } while(0)
#define SET_UNCOVERED(s, v)     do { Square _set_sq = (s); _set_sq = (_set_sq & ~0x40) | (Square)((v) ? 0x40 : 0x00); (s) = _set_sq; } while(0)
#define SET_FLAG(s, v)          do { Square _set_sq = (s); _set_sq = (_set_sq & ~0x20) | (Square)((v) ? 0x20 : 0x00); (s) = _set_sq; } while(0)
#define SET_SURROUNDING(s, v)   do { Square _set_sq = (s); _set_sq = (_set_sq & ~0x1F) | (Square)((unsigned) (v) & 0x1F); (s) = _set_sq; } while(0)

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
void init_grid(Square *grid, int width, int height, bool *mine_positions);
Square *create_grid(int width, int height, int mines, int x, int y, int safe_zone, bool no_guess);
int get_bbbv(Square *grid, int width, int height);
