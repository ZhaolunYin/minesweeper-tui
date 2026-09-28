#include "ms/ui.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

#include "ms.h"
#include "ms/board.h"
#include "ms/config.h"
#include "ms/draw.h"
#include "ms/highscore.h"

/// Displays a boxed, navigable menu and returns the selected option index.
static int _select_menu(int x, int y, int width, int height, int n_lines, int n_options, const char **lines) {
    WINDOW *window = newwin(height, width, y, x);
    keypad(window, true);
    int cv = curs_set(0);
    int ch = 0;
    int opt = 0;
    while (ch != KEY_ENTER && ch != '\n' && ch != '\r' && ch != ' ') {
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

/// Resolves a difficulty name to a preset index, or opens the menu if none matches.
int select_difficulty(int term_width, int term_height, char *difficulty, bool no_guess) {
    LOG(LOG_INFO, "Selecting difficulty");
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
            if (strcmp(difficulty, level) == 0 && i != CUSTOM_DIFFICULTY + DIFFICULTY_OPS - DIFFICULTY_LINES) {
                LOG(LOG_INFO, "Using difficulty provided by command line argument");
                return (i + no_guess);
            }
        }
    }

    int width = 0;
    for (int i = 0; i < DIFFICULTY_LINES; i++) {
        if ((int) strlen(DIFFICULTIES[i]) > width)
            width = strlen(DIFFICULTIES[i]);
    }
    width += BOX_WIDTH;
    if (width > term_width || (DIFFICULTY_LINES + 2) > term_height) {
        screen_too_small();
        return -1;
    }
    return _select_menu(
            (term_width - width) / 2,
            (term_height - (DIFFICULTY_LINES + 2)) / 2,
            width,
            DIFFICULTY_LINES + 2,
            DIFFICULTY_LINES,
            DIFFICULTY_OPS,
            DIFFICULTIES);
}

/// Renders and saves end-of-game and personal-best statistics in a popup window.
static void _show_stats(Game *game) {
    LOG(LOG_INFO, "Showing game stats");
    char time_text[BUFSIZ];
    char time_text_pb[BUFSIZ];

    char bbbv_text[BUFSIZ];

    char bbbv_s_text[BUFSIZ];
    char bbbv_s_text_pb[BUFSIZ];

    char clicks_text[BUFSIZ];
    char clicks_text_pb[BUFSIZ];

    char efficiency_text[BUFSIZ];
    char efficiency_text_pb[BUFSIZ];

    char *text[] = {
        "Game Stats",
        time_text,
        bbbv_text,
        bbbv_s_text,
        clicks_text,
        efficiency_text,
        "",
        "All Time",
        time_text_pb,
        bbbv_s_text_pb,
        clicks_text_pb,
        efficiency_text_pb,
        NULL
    };

    // Time
    if (game->time)
        snprintf(time_text, sizeof(time_text), "Time: %lld.%3.llds", game->time / 1000, game->time % 1000);
    else
        snprintf(time_text, sizeof(time_text), "Time: None");

    // 3BV
    int bbbv = get_bbbv(game->grid, game->width, game->height);
    snprintf(bbbv_text, sizeof(bbbv_text), "3BV: %d", bbbv);

    // 3BV/s
    double bbbv_s = 0;
    if (game->time)
        bbbv_s = (double) bbbv / ((double) game->time / 1000);
    snprintf(bbbv_s_text, sizeof(bbbv_s_text), "3BV/s: %g", bbbv_s);

    // Clicks
    snprintf(clicks_text, sizeof(clicks_text), "Clicks: %d", game->clicks);

    // Efficiency
    double efficiency = 0;
    if (game->clicks) {
        efficiency = ((double) bbbv / game->clicks);
        snprintf(efficiency_text, sizeof(efficiency_text), "Efficiency: %g%%", efficiency * 100);
    }
    Scores scores = (Scores) {
        .time = game->time,
        .bbbv_s = bbbv_s,
        .clicks = game->clicks,
        .efficiency = efficiency,
    };
    Scores highscores = load_highscore(game->difficulty);
    if (scores.time >= highscores.time)
        strncat(time_text, " (PB)", sizeof(time_text) - strlen(time_text) - 1);

    if (scores.bbbv_s >= highscores.bbbv_s)
        strncat(bbbv_s_text, " (PB)", sizeof(bbbv_s_text) - strlen(bbbv_s_text) - 1);

    if (scores.clicks >= highscores.clicks)
        strncat(clicks_text, " (PB)", sizeof(clicks_text) - strlen(clicks_text) - 1);

    if (scores.efficiency >= highscores.efficiency)
        strncat(efficiency_text, " (PB)", sizeof(efficiency_text) - strlen(efficiency_text) - 1);

    write_highscore(&scores, game->difficulty);
    highscores = load_highscore(game->difficulty);

    // Time
    if (highscores.clicks)
        snprintf(time_text_pb, sizeof(time_text_pb), "Time: %lld.%3.llds", highscores.time / 1000, highscores.time % 1000);
    else
        snprintf(time_text_pb, sizeof(time_text_pb), "Time: None");
    // 3BV/s
    snprintf(bbbv_s_text_pb, sizeof(bbbv_s_text_pb), "3BV/s: %g", highscores.bbbv_s);
    // Clicks
    if (highscores.clicks)
        snprintf(clicks_text_pb, sizeof(clicks_text_pb), "Clicks: %d", highscores.clicks);
    else
        snprintf(clicks_text_pb, sizeof(clicks_text_pb), "Clicks: None");
    // Efficiency
    snprintf(efficiency_text_pb, sizeof(efficiency_text_pb), "Efficiency: %g%%", highscores.efficiency * 100);

    int width = 0;
    for (int i = 0; text[i]; i++) {
        if ((int) strlen(text[i]) > width)
            width = strlen(text[i]);
    }
    width += BOX_WIDTH;
    int height = 0;
    for (int i = 0; text[i]; i++) {
        height++;
    }
    height += BOX_WIDTH;
    if (width > getmaxx(stdscr) || height > getmaxy(stdscr)) {
        screen_too_small();
        return;
    }
    WINDOW *window = newwin(getmaxy(stdscr), width, 0, 0);
    box(window, 0, 0);

    for (int i = 0; text[i]; i++) {
        mvwprintw(window, i + BOX_WIDTH / 2, BOX_WIDTH / 2, "%s", text[i]);
    }
    wrefresh(window);
    delwin(window);
}

