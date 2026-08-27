#pragma once

#define SCORING_DIFFICULTIES 6

#define SCORE_FILE "scores"

typedef struct {
    long long time;
    double bbbv_s;
    int clicks;
    double efficiency;
} Scores;

void show_highscores(void);
Scores load_highscore(int difficulty);
void write_highscore(Scores *score, int difficulty);
