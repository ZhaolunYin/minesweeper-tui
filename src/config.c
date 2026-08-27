#include "ms.h"
#include "ms/log.h"

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
    .height = 30,
    .mines = 160,
};

/// Copies a preset's width, height and mines into the provided pointers.
void load_preset(const Preset preset, int *width, int *height, int *mines) {
    *width = preset.width;
    *height = preset.height;
    *mines = preset.mines;
    LOG(LOG_DEBUG, "Loaded preset: Width: %d Height: %d Mines: %d", *width, *height, *mines);
}
