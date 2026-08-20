#pragma once
#include <stdbool.h>

int select_difficulty(int term_width, int term_height);
bool select_play_again(int term_width, bool win);
void select_custom(int *result, int term_width, int term_height);
