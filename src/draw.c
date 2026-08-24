#include "ms.h"

#include <stddef.h>
#include <string.h>

#define SYMBOLS (const char[9]) { '.', '1', '2', '3', '4', '5', '6', '7', '8' }

#define DIGITS 3

/// Initialises color pairs
void init_color_pairs() {
    start_color();
    use_default_colors();

    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_BLUE, -1);
}

/// Draws grid on board with specified dimensions.
void draw_grid(WINDOW *board, Square *grid, int board_width, int board_height, int difficulty, bool show_all) {
    box(board, 0, 0);
    const char *difficulty_text = DIFFICULTIES[DIFFICULTY_LINES - DIFFICULTY_OPS + difficulty - (difficulty % 2)];
    if (difficulty % 2 == 1) {
        if (strlen("Minesweeper-") + strlen(difficulty_text) + strlen("-No guess") < (size_t) board_width)
            mvwprintw(board, 0, 0, "Minesweeper-%s-No guess", difficulty_text);
        else if (strlen(difficulty_text) + strlen("-No guess") < (size_t) board_width)
            mvwprintw(board, 0, 0, "%s-No guess", difficulty_text);
        else if (strlen(difficulty_text) < (size_t) board_width)
            mvwprintw(board, 0, (board_width - strlen(difficulty_text)) / 2, "%s", difficulty_text);
    }
    else {
        if (strlen("Minesweeper-") + strlen(difficulty_text) < (size_t) board_width)
            mvwprintw(board, 0, 0, "Minesweeper-%s", difficulty_text);
        else if (strlen(difficulty_text) < (size_t) board_width)
            mvwprintw(board, 0, (board_width - strlen(difficulty_text)) / 2, "%s", difficulty_text);
    }

    if (!grid) {
        for (int y = 2; y < board_height - 1; y++) {
            for (int x = 1; x < board_width - 1; x += 2) {
                wattron(board, COLOR_PAIR(3));
                mvwaddch(board, y, x, '#');
                wattroff(board, COLOR_PAIR(3));
            }
        }
    }
    else {
        int i = 0;
        for (int y = 2; y < board_height - 1; y++) {
            for (int x = 1; x < board_width - 1; x += 2) {
                if (show_all) {
                    if (grid[i].mine) {
                        wattron(board, COLOR_PAIR(1));
                        mvwaddch(board, y, x, '*');
                        wattroff(board, COLOR_PAIR(1));
                    }
                    else {
                        wattron(board, COLOR_PAIR(2));
                        mvwaddch(board, y, x, SYMBOLS[grid[i].surrounding]);
                        wattroff(board, COLOR_PAIR(2));
                    }
                }
                else {
                    if (grid[i].uncovered) {
                        wattron(board, COLOR_PAIR(2));
                        mvwaddch(board, y, x, SYMBOLS[grid[i].surrounding]);
                        wattroff(board, COLOR_PAIR(2));
                    }
                    else {
                        if (grid[i].flag) {
                            wattron(board, COLOR_PAIR(1));
                            mvwaddch(board, y, x, 'F');
                            wattroff(board, COLOR_PAIR(1));
                        }
                        else {
                            wattron(board, COLOR_PAIR(3));
                            mvwaddch(board, y, x, '#');
                            wattroff(board, COLOR_PAIR(3));
                        }
                    }
                }
                i++;
            }
        }
    }
}

/// Reads input and moves cursor. Returns true if enter was pressed or false if not
bool move_cursor(WINDOW *win, Square *grid, int width, int height, int *cursor_x, int *cursor_y, int *flag_n, int mine_n) {
    wmove(win, (*cursor_y) + 2, (*cursor_x) * 2 + 1);
    int ch = wgetch(win);
    MEVENT event;
    switch (ch) {
        case KEY_UP:
        case 'k':
            if (*cursor_y > 0)
                (*cursor_y)--;
            break;
        case KEY_DOWN:
        case 'j':
            if (*cursor_y < height - 1)
                (*cursor_y)++;
            break;
        case KEY_LEFT:
        case 'h':
            if (*cursor_x > 0)
                (*cursor_x)--;
            break;
        case KEY_RIGHT:
        case 'l':
            if (*cursor_x < width - 1)
                (*cursor_x)++;
            break;
        case 'f':
            if (grid && flag_n) {
                Square *square = get_square(grid, width, height, *cursor_x, *cursor_y);
                if (!square->uncovered) {
                    if (!square->flag && *flag_n >= mine_n)
                        break;
                    square->flag = !square->flag;
                    if (square->flag)
                        (*flag_n)++;
                    else
                        (*flag_n)--;
                }
            }
            break;
        case KEY_ENTER:
        case '\n':
        case '\r':
            return true;
        case KEY_MOUSE:
            if (getmouse(&event) == OK && wenclose(win, event.y, event.x)) {
                int startx, starty;
                getbegyx(win, starty, startx);
                int maxx, maxy;
                getmaxyx(win, maxy, maxx);
                int x = event.x - startx;
                int y = event.y - starty;
                if (x < BOX_WIDTH / 2 || x >= maxx - BOX_WIDTH / 2 || y < BOX_WIDTH / 2 + STATS_ROW || y >= maxy - BOX_WIDTH / 2)
                    break;
                *cursor_x = board_x_to_x(x);
                *cursor_y = board_y_to_y(y);
                if (event.bstate & BUTTON1_PRESSED)
                    return true;
                else if (event.bstate & BUTTON3_PRESSED) {
                    if (grid && flag_n) {
                        Square *square = get_square(grid, width, height, *cursor_x, *cursor_y);
                        if (!square->uncovered) {
                            if (!square->flag && *flag_n >= mine_n)
                                break;
                            square->flag = !square->flag;
                            if (square->flag)
                                (*flag_n)++;
                            else
                                (*flag_n)--;
                        }
                    }
                }
                break;
            }
        default:
            break;
    }
    wmove(win, y_to_board_y(*cursor_y), x_to_board_x(*cursor_x));
    wrefresh(win);
    return false;
}

void draw_stats(WINDOW *board, int mine_total, int flags, int time) {
    int maxx = getmaxx(board);
    mvwprintw(board, 1, 1, "%03d", mine_total - flags);
    mvwprintw(board, 1, maxx - DIGITS - 1, "%03d", time);
}
