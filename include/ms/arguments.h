#pragma once
#include <stdbool.h>
#include <argp.h>

#define NO_SAVE 1000
#define NO_COLOR 1001

struct arguments {
    int width;
    int height;
    int mines;
    char *difficulty;
    bool no_guess;
    char *import_filename;
    char *export_filename;

    int seed;
    bool save_highscore;

    bool color;
};

struct arguments default_args(void);
extern struct argp argp;
