#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ms.h"

static char *_highscore_filename() {
    char *buf = malloc(BUFSIZ);
    char *cur = buf;
    char *end = buf + BUFSIZ;
    if (getenv("XDG_DATA_HOME")) {
        cur += snprintf(cur, end - cur, "%s", getenv("XDG_DATA_HOME")); 
    }
    else if (getenv("HOME")) {
        cur += snprintf(cur, end - cur, "%s/.local/share", getenv("HOME"));
    }
    cur += snprintf(cur, end - cur, "/%s", FOLDER_NAME);
    mkdir(buf, 0755);
    cur += snprintf(cur, end - cur, "/%s", SCORE_FILE);
    return buf;
}

static void _fix_highscore_file() {
    char *file = _highscore_filename();

    FILE *fptr = fopen(file, "w");
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        fprintf(fptr, "0\n");
    }
    fclose(fptr);

    free(file);
}

long long load_highscore(int difficulty) {
    if (difficulty < 0 || difficulty > SCORING_DIFFICULTIES - 1)
        return 0;
    char *file = _highscore_filename();
    FILE *fptr = fopen(file, "r");
    if (!fptr) {
        _fix_highscore_file();
        return 0;
    }
    long long result[SCORING_DIFFICULTIES];
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        if (fscanf(fptr, "%lld", &result[i]) != 1) {
            fclose(fptr);
            _fix_highscore_file();
            return 0;
        }
    }
    fclose(fptr);
    free(file);
    return result[difficulty];
}

void write_highscore(long score, int difficulty) {
    long long scores[SCORING_DIFFICULTIES];
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        scores[i] = load_highscore(i);
    }
    scores[difficulty] = score;
    if (difficulty < 0 || difficulty > SCORING_DIFFICULTIES - 1)
        return;
    char *file = _highscore_filename();
    FILE *fptr = fopen(file, "w");
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        fprintf(fptr, "%lld\n", scores[i]);
    }
    fclose(fptr);
    free(file);
}

void show_highscores() {
    size_t width = 0;
    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        size_t len = strlen(DIFFICULTIES[i + DIFFICULTY_LINES - DIFFICULTY_OPS]);
        if (len > width)
            width = len;
    }
    width += 1;
    const char *col1 = "Difficulty";
    const char *col2 = "Time";

    printf("%s", col1);
    // + 1 for colon
    for (size_t i = 0; i < width - strlen(col1) + 1; i++)
        printf(" ");
    printf("%s\n", col2);

    // + 1 for colon
    for (size_t i = 0; i < width + 1; i++)
        printf("-");

    for (size_t i = 0; i < strlen(col2); i++)
        printf("-");
    printf("\n");

    for (int i = 0; i < SCORING_DIFFICULTIES; i++) {
        long long score = load_highscore(i);
        printf("%s:", DIFFICULTIES[i + 1]);
        size_t len = strlen(DIFFICULTIES[i + DIFFICULTY_LINES - DIFFICULTY_OPS]);
        for (size_t j = 0; j < width - len; j++)
            printf(" ");
        if (score)
            printf("%lld.%3.llds\n", score / 1000, score % 1000);
        else
            printf("None\n");
    }
}
