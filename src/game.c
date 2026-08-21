#include "game.h"
#include "draw.h"
#include "board.h"
#include "grid.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define SAFE_ZONE 1
#define MIN_MINES 1

#define SCORING_DIFFICULTIES 6

void _fix_highscore_file() {
    char dir[BUFSIZ];
    char file[BUFSIZ];
    char *data_dir = getenv("XDG_DATA_HOME");
    if (!data_dir) {
        char *home = getenv("HOME");
        if (!home)
            return;
        snprintf(dir, BUFSIZ, "%s/.local/share/%s", home, FOLDER_NAME);
    }
    else {
        snprintf(dir, BUFSIZ, "%s/%s", data_dir, FOLDER_NAME);
    }
    mkdir(dir, 0755);
    snprintf(file, BUFSIZ, "%s/%s/%s", data_dir, FOLDER_NAME, SCORE_FILE);

    FILE *fptr = fopen(file, "w");
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        fprintf(fptr, "0\n");
    }
    fclose(fptr);
}

long _load_highscore(int difficulty) {
    if (difficulty < 0)
        difficulty = 0;
    if (difficulty > SCORING_DIFFICULTIES - 1)
        difficulty = SCORING_DIFFICULTIES - 1;
    char dir[BUFSIZ];
    char file[BUFSIZ];
    char *data_dir = getenv("XDG_DATA_HOME");
    if (!data_dir) {
        char *home = getenv("HOME");
        if (!home)
            return 0;
        snprintf(dir, BUFSIZ, "%s/.local/share/%s", home, FOLDER_NAME);
    }
    else {
        snprintf(dir, BUFSIZ, "%s/%s", data_dir, FOLDER_NAME);
    }
    mkdir(dir, 0755);
    snprintf(file, BUFSIZ, "%s/%s/%s", data_dir, FOLDER_NAME, SCORE_FILE);
    FILE *fptr = fopen(file, "r");
    if (!fptr) {
        _fix_highscore_file();
        return 0;
    }
    long long result[SCORING_DIFFICULTIES];
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        if (fscanf(fptr, "%lld", &result[i]) != 1) {
            fclose(fptr);
            _fix_highscore_file();
            return 0;
        }
    }
    fclose(fptr);
    return result[difficulty];
}

void _write_highscore(long score, int difficulty) {
    long long scores[SCORING_DIFFICULTIES];
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        scores[i] = _load_highscore(i);
    }
    scores[difficulty] = score;
    if (difficulty < 0)
        difficulty = 0;
    if (difficulty > SCORING_DIFFICULTIES - 1)
        difficulty = SCORING_DIFFICULTIES - 1;
    char dir[BUFSIZ];
    char file[BUFSIZ];
    char *data_dir = getenv("XDG_DATA_HOME");
    if (!data_dir) {
        char *home = getenv("HOME");
        if (!home)
            return;
        snprintf(dir, BUFSIZ, "%s/.local/share/%s", home, FOLDER_NAME);
    }
    else {
        snprintf(dir, BUFSIZ, "%s/%s", data_dir, FOLDER_NAME);
    }
    mkdir(dir, 0755);
    snprintf(file, BUFSIZ, "%s/%s/%s", data_dir, FOLDER_NAME, SCORE_FILE);

    FILE *fptr = fopen(file, "w");
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        fprintf(fptr, "%lld\n", scores[i]);
    }
    fclose(fptr);
}

Game init_game() {
    Game game;
    getmaxyx(stdscr, game.max_y, game.max_x);

    int result[3];
    game.difficulty = select_difficulty(game.max_x, game.max_y);
    switch (game.difficulty) {
        case 0:
            game.width = 9;
            game.height = 9;
            game.mines = 10;
            game.no_guess = false;
            break;
        case 1:
            game.width = 9;
            game.height = 9;
            game.mines = 10;
            game.no_guess = true;
            break;
        case 2:
            game.width = 16;
            game.height = 16;
            game.mines = 40;
            game.no_guess = false;
            break;
        case 3:
            game.width = 16;
            game.height = 16;
            game.mines = 40;
            game.no_guess = true;
            break;
        case 4:
            game.width = 30;
            game.height = 16;
            game.mines = 99;
            game.no_guess = false;
            break;
        case 5:
            game.width = 30;
            game.height = 16;
            game.mines = 99;
            game.no_guess = true;
            break;
        case 6:
            select_custom(result, game.max_x, game.max_y);
            // If win on one click - 
            // Safezone * 2 + 1 is minimum width / height first click uncovers
            if (result[0] < SAFE_ZONE * 2 + 2)
                result[0] = SAFE_ZONE * 2 + 2;
            if (result[1] < SAFE_ZONE * 2 + 2)
                result[1] = SAFE_ZONE * 2 + 2;

            // If bigger than screen
            if (result[0] > board_width_to_width(game.max_x) - 1)
                result[0] = board_width_to_width(game.max_x) - 1;
            if (result[1] > board_height_to_height(game.max_y) - 1)
                result[1] = board_height_to_height(game.max_y) - 1;

            // If too little mines
            if (result[2] < MIN_MINES)
                result[2] = MIN_MINES;

            // If no of mines causes 1st click win or impossible 1st click
            if (result[2] > result[0] * result[1] - (SAFE_ZONE * 2 + 1) * (SAFE_ZONE * 2 + 1) + 1)
                result[2] = result[0] * result[1] - (SAFE_ZONE * 2 + 1) * (SAFE_ZONE * 2 + 1) + 1;

            // Assign
            game.width = result[0];
            game.height = result[1];
            game.mines = result[2];
            game.no_guess = false;
            break;
    }

    int board_height = height_to_board_height(game.height);
    int board_width = width_to_board_width(game.width);
    game.cursor_x = game.width / 2;
    game.cursor_y = game.height / 2;
    game.flags = 0;
    game.win = false;
    game.grid = NULL;
    game.score = 0;
    game.highscore = _load_highscore(game.difficulty);

    game.board = newwin(board_height, board_width, (game.max_y - board_height) / 2, (game.max_x - board_width) / 2);
    keypad(game.board, true);
    wtimeout(game.board, 250);

    return game;
}

void game_loop(Game *game) {
    clear();
    refresh();
    draw_grid(game->board, NULL, width_to_board_width(game->width), height_to_board_height(game->height), game->difficulty, false);
    draw_stats(game->board, game->mines, 0, 0);
    wrefresh(game->board);

    do {
        if (move_cursor(game->board, NULL, game->width, game->height, &game->cursor_x, &game->cursor_y, NULL, game->mines))
            game->grid = create_grid(game->width, game->height, game->mines, game->cursor_x, game->cursor_y, SAFE_ZONE, game->no_guess);
    } while (!game->grid);

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
    if ((!game->highscore || game->score < game->highscore) && game->win) {
        game->highscore = game->score;
        _write_highscore(game->highscore, game->difficulty);
    }
    bool play_again = select_play_again(game->max_x, game->win, game->score, game->highscore);
    delwin(game->board);
    free(game->grid);
    clear();
    refresh();
    return play_again;
}