/// Presents the win/lose result and play-again or export options.
int select_play_again(int term_width, bool show_export, Game *game) {
    LOG(LOG_INFO, "Selecting play again option");
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
    width += BOX_WIDTH;
    int nlines = PLAY_AGAIN_LINES - !show_export;
    int nops = PLAY_AGAIN_OPS - !show_export;
    if (width > term_width || nlines + 2 > getmaxy(stdscr)) {
        screen_too_small();
        return -1;
    }
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

/// Lets the user set custom width, height and mines via the keyboard.
Preset select_custom(int term_width, int term_height, Preset config) {
    LOG(LOG_INFO, "Selecting custom grid dimensions");
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
    width += BOX_WIDTH;
    if (width > term_width || CUSTOM_LINES + 2 > term_height) {
        screen_too_small();
        return (Preset) {
            .width = -1,
            .height = -1,
            .mines = -1
        };
    }
    WINDOW *window = newwin(CUSTOM_LINES + 2, width, (term_height - (CUSTOM_LINES + 2)) / 2, (term_width - width) / 2);
    keypad(window, true);
    int cv = curs_set(0);
    for (int i = 0; i < CUSTOM_LINES; i++) {
        int ch = 0;
        box(window, 0, 0);
        while (ch != KEY_ENTER && ch != '\n' && ch != '\r') {
            for (int j = 0; j < CUSTOM_LINES; j++) {
                if (j == i)
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
    }
    wclear(window);
    wrefresh(window);
    delwin(window);
    curs_set(cv);
    refresh();
    return config;
}

/// Prompts the user to type a filename for export.
void select_filename(int term_width, char *filename, size_t len) {
    LOG(LOG_INFO, "Selecting filename");
    if (!filename) {
        LOG(LOG_ERROR, "Invalid filename buffer");
        return;
    }
    for (char *p = filename; p < filename + len; p++) {
        *p = '\0';
    }
    if (FILENAME_LINES + 2 > getmaxx(stdscr)) {
        screen_too_small();
        return;
    }
    WINDOW *window = newwin(FILENAME_LINES + 2, term_width, 0, 0);
    keypad(window, true);
    box(window, 0, 0);
    int ch = 0;
    size_t index = 0;
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
