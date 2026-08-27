#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "ms.h"
#include "ms/board.h"
#include "ms/grid.h"
#include "ms/log.h"

#define SAFE_ZONE 1
#define MIN_MINES 1

/// Clamps a preset's width, height and mines to valid, screen-fitting values.
void clamp_custom(Preset *preset, int max_x, int max_y) {
    // If win on one click - 
    // Safezone * 2 + 1 is minimum width / height first click uncovers
    if (preset->width < SAFE_ZONE * 2 + 2) {
        preset->width = SAFE_ZONE * 2 + 2;
        LOG(LOG_WARNING, "Width provided would not allow for safe zone. Using minimum width %d", SAFE_ZONE * 2 + 2);
    }
    if (preset->height < SAFE_ZONE * 2 + 2) {
        LOG(LOG_WARNING, "Height provided would not allow for safe zone. Using minimum height %d", SAFE_ZONE * 2 + 2);
        preset->height = SAFE_ZONE * 2 + 2;
    }

    // If bigger than screen
    if (preset->width > board_width_to_width(max_x) - 1) {
        LOG(LOG_WARNING, "Width bigger than screen width (%d). Clamping to screen height", board_width_to_width(max_x) - 1);
        preset->width = board_width_to_width(max_x) - 1;
    }
    if (preset->height > board_height_to_height(max_y) - 1) {
        LOG(LOG_WARNING, "Height bigger than screen height (%d). Clamping to screen height", board_height_to_height(max_y) - 1);
        preset->height = board_height_to_height(max_y) - 1;
    }

    // If too little mines
    if (preset->mines < MIN_MINES) {
        LOG(LOG_WARNING, "Must have at least %d mines. Using %d mines.", MIN_MINES, MIN_MINES);
        preset->mines = MIN_MINES;
    }

    // If no of mines causes 1st click win or impossible 1st click
    if (preset->mines > preset->width * preset->height - ((SAFE_ZONE * 2 + 1) * (SAFE_ZONE * 2 + 1) + 1)) {
        LOG(LOG_WARNING, "Too many mines. Game would either be won in 1 click or 1st click would be impossible.");
        preset->mines = preset->width * preset->height - ((SAFE_ZONE * 2 + 1) * (SAFE_ZONE * 2 + 1) + 1);
    }
}

/// Creates and initializes a game, selecting or importing a board from the arguments.
Game *init_game(struct arguments *args) {
    Game *game = malloc(sizeof(Game));
    if (!game) {
        LOG(LOG_ERROR, "Failed to allocate memory for game object");
        return NULL;
    }
    getmaxyx(stdscr, game->max_y, game->max_x);

    game->grid = NULL;
    game->args = args;

    if (args->import_filename) {
        LOG(LOG_DEBUG, "Importing grid");
        game->grid = import_grid(args->import_filename, &game->width, &game->height, &game->mines);
        if (!game->grid) {
            LOG(LOG_ERROR, "Failed to import grid");
            free(game);
            return NULL;
        }
        game->difficulty = CUSTOM_DIFFICULTY;
        game->no_guess = false;
    }
    else if (!args->width && !args->height && !args->mines) {
        LOG(LOG_DEBUG, "Selecting difficulty");
        game->difficulty = select_difficulty(game->max_x, game->max_y, args->difficulty, args->no_guess);
        switch (game->difficulty) {
            case 0:
                load_preset(BEGINNER, &game->width, &game->height, &game->mines);
                game->no_guess = false;
                break;
            case 1:
                load_preset(BEGINNER, &game->width, &game->height, &game->mines);
                game->no_guess = true;
                break;
            case 2:
                load_preset(INTERMEDIATE, &game->width, &game->height, &game->mines);
                game->no_guess = false;
                break;
            case 3:
                load_preset(INTERMEDIATE, &game->width, &game->height, &game->mines);
                game->no_guess = true;
                break;
            case 4:
                load_preset(EXPERT, &game->width, &game->height, &game->mines);
                game->no_guess = false;
                break;
            case 5:
                load_preset(EXPERT, &game->width, &game->height, &game->mines);
                game->no_guess = true;
                break;
            case 6:
                game->no_guess = false;
                Preset custom = select_custom(game->max_x, game->max_y, (Preset) { 0 });
                clamp_custom(&custom, game->max_x, game->max_y);
                load_preset(custom, &game->width, &game->height, &game->mines);
                break;
        }
    }
    else if (!args->width || !args->height || !args->mines) {
        LOG(LOG_DEBUG, "Not all --width, --height and --mines were used. Selecting custom difficulty with those defaults");
        game->difficulty = CUSTOM_DIFFICULTY;
        Preset values = {
            .width = args->width,
            .height = args->height,
            .mines = args->mines,
        };
        Preset custom = select_custom(game->max_x, game->max_y, values);
        clamp_custom(&custom, game->max_x, game->max_y);
        load_preset(custom, &game->width, &game->height, &game->mines);
        game->no_guess = false;
    }
    else {
        LOG(LOG_DEBUG, "Using provided command line options");
        game->difficulty = CUSTOM_DIFFICULTY;
        Preset values = {
            .width = args->width,
            .height = args->height,
            .mines = args->mines,
        };
        clamp_custom(&values, game->max_x, game->max_y);
        load_preset(values, &game->width, &game->height, &game->mines);
        game->no_guess = false;
    }

    int board_height = height_to_board_height(game->height);
    int board_width = width_to_board_width(game->width);
    game->cursor_x = game->width / 2;
    game->cursor_y = game->height / 2;
    game->start_x = 0;
    game->start_y = 0;
    game->flags = 0;
    game->win = false;
    game->time = 0;
    game->clicks = 0;

    game->board = newwin(board_height, board_width, (game->max_y - board_height) / 2, (game->max_x - board_width) / 2);
    keypad(game->board, true);
    wtimeout(game->board, 250);
    LOG(LOG_INFO, "Game initialised.");
    LOG(LOG_DEBUG, "Width: %d, Height: %d, Mines: %d", game->width, game->height, game->mines);
    LOG(LOG_DEBUG, "Difficulty: %s", DIFFICULTIES[game->difficulty + DIFFICULTY_LINES - DIFFICULTY_OPS]);
    LOG(LOG_DEBUG, "No-guess: %s", game->no_guess ? "true" : "false");

    return game;
}

