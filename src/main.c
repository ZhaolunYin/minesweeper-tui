#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <argp.h>

#include "ms.h"
#include "ms/log.h"

int main(int argc, char **argv) {
    LOG(LOG_INFO, "Program started");

    LOG(LOG_DEBUG, "Parsing command line arguments");
    struct arguments args = default_args();
    argp_parse(&argp, argc, argv, 0, 0, &args);

    LOG(LOG_DEBUG, "Initialising ncurses screen");
    initscr();
    if (args.color)
        init_color_pairs();
    mousemask(BUTTON1_PRESSED | BUTTON3_PRESSED, NULL);
    cbreak();
    noecho();

    LOG(LOG_DEBUG, "Seeding random number generator with seed %d", args.seed);
    srand(args.seed);
    bool play_again = true;
    while (play_again) {
        LOG(LOG_INFO, "Started new game");
        Game *game = init_game(&args);
        if (!game) {
            LOG(LOG_ERROR, "Failed to initialise game object");
            endwin();
            return 1;
        }
        game_loop(game);
        play_again = end_game(game);
        free(game);
    }
    end_log();
    endwin();
    LOG(LOG_INFO, "Program ended");
}
