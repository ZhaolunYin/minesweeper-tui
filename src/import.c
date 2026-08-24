#include <stdio.h>
#include <stdlib.h>

#include "ms.h"

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
    Square *grid = malloc(*width * *height * sizeof(Square));
    fclose(fptr);
    init_grid(grid, *width, *height, *mines, mine_positions);
    if (selection)
        select_square(&grid, *width, *height, selection_x, selection_y);
    return grid;
}
