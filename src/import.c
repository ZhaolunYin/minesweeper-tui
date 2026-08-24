#include <stdio.h>
#include <stdlib.h>

#include "ms.h"
#include "ms/grid.h"

Square *import_grid(const char *filename, int *width, int *height, int *mines) {
    FILE *fptr = fopen(filename, "r");
    if (!fptr)
        return NULL;
    int x = 0, y = 0;
    int mine_positions[BUFSIZ];
    int mine_index = 0;
    int i = 0;
    int ch;
    bool selection = false;
    int selection_x, selection_y;
    while ((ch = fgetc(fptr)) != EOF) {
        if (ch == ' ') {
            continue;
        }
        else if (ch == '\n') {
            if (y && x != *width)
                return NULL;
            y++;
            *width = x;
            x = 0;
        }
        else {
            if (ch == '*') {
                mine_positions[mine_index++] = i;
            }
            if (ch == 'X') {
                selection = true;
                selection_x = x;
                selection_y = y;
            }
            x++;
            i++;
        }
    }
    *height = y;
    *mines = mine_index;
    Square *grid = malloc((size_t) *width * *height * sizeof(Square));
    fclose(fptr);
    init_grid(grid, *width, *height, *mines, mine_positions);
    if (selection)
        select_square(&grid, *width, *height, selection_x, selection_y);
    return grid;
}

void export_grid(Square *grid, int width, int height, int x, int y, char *filename) {
    FILE *fptr = fopen(filename, "w");
    if (!fptr) {
        return;
    }
    for (int gy = 0; gy < height; gy++) {
        for (int gx = 0; gx < width; gx++) {
            if (get_square(grid, width, height, gx, gy)->mine) {
                fprintf(fptr, "* ");
            }
            else if (gy == y && gx == x) {
                fprintf(fptr, "X ");
            }
            else {
                fprintf(fptr, ". ");
            }
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);
}
