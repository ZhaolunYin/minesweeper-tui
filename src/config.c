#include "ms.h"

const Preset BEGINNER = {
    .width = 9,
    .height = 9,
    .mines = 10,
};

const Preset INTERMEDIATE = {
    .width = 16,
    .height = 16,
    .mines = 40,
};

const Preset EXPERT = {
    .width = 30,
    .height = 16,
    .mines = 99,
};

const Preset CUSTOM_DEFAULTS = {
    .width = 30,
    .height = 16,
    .mines = 99,
};

void load_preset(const Preset preset, int *width, int *height, int *mines) {
    *width = preset.width;
    *height = preset.height;
    *mines = preset.mines;
}
