#include <stdlib.h>
#include <stdbool.h>
#include <threads.h>
#include <time.h>
#include <ncurses.h>

#include "draw.h"
#include "game.h"

int main() {
    srand(time(NULL));
    initscr();
    init_color_pairs();
    mousemask(BUTTON1_PRESSED | BUTTON3_PRESSED, NULL);
    cbreak();
    noecho();
    bool play_again = true;
    while (play_again) {
        Game game = init_game();
        game_loop(&game);
        play_again = end_game(&game);
    }
    endwin();
}
