#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ms.h"

const char *argp_program_version = 
    "minesweeper-tui " VERSION "\n"
    "Copyright (C) 2026 Zhaolun Yin\n"
    "License: MIT";
const char *argp_program_bug_address = "https://github.com/ZhaolunYin/minesweeper-tui/issues";

struct arguments default_args(void) {
    struct arguments args;
    args.width = 0;
    args.height = 0;
    args.mines = 0;
    args.difficulty = NULL;
    args.no_guess = false;
    args.import_filename = NULL;
    args.export_filename = NULL;

    args.seed = time(NULL);
    args.save_highscore = true;

    args.color = true;
    return args;
}

static struct argp_option options[] = {
    { "\nBoard options", 0, 0, OPTION_DOC, 0, 0 },
    { "width",      'w', "WIDTH",       0, "Width of board.", 1 },
    { "height",     'h', "HEIGHT",      0, "Height of board.", 1 },
    { "mines",      'm', "MINES",       0, "Number of mines in grid.", 1 },

    { "difficulty", 'd', "LEVEL",       0, "Difficulty of board.", 2 },
    { "no-guess",   'n', 0,             0, "Generate a board that does not require guessing.", 2 },

    { "import",     'i', "FILE",        0, "Import board from file.", 3 },
    { "export",     'e', "FILE",        0, "Export board to file.", 3 },

    { "\nGame options", 0, 0, OPTION_DOC, 0, 4 },
    { "seed",       's', "SEED",        0, "Seed for randomly generated board.", 5 },
    { "highscores", 'H', 0,             0, "Show highscores.", 5 },
    { "no-save",    NO_SAVE, 0,         0, "Do not save highscores.", 5 },

    { "\nColor options", 0, 0, OPTION_DOC, 0, 6 },
    { "no-color",   NO_COLOR, 0,        0, "Do not use color.", 7 },

    { 0 },
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'w':
            arguments->width = atoi(arg);
            break;
        case 'h':
            arguments->height = atoi(arg);
            break;
        case 'm':
            arguments->mines = atoi(arg);
            break;
        case 'd':
            bool valid = false;
            for (char *p = arg; *p; p++) {
                *p = tolower(*p);
            }
            for (int i = 0; i < DIFFICULTY_OPS - 1; i++) {
                char level[BUFSIZ];
                snprintf(level, BUFSIZ, "%s", DIFFICULTIES[i + DIFFICULTY_LINES - DIFFICULTY_OPS]);
                for (char *p = level; *p; p++) {
                    *p = tolower(*p);
                }
                if (strcmp(arg, level) == 0)
                    valid = true;
            }
            if (!valid)
                argp_error(state, "LEVEL must be either \'Beginner\', \'Intermediate\' or \'Expert\'");
            arguments->difficulty = arg;
            break;
        case 'n':
            arguments->no_guess = true;
            break;
        case 'i':
            arguments->import_filename = arg;
            break;
        case 'e':
            arguments->export_filename = arg;
            break;
        case 's':
            arguments->seed = atoi(arg);
            break;
        case 'H':
            show_highscores();
            exit(0);
        case NO_SAVE:
            arguments->save_highscore = false;
            break;
        case NO_COLOR:
            arguments->color = false;
            break;
        case ARGP_KEY_END:
            if ((arguments->width || arguments->height || arguments->mines) && arguments->difficulty)
                argp_error(state, "--width, --height and --mines cannot be used with --difficulty");
            if (arguments->no_guess && !arguments->difficulty)
                argp_error(state, "--no-guess must be used with --difficulty");
            if (arguments->import_filename && arguments->export_filename)
                argp_error(state, "--import and --export cannot be used together");
            if (arguments->import_filename &&
                    (arguments->height || arguments->width || arguments->mines ||
                     arguments->difficulty || arguments->no_guess || arguments->seed))
                argp_error(state, "--import cannot be used with any board options or --seed");
    }
    return 0;
}

static char doc[] = "A terminal-based Minesweeper game";
static char args_doc[] = "";

struct argp argp = {
    .options = options,
    .parser = parse_opt,
    .args_doc = args_doc,
    .doc = doc,
};
