#include "ms/highscore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ms.h"
#include "ms/log.h"

/// Builds and returns the highscore file path, creating its directory.
static char *_highscore_filename(void) {
    char *buf = malloc(BUFSIZ);
    if (!buf) {
        LOG(LOG_ERROR, "Failed to initialize filename buffer");
        return NULL;
    }
    char *cur = buf;
    char *end = buf + BUFSIZ;
    if (getenv("XDG_DATA_HOME")) {
        cur += snprintf(cur, end - cur, "%s", getenv("XDG_DATA_HOME")); 
    }
    else if (getenv("HOME")) {
        cur += snprintf(cur, end - cur, "%s/.local/share", getenv("HOME"));
    }
    cur += snprintf(cur, end - cur, "/%s", PROGRAM_NAME);
    mkdir(buf, 0755);
    cur += snprintf(cur, end - cur, "/%s", SCORE_FILE);
    return buf;
}

/// Creates a fresh highscore file filled with zeroed scores.
static void _fix_highscore_file(void) {
    LOG(LOG_WARNING, "Highscore file was malformed or non-existant. Generating new file");
    char *file = _highscore_filename();
    if (!file)
        return;

    FILE *fptr = fopen(file, "w");
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        fprintf(fptr, "0,0,0,0\n");
    }
    fclose(fptr);

    free(file);
}

/// Loads the stored scores for a difficulty, recreating the file if it is missing.
Scores load_highscore(int difficulty) {
    LOG(LOG_INFO, "Loading highscore");
    if (difficulty < 0 || difficulty > SCORING_DIFFICULTIES - 1) {
        LOG(LOG_ERROR, "Invalid difficulty");
        return (Scores) { 0 };
    }
    char *file = _highscore_filename();
    if (!file)
        return (Scores) { 0 };
    FILE *fptr = fopen(file, "r");
    if (!fptr) {
        _fix_highscore_file();
        return (Scores) { 0 };
    }
    Scores result[SCORING_DIFFICULTIES];
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        if (fscanf(fptr, "%lld,%lf,%d,%lf", &result[i].time, &result[i].bbbv_s, &result[i].clicks, &result[i].efficiency) != 4) {
            fclose(fptr);
            _fix_highscore_file();
            return (Scores) { 0 };
        }
    }
    fclose(fptr);
    free(file);
    return result[difficulty];
}

/// Updates and persists the highscore for a difficulty if it beats the stored value.
void write_highscore(Scores *score, int difficulty) {
    LOG(LOG_INFO, "Saving highscore");
    if (difficulty < 0 || difficulty > SCORING_DIFFICULTIES - 1) {
        LOG(LOG_ERROR, "Invalid difficulty");
        return;
    }
    Scores scores[SCORING_DIFFICULTIES];
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        scores[i] = load_highscore(i);
    }

    // Time
    if (!scores[difficulty].time || score->time < scores[difficulty].time) {
        scores[difficulty].time = score->time;
        LOG(LOG_INFO, "Saved time");
    }
    // 3BV/s
    if (!scores[difficulty].bbbv_s || score->bbbv_s > scores[difficulty].bbbv_s) {
        scores[difficulty].bbbv_s = score->bbbv_s;
        LOG(LOG_INFO, "Saved 3BV");
    }
    // Clicks
    if (!scores[difficulty].clicks || score->clicks < scores[difficulty].clicks) {
        scores[difficulty].clicks = score->clicks;
        LOG(LOG_INFO, "Saved clicks");
    }
    // Efficiency
    if (!scores[difficulty].efficiency || score->efficiency > scores[difficulty].efficiency) {
        scores[difficulty].efficiency = score->efficiency;
        LOG(LOG_INFO, "Saved efficiency");
    }

    char *file = _highscore_filename();
    if (!file)
        return;
    FILE *fptr = fopen(file, "w");
    if (!fptr) {
        LOG(LOG_ERROR, "Failed to open file");
        return;
    }

    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        fprintf(fptr, "%lld,%lf,%d,%lf\n", scores[i].time, scores[i].bbbv_s, scores[i].clicks, scores[i].efficiency);
    }
    fclose(fptr);
    free(file);
}

/// Prints all highscores to standard output in a table.
void show_highscores(void) {
    char difficulty_col[SCORING_DIFFICULTIES][BUFSIZ];

    char time_col[SCORING_DIFFICULTIES][BUFSIZ];
    char bbbv_s_col[SCORING_DIFFICULTIES][BUFSIZ];
    char clicks_col[SCORING_DIFFICULTIES][BUFSIZ];
    char efficiency_col[SCORING_DIFFICULTIES][BUFSIZ];

    size_t difficulty_width = strlen("Difficulty");

    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        snprintf(difficulty_col[i], BUFSIZ, "%s", DIFFICULTIES[i + DIFFICULTY_LINES - DIFFICULTY_OPS]);
        Scores scores = load_highscore(i);
        if (scores.time)
            snprintf(time_col[i], BUFSIZ, "%lld.%3.llds", scores.time / 1000, scores.time % 1000);
        else
            snprintf(time_col[i], BUFSIZ, "None");

        snprintf(bbbv_s_col[i], BUFSIZ, "%g", scores.bbbv_s);

        if (scores.clicks)
            snprintf(clicks_col[i], BUFSIZ, "%d", scores.clicks);
        else
            snprintf(clicks_col[i], BUFSIZ, "None");

        snprintf(efficiency_col[i], BUFSIZ, "%g%%", scores.efficiency);

        if (strlen(difficulty_col[i]) > difficulty_width)
            difficulty_width = strlen(difficulty_col[i]);
    }

    printf("%-*s  %-10s  %-8s  %s", (int) difficulty_width, "Difficulty", "Time", "3BV/s", "Efficiency");
    printf("\n");
    for (size_t i = 0; i < difficulty_width + 2 + 10 + 1 + 8 + 1 + strlen("Efficiency"); i++)
        printf("-");
    printf("\n");

    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        printf("%-*s  ", (int) difficulty_width, difficulty_col[i]);
        printf("%-10s  ", time_col[i]);
        printf("%-8s  ", bbbv_s_col[i]);
        printf("%s\n", efficiency_col[i]);
    }
}
