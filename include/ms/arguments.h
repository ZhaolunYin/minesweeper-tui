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
    char *filename;

    int seed;
    bool save_highscore;

    bool color;
};

struct arguments default_args();
extern struct argp argp;