/// Runs the interactive game loop, handling drawing and input until the round ends.
void game_loop(Game *game) {
    clear();
    refresh();
    draw_grid(game->board, NULL, width_to_board_width(game->width), height_to_board_height(game->height), game->difficulty, false);
    draw_stats(game->board, game->mines, 0, 0);
    wrefresh(game->board);

    if (!game->grid) {
        do {
            if (move_cursor(game->board, NULL, game->width, game->height, &game->cursor_x, &game->cursor_y, NULL, game->mines) == CLICK) {
                game->start_x = game->cursor_x;
                game->start_y = game->cursor_y;
                game->grid = create_grid(game->width, game->height, game->mines, game->cursor_x, game->cursor_y, SAFE_ZONE, game->no_guess);
                game->clicks++;
            }
        } while (!game->grid);
    }
    LOG(LOG_INFO, "Grid initialised");
    FOR_EACH_IN_GRID(game->grid, game->width, game->height) {
        if (get_square(game->grid, game->width, game->height, x, y)->mine)
            LOG(LOG_DEBUG, "Mine at (%d, %d)", x, y);
    }

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int selected = 0;
    while (selected != -1) {
        if (all_selected(game->grid, game->width, game->height)) {
            game->win = true;
            LOG(LOG_INFO, "Game won.");
            break;
        }
        do {
            draw_grid(game->board, game->grid, width_to_board_width(game->width), height_to_board_height(game->height), game->difficulty, false);
            draw_stats(game->board, game->mines, game->flags, game->time / 1000);
            wrefresh(game->board);
            switch (move_cursor(game->board, game->grid, game->width, game->height, &game->cursor_x, &game->cursor_y, &game->flags, game->mines)) {
                case CLICK:
                    selected = select_square(&game->grid, game->width, game->height, game->cursor_x, game->cursor_y);
                    game->clicks++;
                    break;
                case FLAG:
                    game->clicks++;
                    break;
                default:
                    break;
            }
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            game->time = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / (1000 * 1000);
        } while (!selected);
    }
}

/// Draws the finished board, handles export and replay, and returns whether to play again.
bool end_game(Game *game) {
    LOG(LOG_INFO, "Game ended");
    draw_grid(game->board, game->grid, width_to_board_width(game->width), height_to_board_height(game->height), game->difficulty, true);
    wrefresh(game->board);
    int play_again;
    if (game->args->export_filename) {
        export_grid(game->grid, game->width, game->height, game->start_x, game->start_y, game->args->export_filename);
        play_again = false;
    }
    else {
        bool first = true;
        do {
            play_again = select_play_again(game->max_x, first && !game->args->import_filename, game);
            if (play_again == 2) {
                LOG(LOG_INFO, "Exporting board");
                char filename[BUFSIZ];
                select_filename(game->max_x, filename, BUFSIZ);
                export_grid(game->grid, game->width, game->height, game->start_x, game->start_y, filename);
                LOG(LOG_DEBUG, "Exported board to %s", filename);
            }
            first = false;
        } while (play_again == 2);
    }
    delwin(game->board);
    free(game->grid);
    clear();
    refresh();
    if (play_again)
        LOG(LOG_INFO, "Playing again.");
    return play_again;
}
