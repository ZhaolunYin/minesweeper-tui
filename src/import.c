#include <stdio.h>
#include <stdlib.h>

#include "ms.h"

/// Imports a board from an ASCII file, validating it fits the current screen.
Square *import_grid(const char *filename, int *width, int *height, int *mines) {
    LOG(LOG_INFO, "Importing board");
    FILE *fptr = fopen(filename, "r");
    if (!fptr) {
        LOG(LOG_ERROR, "Failed to open %s", filename);
        return NULL;
    }
    int x = 0, y = 0;
    int mine_positions[BUFSIZ];
    size_t mine_index = 0;
    int i = 0;
    int ch;
    bool selection = false;
    int selection_x, selection_y;
    while ((ch = fgetc(fptr)) != EOF) {
        if (ch == ' ') {
            continue;
        }
        else if (ch == '\n') {
            if (y && x != *width) {
                LOG(LOG_ERROR, "Malformed file at %d:%d", x, y);
                return NULL;
            }
            y++;
            *width = x;
            x = 0;
        }
        else {
            if (ch == '*') {
                if (mine_index - 1 < BUFSIZ)
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
    fclose(fptr);
    *height = y;
    *mines = mine_index;
    Preset old = (Preset) {
        .width = *width,
        .height = *height,
        .mines = *mines,
    };
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);
    Preset new = old;
    clamp_custom(&new, max_x, max_y);
    if (new.width != old.width || new.height != old.height || new.mines != old.mines) {
        LOG(LOG_ERROR, "Invalid grid proportions");
        return NULL;
    }
    Square *grid = malloc((size_t) *width * *height * sizeof(Square));
    init_grid(grid, *width, *height, *mines, mine_positions);
    if (selection)
        select_square(&grid, *width, *height, selection_x, selection_y);
    return grid;
}

/// Exports a board to an ASCII file using '.', '*' and 'X' symbols.
void export_grid(Square *grid, int width, int height, int x, int y, char *filename) {
    LOG(LOG_INFO, "Exporting board");
    FILE *fptr = fopen(filename, "w");
    if (!fptr) {
        LOG(LOG_ERROR, "Failed to open file");
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
