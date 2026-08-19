#include "draw.h"

const char symbols[9] = { '.', '1', '2', '3', '4', '5', '6', '7', '8' };

/// Draws grid on board with specified dimensions.
void draw_grid(WINDOW *board, Square *grid, int board_width, int board_height, bool show_all) {
    box(board, 0, 0);
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
                        mvwaddch(board, y, x, symbols[grid[i].surrounding]);
                        wattroff(board, COLOR_PAIR(2));
                    }
                }
                else {
                    if (grid[i].uncovered) {
                        wattron(board, COLOR_PAIR(2));
                        mvwaddch(board, y, x, symbols[grid[i].surrounding]);
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
bool move_cursor(WINDOW *win, Square *grid, int width, int height, int *cursor_x, int *cursor_y, int *flag_n) {
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
                Square *square = &grid[(*cursor_y) * width + (*cursor_x)];
                if (!square->uncovered) {
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
                if (x < 1 || x >= maxx - 1 || y < 1 || y >= maxy - 1)
                    break;
                *cursor_x = (x - 1) / 2;
                *cursor_y = y - 2;
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
                        }

                    }
                }
                break;
            }
        default:
            break;
    }
    wmove(win, (*cursor_y) + 1, (*cursor_x) * 2 + 1);
    wrefresh(win);
    return false;
}

void draw_stats(WINDOW *board, int mine_total, int flags, time_t before) {
    const int DIGITS = 3;
    int maxx = getmaxx(board);
    mvwprintw(board, 1, 1, "%03d", mine_total - flags);
    time_t now;
    time(&now);
    mvwprintw(board, 1, maxx - DIGITS - 1, "%03ld", now - before);
}
