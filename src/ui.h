#pragma once
#include <stdbool.h>

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

#define PLAY_AGAIN_LINES 6
#define PLAY_AGAIN_OPS 2

#define CUSTOM_LINES 3
#define CUSTOM_MAX_DIGITS 3

int select_difficulty(int term_width, int term_height);
bool select_play_again(int term_width, bool win, long long score, long long highscore);
void select_custom(int *result, int term_width, int term_height);
