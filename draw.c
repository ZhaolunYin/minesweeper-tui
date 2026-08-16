#include "draw.h"
#include <time.h>

const char symbols[9] = { '.', '1', '2', '3', '4', '5', '6', '7', '8' };

/// Draws grid on board with specified dimensions.
void draw_grid(WINDOW *board, Square *grid, int board_width, int board_height, bool show_all) {
    box(board, 0, 0);
    if (!grid) {
        for (int y = 1; y < board_height - 1; y++) {
            for (int x = 1; x < board_width - 1; x += 2) {
                mvwaddch(board, y, x, '#');
            }
        }
    }
    else {
        int i = 0;
        for (int y = 1; y < board_height - 1; y++) {
            for (int x = 1; x < board_width - 1; x += 2) {
                if (show_all) {
                    if (grid[i].mine) {
                        wattron(board, COLOR_PAIR(1));
                        mvwprintw(board, y, x, "*");
                        wattroff(board, COLOR_PAIR(1));
                    }
                    else
                        mvwaddch(board, y, x, symbols[grid[i].surrounding]);
                }
                else {
                    if (grid[i].uncovered)
                        mvwaddch(board, y, x, symbols[grid[i].surrounding]);
                    else {
                        if (grid[i].flag) {
                            wattron(board, COLOR_PAIR(1));
                            mvwprintw(board, y, x, "F");
                            wattroff(board, COLOR_PAIR(1));
                        }
                        else
                            mvwaddch(board, y, x, '#');
                    }
                }
                i++;
            }
        }
    }
}

/// Reads input and moves cursor. Returns true if enter was pressed or false if not
bool move_cursor(WINDOW *win, Square *grid, int width, int height, int *cursor_x, int *cursor_y, int *flag_n) {
    wmove(win, (*cursor_y) + 1, (*cursor_x) * 2 + 1);
    for (;;) {
        int ch = wgetch(win);
        MEVENT event;
        switch (ch) {
            case KEY_UP:
                if (*cursor_y > 0)
                    (*cursor_y)--;
                break;
            case KEY_DOWN:
                if (*cursor_y < height - 1)
                    (*cursor_y)++;
                break;
            case KEY_LEFT:
                if (*cursor_x > 0)
                    (*cursor_x)--;
                break;
            case KEY_RIGHT:
                if (*cursor_x < width - 1)
                    (*cursor_x)++;
                break;
            case 'f':
                if (grid && flag_n) {
                    Square *square = &grid[(*cursor_y) * width + (*cursor_x)];
                    if (!square->uncovered) {
                        square->flag = !square->flag;
                        if (square->flag)
                            (*flag_n)++;
                        else
                            (*flag_n)--;
                        return false;
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
                    if (x < 1 || x >= maxx - 1 || y < 1 || y >= maxy - 1)
                        break;
                    *cursor_x = (x - 1) / 2;
                    *cursor_y = y - 1;
                    if (event.bstate & BUTTON1_PRESSED)
                        return true;
                    else if (event.bstate & BUTTON3_PRESSED) {
                        if (grid && flag_n) {
                            Square *square = &grid[(*cursor_y) * width + (*cursor_x)];
                            if (!square->uncovered) {
                                square->flag = !square->flag;
                                if (square->flag)
                                    (*flag_n)++;
                                else
                                    (*flag_n)--;
                                return false;
                            }

                        }
                    }
                    break;
                }
        }
        wmove(win, (*cursor_y) + 1, (*cursor_x) * 2 + 1);
        wrefresh(win);
    }
}

void draw_stats(int mine_total, int flags, time_t before) {
    const int DIGITS = 3;
    int maxx = getmaxx(stdscr);
    mvprintw(0, 0, "%3d", mine_total - flags);
    time_t now;
    time(&now);
    mvprintw(0, maxx - DIGITS, "%3ld", now - before);
}
