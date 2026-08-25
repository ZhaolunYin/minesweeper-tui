#include "ms/ui.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#include "ms.h"

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
                            opt = (relative_y - 1) - n_lines + n_options;
                            ch = KEY_ENTER;
                        }
                    }
                }
                break;
        }
    }
    wclear(window);
    wrefresh(window);
    delwin(window);
    curs_set(cv);
    refresh();
    return opt;
}

int select_difficulty(int term_width, int term_height, char *difficulty, bool no_guess) {
    if (difficulty) {
        for (char *p = difficulty; *p; p++) {
            *p = tolower(*p);
        }
        for (int i = 0; i < DIFFICULTY_OPS - 1; i++) {
            char level[BUFSIZ];
            snprintf(level, BUFSIZ, "%s", DIFFICULTIES[i + DIFFICULTY_LINES - DIFFICULTY_OPS]);
            for (char *p = level; *p; p++) {
                *p = tolower(*p);
            }
            if (strcmp(difficulty, level) == 0 && i != CUSTOM_DIFFICULTY + DIFFICULTY_OPS - DIFFICULTY_LINES)
                return (i + no_guess);
        }
    }

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

static void _show_stats(Game *game) {
    char score_text[BUFSIZ];
    char highscore_text[BUFSIZ];
    char text_3bv[BUFSIZ];
    char sec_3bv_text[BUFSIZ];
    char clicks_text[BUFSIZ];
    char efficiency_text[BUFSIZ];

    char *text[STATS_N] = {
        score_text,
        highscore_text,
        text_3bv,
        sec_3bv_text,
        clicks_text,
        efficiency_text,
    };

    // Score
    if (game->score)
        snprintf(score_text, sizeof(score_text), "Score: %lld.%3.llds", game->score / 1000, game->score % 1000);
    else
        snprintf(score_text, sizeof(score_text), "Score: None");

    // Highscore
    if (game->highscore)
        snprintf(highscore_text, sizeof(highscore_text), "Highscore: %lld.%3.llds", game->highscore / 1000, game->highscore % 1000);
    else
        snprintf(highscore_text, sizeof(highscore_text), "Highscore: None");

    // 3BV
    int _3bv = get_3bv(game->grid, game->width, game->height);
    snprintf(text_3bv, sizeof(text_3bv), "3BV: %d", _3bv);

    // 3BV/s
    if (game->score)
        snprintf(sec_3bv_text, sizeof(sec_3bv_text), "3BV/s: %g", (double) _3bv / ((double) game->score / 1000));

    // Clicks
    snprintf(clicks_text, sizeof(clicks_text), "Clicks: %d", game->clicks);

    // Efficiency
    if (game->clicks) {
        double efficiency = ((double) _3bv / game->clicks) * 100;
        snprintf(efficiency_text, sizeof(efficiency_text), "Efficiency: %g%%", efficiency);
    }


    int width = 0;
    for (int i = 0; i < STATS_N; i++) {
        if ((int) strlen(text[i]) > width)
            width = strlen(text[i]);
    }
    width += 2;
    WINDOW *window = newwin(getmaxy(stdscr), width, 0, 0);
    box(window, 0, 0);
    for (int i = 0; i < STATS_N; i++) {
        mvwprintw(window, i + BOX_WIDTH / 2, BOX_WIDTH / 2, "%s", text[i]);
    }
    wrefresh(window);
    delwin(window);
}

int select_play_again(int term_width, bool show_export, Game *game) {
    char *ops[PLAY_AGAIN_OPS] = {
        "You lose!",
        "You win!",
    };
    char *lines[PLAY_AGAIN_LINES] = {
        ops[(int) game->win],
        "Play again?",
        "Yes",
        "No",
        "Export board",
    };
    if (game->win)
        _show_stats(game);
    int width = 0;
    for (int i = 0; i < PLAY_AGAIN_LINES; i++) {
        if ((int) strlen(lines[i]) > width)
            width = strlen(lines[i]);
    }
    width += 2;
    int nlines = PLAY_AGAIN_LINES - !show_export;
    int nops = PLAY_AGAIN_OPS - !show_export;
    int result = _select_menu(
            (term_width - width) / 2,
            0,
            width,
            nlines + 2,
            nlines,
            nops,
            (const char **) lines);
    if (result == 0 || result == 1)
        result = !result;
    return result;
}

Preset select_custom(int term_width, int term_height, Preset config) {
    const char *lines[CUSTOM_LINES] = {
        "Width: ",
        "Height:",
        "Mines: ",
    };
    int *values[CUSTOM_LINES];
    if (!config.width)
        config.width = CUSTOM_DEFAULTS.width;

    if (!config.height)
        config.height = CUSTOM_DEFAULTS.height;

    if (!config.mines)
        config.mines = CUSTOM_DEFAULTS.mines;
    values[0] = &config.width;
    values[1] = &config.height;
    values[2] = &config.mines;

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
                mvwprintw(window, j + 1, 1, "%s%03d", lines[j], *values[j]);
                wattroff(window, A_REVERSE);
            }
            wrefresh(window);
            ch = wgetch(window);
            if (ch >= '0' && ch <= '9') {
                if (*values[i] > CUSTOM_MAX_VALUE / 10)
                    continue;
                *values[i] *= 10;
                *values[i] += ch - '0';
            }
            else if (ch == KEY_BACKSPACE) {
                *values[i] /= 10;
            }
        }
        opt++;
    }
    wclear(window);
    wrefresh(window);
    delwin(window);
    curs_set(cv);
    refresh();
    return config;
}

void select_filename(int term_width, char *filename, size_t len) {
    if (!filename)
        return;
    WINDOW *window = newwin(FILENAME_LINES + 2, term_width, 0, 0);
    keypad(window, true);
    box(window, 0, 0);
    int ch = 0;
    size_t index = 0;
    for (char *p = filename; p < filename + len; p++) {
        *p = '\0';
    }
    while (ch != KEY_ENTER && ch != '\n' && ch != '\r') {
        wclear(window);
        mvwprintw(window, BOX_WIDTH / 2, BOX_WIDTH / 2, "Select File");
        mvwprintw(window, BOX_WIDTH / 2 + 1, BOX_WIDTH / 2, "%s", filename);
        box(window, 0, 0);
        ch = wgetch(window);
        if (ch == KEY_BACKSPACE) {
            if (index > 0)
                filename[--index] = '\0';
        }
        else if (ch == 27) {
            for (char *p = filename; p < filename + len; p++) {
                *p = '\0';
            }
            index = 0;
            break;
        }
        else if (ch >= ' ' && ch <= '~'){
            if (index < (size_t) term_width - 2 && index < len - 1)
                filename[index++] = ch;
        }
    }
    wclear(window);
    wrefresh(window);
    delwin(window);
}
