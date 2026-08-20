#include "game.h"
#include "draw.h"
#include "board.h"
#include "ui.h"
#include <stdlib.h>

Game init_game() {
    Game game;
    getmaxyx(stdscr, game.max_y, game.max_x);

    int result[3];
    switch (select_difficulty(game.max_x, game.max_y)) {
        case 0:
            game.width = 9;
            game.height = 9;
            game.mines = 10;
            break;
        case 1:
            game.width = 16;
            game.height = 16;
            game.mines = 40;
            break;
        case 2:
            game.width = 30;
            game.height = 16;
            game.mines = 99;
            break;
        case 3:
            select_custom(result, game.max_x, game.max_y);
            // Check width
            if (result[0] <= 3)
                result[0] = 4;
            if (result[0] >= (game.max_x - 1) / 2)
                result[0] = (game.max_x - 1) / 2 - 1;
            // Check height
            if (result[1] <= 3)
                result[1] = 4;
            if (result[1] >= game.max_y - 3)
                result[1] = game.max_y - 4;
            // Check mines
            if (!result[2])
                result[2] = 1;
            if (result[2] >= result[0] * result[1] - 9)
                result[2] = result[0] * result[1] - 10;
            // Assign
            game.width = result[0];
            game.height = result[1];
            game.mines = result[2];
            break;
    }

    int board_height = height_to_board_height(game.height);
    int board_width = width_to_board_width(game.width);
    game.cursor_x = game.width / 2;
    game.cursor_y = game.height / 2;
    game.flags = 0;
    game.win = false;
    game.grid = NULL;

    game.board = newwin(board_height, board_width, (game.max_y - board_height) / 2, (game.max_x - board_width) / 2);
    keypad(game.board, true);
    wtimeout(game.board, 100);

    return game;
}

void game_loop(Game *game) {
    time_t start;
    time(&start);
    clear();
    refresh();
    draw_grid(game->board, NULL, width_to_board_width(game->width), height_to_board_height(game->height), false);
    draw_stats(game->board, game->mines, 0, start);
    wrefresh(game->board);

    do {
        if (move_cursor(game->board, NULL, game->width, game->height, &game->cursor_x, &game->cursor_y, NULL))
            game->grid = create_grid(game->width, game->height, game->mines, game->cursor_x, game->cursor_y);
    } while (!game->grid);


    int selected = 0;
    time(&start);

    do {
        wclear(game->board);
        do {
            draw_grid(game->board, game->grid, width_to_board_width(game->width), height_to_board_height(game->height), false);
            draw_stats(game->board, game->mines, game->flags, start);
            wrefresh(game->board);
            if (move_cursor(game->board, game->grid, game->width, game->height, &game->cursor_x, &game->cursor_y, &game->flags))
                selected = select_square(&game->grid, game->width, game->height, game->cursor_x, game->cursor_y);
        } while (!selected);
        if (all_selected(game->grid, game->width, game->height)) {
            game->win = true;
        }
    } while (selected != -1 && !game->win);
}

bool end_game(Game *game) {
    draw_grid(game->board, game->grid, width_to_board_width(game->width), height_to_board_height(game->height), true);
    wrefresh(game->board);
    bool play_again = select_play_again(game->max_x, game->max_y, game->win);
    int ch;
    delwin(game->board);
    free(game->grid);
    clear();
    refresh();
    return play_again;
}
