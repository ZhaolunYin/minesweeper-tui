#pragma once
#include "grid.h"
Square *import_grid(const char *filename, int *width, int *height, int *mines);
void export_grid(Square *grid, int width, int height, int x, int y, char *filename);
