#pragma once
#include <stdbool.h>
#include "config.h"

#define DIFFICULTY_LINES 8
#define DIFFICULTY_OPS 7
#define DIFFICULTIES (const char *[DIFFICULTY_LINES]) { \
        "Select difficulty", \
        "Beginner", \
        "Beginner - No guess", \
        "Intermediate", \
        "Intermediate - No guess", \
        "Expert", \
        "Expert - No guess", \
        "Custom", \
}
#define CUSTOM_DIFFICULTY (DIFFICULTY_OPS - 1)

#define PLAY_AGAIN_LINES 6
#define PLAY_AGAIN_OPS 2

#define CUSTOM_LINES 3
#define CUSTOM_MAX_DIGITS 3
#define CUSTOM_MAX_VALUE 999

int select_difficulty(int term_width, int term_height, char *difficulty, bool no_guess);
bool select_play_again(int term_width, bool win, long long score, long long highscore);
Preset select_custom(int term_width, int term_height, Preset config);
