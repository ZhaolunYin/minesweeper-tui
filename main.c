#include <stdlib.h>
#include <stdbool.h>
#include <threads.h>
#include <time.h>
#include <ncurses.h>

#include "draw.h"
#include "grid.h"
#include "ui.h"

int main() {
    srand(time(NULL));
    bool play_again = true;
    initscr();
    start_color();
    use_default_colors();

    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_BLUE, -1);

    mousemask(BUTTON1_PRESSED | BUTTON3_PRESSED, NULL);
    cbreak();
    noecho();
    int width, height, mine_n;
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    while (play_again) {
        switch (select_difficulty(max_x, max_y)) {
            case 0:
                width = 9;
                height = 9;
                mine_n = 10;
                break;
            case 1:
                width = 16;
                height = 16;
                mine_n = 40;
                break;
            case 2:
                width = 30;
                height = 16;
                mine_n = 99;
                break;
        }

        int board_height = height + 3;
        int board_width = (width * 2) + 1;
        int cursor_x = width / 2;
        int cursor_y = height / 2;
        int flag_n = 0;

        bool win = false;

        WINDOW *board = newwin(board_height, board_width, (max_y - board_height) / 2, (max_x - board_width) / 2);
        keypad(board, true);
        wtimeout(board, 100);

        Square *grid = NULL;
        time_t start;
        time(&start);
        draw_grid(board, NULL, board_width, board_height, false);
        draw_stats(board, mine_n, 0, start);
        wrefresh(board);

        do {
            if (move_cursor(board, NULL, width, height, &cursor_x, &cursor_y, NULL))
                grid = create_grid(width, height, mine_n, cursor_x, cursor_y);
        } while (!grid);

        int selected = 0;
        time(&start);

        do {
            wclear(board);
            do {
                draw_grid(board, grid, board_width, board_height, false);
                draw_stats(board, mine_n, flag_n, start);
                wrefresh(board);
                if (move_cursor(board, grid, width, height, &cursor_x, &cursor_y, &flag_n))
                    selected = select_square(&grid, width, height, cursor_x, cursor_y);
            } while (!selected);
            if (all_selected(grid, width, height)) {
                win = true;
            }
        } while (selected != -1 && !win);
        draw_grid(board, grid, board_width, board_height, true);
        wrefresh(board);
        play_again = select_play_again(max_x, max_y, win);
        int ch;
        delwin(board);
        free(grid);
        clear();
        refresh();
    }
    endwin();
}
