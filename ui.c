#include "ui.h"
#include <string.h>
#include <ncurses.h>

#define DIFFICULTY_LINES 4
#define DIFFICULTY_OPS 3
#define PLAY_AGAIN_LINES 4
#define PLAY_AGAIN_OPS 2

int select_difficulty(int term_width, int term_height) {
    const char *lines[DIFFICULTY_LINES] = {
        "Select difficulty",
        "1. Beginner",
        "2. Intermediate",
        "3. Expert",
    };
    int width = 0;
    for (int i = 0; i < DIFFICULTY_LINES; i++) {
        if (strlen(lines[i]) > width)
            width = strlen(lines[i]);
    }
    width += 2;
    WINDOW *window = newwin(DIFFICULTY_LINES + 2, width, (term_height - (DIFFICULTY_LINES + 2)) / 2, (term_width - width) / 2);
    keypad(window, true);
    int cv = curs_set(0);
    int ch = 0;
    int opt = 0;
    while (ch != KEY_ENTER && ch != '\n' && ch != '\r') {
        box(window, 0, 0);
        for (int i = 0; i < DIFFICULTY_LINES; i++) {
            if (i == opt + DIFFICULTY_LINES - DIFFICULTY_OPS)
                wattron(window, A_REVERSE);
            mvwprintw(window, i + 1, 1, "%s", lines[i]);
            wattroff(window, A_REVERSE);
        }
        wrefresh(window);
        ch = wgetch(window);
        MEVENT event;
        switch (ch) {
            case KEY_UP:
                if (opt > 0)
                    opt--;
                break;
            case KEY_DOWN:
                if (opt < DIFFICULTY_OPS - 1)
                    opt++;
                break;

            case KEY_MOUSE:
                if (getmouse(&event) == OK && wenclose(window, event.y, event.x)) {
                    int relative_x = event.x - ((term_width - width) / 2);
                    int relative_y = event.y - ((term_height - (DIFFICULTY_LINES + 2)) / 2);
                    if (event.bstate & BUTTON1_PRESSED) {
                        if (relative_x > 0 && relative_x < width - 1 && relative_y > 0 && relative_y < DIFFICULTY_LINES + 1) {
                            opt = (relative_y - 1) - DIFFICULTY_LINES + DIFFICULTY_OPS;
                            ch = KEY_ENTER;
                        }
                    }
                }
                break;
        }
    }
    delwin(window);
    curs_set(cv);
    refresh();
    return opt;
}


bool select_play_again(int term_width, int term_height, bool win) {
    char *ops[PLAY_AGAIN_OPS] = {
        "You lose!",
        "You win!",
    };
    char *lines[PLAY_AGAIN_LINES] = {
        ops[(int) win],
        "Play again?",
        "Yes",
        "No",
    };
    int width = 0;
    for (int i = 0; i < PLAY_AGAIN_LINES; i++) {
        if (strlen(lines[i]) > width)
            width = strlen(lines[i]);
    }
    width += 2;
    WINDOW *window = newwin(PLAY_AGAIN_LINES + 2, width, 1, (term_width - width) / 2);
    keypad(window, true);
    int cv = curs_set(0);
    int ch = 0;
    int opt = 0;
    while (ch != KEY_ENTER && ch != '\n' && ch != '\r') {
        box(window, 0, 0);
        for (int i = 0; i < PLAY_AGAIN_LINES; i++) {
            if (i == opt + PLAY_AGAIN_LINES - PLAY_AGAIN_OPS)
                wattron(window, A_REVERSE);
            mvwprintw(window, i + 1, 1, "%s", lines[i]);
            wattroff(window, A_REVERSE);
        }
        wrefresh(window);
        ch = wgetch(window);
        MEVENT event;
        switch (ch) {
            case KEY_UP:
                if (opt > 0)
                    opt--;
                break;
            case KEY_DOWN:
                if (opt < PLAY_AGAIN_OPS - 1)
                    opt++;
                break;

            case KEY_MOUSE:
                if (getmouse(&event) == OK && wenclose(window, event.y, event.x)) {
                    int relative_x = event.x - ((term_width - width) / 2);
                    int relative_y = event.y - 1;
                    if (event.bstate & BUTTON1_PRESSED) {
                        if (relative_x > 0 && relative_x < width - 1 && relative_y > 0 && relative_y < PLAY_AGAIN_LINES + 1) {
                            opt = (relative_y - 1) - (PLAY_AGAIN_LINES - PLAY_AGAIN_OPS);
                            ch = KEY_ENTER;
                        }
                    }
                }
                break;
        }
    }
    delwin(window);
    curs_set(cv);
    refresh();
    return (bool) !opt;
}
