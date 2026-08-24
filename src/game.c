#include <stdlib.h>
#include <sys/stat.h>

#include "ms.h"

#define SAFE_ZONE 1
#define MIN_MINES 1

static void _clamp_custom(Preset *preset, int max_x, int max_y) {
    // If win on one click - 
    // Safezone * 2 + 1 is minimum width / height first click uncovers
    if (preset->width < SAFE_ZONE * 2 + 2)
        preset->width = SAFE_ZONE * 2 + 2;
    if (preset->height < SAFE_ZONE * 2 + 2)
        preset->height = SAFE_ZONE * 2 + 2;

    // If bigger than screen
    if (preset->width > board_width_to_width(max_x) - 1)
        preset->width = board_width_to_width(max_x) - 1;
    if (preset->height > board_height_to_height(max_y) - 1)
        preset->height = board_height_to_height(max_y) - 1;

    // If too little mines
    if (preset->mines < MIN_MINES)
        preset->mines = MIN_MINES;

    // If no of mines causes 1st click win or impossible 1st click
    if (preset->mines > preset->width * preset->height - ((SAFE_ZONE * 2 + 1) * (SAFE_ZONE * 2 + 1) + 1))
        preset->mines = preset->height * preset->height - ((SAFE_ZONE * 2 + 1) * (SAFE_ZONE * 2 + 1) + 1);
}

Game *init_game(struct arguments *args) {
    Game *game = malloc(sizeof(Game));
    getmaxyx(stdscr, game->max_y, game->max_x);

    game->grid = NULL;
    game->args = args;

    if (args->filename) {
        game->grid = import_grid(args->filename, &game->width, &game->height, &game->mines);
        if (!game->grid) {
            free(game);
            return NULL;
        }
        game->difficulty = CUSTOM_DIFFICULTY;
        game->no_guess = false;
    }
    else if (!args->width && !args->height && !args->mines) {
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
                Preset custom = select_custom(game->max_x, game->max_y, (Preset) {0});
                _clamp_custom(&custom, game->max_x, game->max_y);
                load_preset(custom, &game->width, &game->height, &game->mines);
                break;
        }
    }
    else if (!args->width || !args->height || !args->mines) {
        game->difficulty = CUSTOM_DIFFICULTY;
        Preset values = {
            .width = args->width,
            .height = args->height,
            .mines = args->mines,
        };
        Preset custom = select_custom(game->max_x, game->max_x, values);
        _clamp_custom(&custom, game->max_x, game->max_y);
        load_preset(custom, &game->width, &game->height, &game->mines);
        game->no_guess = false;
    }
    else {
        game->difficulty = CUSTOM_DIFFICULTY;
        Preset values = {
            .width = args->width,
            .height = args->height,
            .mines = args->mines,
        };
        _clamp_custom(&values, game->max_x, game->max_y);
        load_preset(values, &game->width, &game->height, &game->mines);
        game->no_guess = false;
    }

    int board_height = height_to_board_height(game->height);
    int board_width = width_to_board_width(game->width);
    game->cursor_x = game->width / 2;
    game->cursor_y = game->height / 2;
    game->flags = 0;
    game->win = false;
    game->score = 0;
    game->highscore = load_highscore(game->difficulty);

    game->board = newwin(board_height, board_width, (game->max_y - board_height) / 2, (game->max_x - board_width) / 2);
    keypad(game->board, true);
    wtimeout(game->board, 250);

    return game;
}

void game_loop(Game *game) {
    clear();
    refresh();
    draw_grid(game->board, NULL, width_to_board_width(game->width), height_to_board_height(game->height), game->difficulty, false);
    draw_stats(game->board, game->mines, 0, 0);
    wrefresh(game->board);

    if (!game->grid) {
        do {
            if (move_cursor(game->board, NULL, game->width, game->height, &game->cursor_x, &game->cursor_y, NULL, game->mines))
                game->grid = create_grid(game->width, game->height, game->mines, game->cursor_x, game->cursor_y, SAFE_ZONE, game->no_guess);
        } while (!game->grid);
    }

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int selected = 0;
    while (selected != -1) {
        if (all_selected(game->grid, game->width, game->height)) {
            game->win = true;
            break;
        }
        do {
            draw_grid(game->board, game->grid, width_to_board_width(game->width), height_to_board_height(game->height), game->difficulty, false);
            draw_stats(game->board, game->mines, game->flags, game->score / 1000);
            wrefresh(game->board);
            if (move_cursor(game->board, game->grid, game->width, game->height, &game->cursor_x, &game->cursor_y, &game->flags, game->mines)) {
                selected = select_square(&game->grid, game->width, game->height, game->cursor_x, game->cursor_y);
            }
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            game->score = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / (1000 * 1000);
        } while (!selected);
    }
}

bool end_game(Game *game) {
    draw_grid(game->board, game->grid, width_to_board_width(game->width), height_to_board_height(game->height), game->difficulty, true);
    wrefresh(game->board);
    if ((!game->highscore || game->score < game->highscore) && game->win && 
            !(game->difficulty < 0 || game->difficulty > SCORING_DIFFICULTIES - 1) &&
            game->args->save_highscore) {
        game->highscore = game->score;
        write_highscore(game->highscore, game->difficulty);
    }
    bool play_again = select_play_again(game->max_x, game->win, game->score, game->highscore);
    delwin(game->board);
    free(game->grid);
    clear();
    refresh();
    return play_again;
}
