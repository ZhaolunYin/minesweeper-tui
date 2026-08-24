#pragma once

typedef struct {
    int width;
    int height;
    int mines;
} Preset;

extern const Preset BEGINNER;
extern const Preset INTERMEDIATE;
extern const Preset EXPERT;
extern const Preset CUSTOM_DEFAULTS;

void load_preset(const Preset preset, int *width, int *height, int *mines);
