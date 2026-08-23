#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ncurses.h>
#include <getopt.h>

#include "draw.h"
#include "game.h"

int main(int argc, char **argv) {
    int cols = 0, rows = 0, mines = 0;
    int seed = time(NULL);
    int c;
    for (;;) {
        static struct option long_options[] = {
            { "cols", required_argument, 0, 'c' },
            { "rows", required_argument, 0, 'r' },
            { "mines", required_argument, 0, 'm' },
            { "seed", required_argument, 0, 's' },
            { 0, 0, 0, 0 },
        };
        c = getopt_long(argc, argv, "c:m:r:s:", long_options, NULL);
        if (c == EOF)
            break;
        switch (c) {
            case 'c':
                cols = atoi(optarg);
                break;
            case 'r':
                rows = atoi(optarg);
                break;
            case 'm':
                mines = atoi(optarg);
                break;
            case 's':
                seed = atoi(optarg);
                break;
        }
    }
    initscr();
    init_color_pairs();
    mousemask(BUTTON1_PRESSED | BUTTON3_PRESSED, NULL);
    cbreak();
    noecho();
    bool play_again = true;
    while (play_again) {
        srand(seed);
        Game game = init_game(cols, rows, mines);
        game_loop(&game);
        play_again = end_game(&game);
    }
    endwin();
}
