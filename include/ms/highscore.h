#pragma once

#define SCORING_DIFFICULTIES 6
#define FOLDER_NAME "minesweeper-tui"
#define SCORE_FILE "score"

void show_highscores();
long long load_highscore(int difficulty);
void write_highscore(long score, int difficulty);
