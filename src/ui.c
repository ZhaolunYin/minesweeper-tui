#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

static int _select_menu(int x, int y, int width, int height, int n_lines, int n_options, const char **lines) {
    WINDOW *window = newwin(height, width, y, x);
    keypad(window, true);
    int cv = curs_set(0);
    int ch = 0;
    int opt = 0;
    while (ch != KEY_ENTER && ch != '\n' && ch != '\r') {
        box(window, 0, 0);
        for (int i = 0; i < n_lines; i++) {
            if (i == opt + n_lines - n_options)
                wattron(window, A_REVERSE);
            mvwprintw(window, i + 1, 1, "%s", lines[i]);
            wattroff(window, A_REVERSE);
        }
        wrefresh(window);
        ch = wgetch(window);
        MEVENT event;
        switch (ch) {
            case KEY_UP:
            case 'k':
                if (opt > 0)
                    opt--;
                break;
            case KEY_DOWN:
            case 'j':
                if (opt < n_options - 1)
                    opt++;
                break;

            case KEY_MOUSE:
                if (getmouse(&event) == OK && wenclose(window, event.y, event.x)) {
                    int relative_x = event.x - x;
                    int relative_y = event.y - y;
                    if (event.bstate & BUTTON1_PRESSED) {
                        if (relative_x > 0 && relative_x < width - 1 && relative_y > 0 && relative_y < height - 1) {
                            opt = (relative_y - 1) - n_options + n_lines;
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

int select_difficulty(int term_width, int term_height) {
    int width = 0;
    for (int i = 0; i < DIFFICULTY_LINES; i++) {
        if ((int) strlen(DIFFICULTIES[i]) > width)
            width = strlen(DIFFICULTIES[i]);
    }
    width += 2;
    return _select_menu(
            (term_width - width) / 2,
            (term_height - (DIFFICULTY_LINES + 2)) / 2,
            width,
            DIFFICULTY_LINES + 2,
            DIFFICULTY_LINES,
            DIFFICULTY_OPS,
            DIFFICULTIES);
}

bool select_play_again(int term_width, bool win, long long score, long long highscore) {
    char *ops[PLAY_AGAIN_OPS] = {
        "You lose!",
        "You win!",
    };
    char score_text[BUFSIZ];
    char highscore_text[BUFSIZ];
    char *lines[PLAY_AGAIN_LINES] = {
        ops[(int) win],
        score_text,
        highscore_text,
        "Play again?",
        "Yes",
        "No",
    };
    if (win)
        snprintf(score_text, sizeof(score_text), "Score: %lld.%3.llds", score / 1000, score % 1000);
    else
        snprintf(score_text, sizeof(score_text), "Score: None");
    if (highscore)
        snprintf(highscore_text, sizeof(highscore_text), "Highscore: %lld.%3.llds", highscore / 1000, highscore % 1000);
    else
        snprintf(highscore_text, sizeof(highscore_text), "Highscore: None");
    int width = 0;
    for (int i = 0; i < PLAY_AGAIN_LINES; i++) {
        if ((int) strlen(lines[i]) > width)
            width = strlen(lines[i]);
    }
    width += 2;
    return (bool) !_select_menu(
            (term_width - width) / 2,
            1,
            width,
            PLAY_AGAIN_LINES + 2,
            PLAY_AGAIN_LINES,
            PLAY_AGAIN_OPS,
            (const char **) lines);
}

void select_custom(int *result, int term_width, int term_height) {
    const char *lines[CUSTOM_LINES] = {
        "Width: ",
        "Height:",
        "Mines: ",
    };
    int values[CUSTOM_LINES] = { 30, 30, 160 };
    int width = 0;
    for (int i = 0; i < CUSTOM_LINES; i++) {
        if ((int) strlen(lines[i]) > width)
            width = strlen(lines[i]) + CUSTOM_MAX_DIGITS;
    }
    width += 2;
    WINDOW *window = newwin(CUSTOM_LINES + 2, width, (term_height - (CUSTOM_LINES + 2)) / 2, (term_width - width) / 2);
    keypad(window, true);
    int cv = curs_set(0);
    int opt = 0;
    for (int i = 0; i < CUSTOM_LINES; i++) {
        int ch = 0;
        box(window, 0, 0);
        while (ch != KEY_ENTER && ch != '\n' && ch != '\r') {
            for (int j = 0; j < CUSTOM_LINES; j++) {
                if (j == opt)
                    wattron(window, A_REVERSE);
                mvwprintw(window, j + 1, 1, "%s%03d", lines[j], values[j]);
                wattroff(window, A_REVERSE);
            }
            wrefresh(window);
            ch = wgetch(window);
            if (ch >= '0' && ch <= '9') {
                if (values[i] > CUSTOM_MAX_VALUE / 10)
                    continue;
                values[i] *= 10;
                values[i] += ch - '0';
            }
            else if (ch == KEY_BACKSPACE) {
                values[i] /= 10;
            }
        }
        result[i] = values[i];
        opt++;
    }
    delwin(window);
    curs_set(cv);
    refresh();
}
