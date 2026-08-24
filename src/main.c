#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <argp.h>

#include "ms.h"

int main(int argc, char **argv) {
    struct arguments args = default_args();
    argp_parse(&argp, argc, argv, 0, 0, &args);

    initscr();
    if (args.color)
        init_color_pairs();
    mousemask(BUTTON1_PRESSED | BUTTON3_PRESSED, NULL);
    cbreak();
    noecho();
    bool play_again = true;
    while (play_again) {
        srand(args.seed);
        Game *game = init_game(&args);
        if (!game) {
            endwin();
            return 1;
        }
        game_loop(game);
        play_again = end_game(game);
        free(game);
    }
    endwin();
}
