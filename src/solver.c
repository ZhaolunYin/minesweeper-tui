#include "solver.h"
#include "grid.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static Neighbours _get_neighbours(Square *grid, int width, int height, int x, int y) {
    Neighbours neighbours;
    neighbours.n = 0;
    FOR_EACH_NEIGHBOUR(x, y, width, height) {
        neighbours.x[neighbours.n] = rx;
        neighbours.y[neighbours.n] = ry;
        neighbours.square[neighbours.n] = get_square(grid, width, height, rx, ry);
        neighbours.n++;
    }
    return neighbours;
}

static int _count_flags(Neighbours *n) {
    int c = 0;
    for (int i = 0; i < n->n; i++)
        c += n->square[i]->flag;
    return c;
}

static int _count_covered(Neighbours *n) {
    int c = 0;
    for (int i = 0; i < n->n; i++) {
        if (!n->square[i]->uncovered && !n->square[i]->flag)
            c++;
    }
    return c;
}

static int _count_shared_covered(Neighbours *a, Neighbours *b) {
    int c = 0;
    for (int i = 0; i < a->n; i++) {
        if (!a->square[i]->uncovered && !a->square[i]->flag) {

            for (int j = 0; j < b->n; j++) {
                if (a->square[i] == b->square[j])
                    c++;
            }
        }
    }
    return c;
}

static int _count_unshared_covered(Neighbours *a, Neighbours *b) {
    int c = 0;
    for (int i = 0; i < a->n; i++) {
        if (!a->square[i]->uncovered && !a->square[i]->flag) {

            bool duplicate = false;
            for (int j = 0; j < b->n; j++) {
                if (a->square[i] == b->square[j])
                    duplicate = true;
            }
            if (!duplicate)
                c++;
        }
    }
    return c;
}

static Neighbours _get_unshared_covered(Neighbours *a, Neighbours *b) {
    Neighbours n;
    n.n = 0;
    for (int i = 0; i < a->n; i++) {
        if (!a->square[i]->uncovered && !a->square[i]->flag) {

            bool duplicate = false;
            for (int j = 0; j < b->n; j++) {
                if (a->square[i] == b->square[j])
                    duplicate = true;
            }
            if (!duplicate) {
                n.x[n.n] = a->x[i];
                n.y[n.n] = a->y[i];
                n.square[n.n] = a->square[i];
                n.n++;
            }
        }
    }
    return n;
}

static bool _b1_pattern(Square *grid, int width, int height, int x, int y) {
    Neighbours n = _get_neighbours(grid, width, height, x, y);
    int covered = _count_covered(&n);
    int flagged = _count_flags(&n);
    int mines_needed = get_square(grid, width, height, x, y)->surrounding - flagged;

    if (covered == mines_needed && mines_needed) {
        for (int i = 0; i < n.n; i++) {
            if (!n.square[i]->uncovered && !n.square[i]->flag)
                n.square[i]->flag = true;
        }
        return true;
    }
    return false;
}

static bool _b2_pattern(Square *grid, int width, int height, int x, int y) {
    Neighbours n = _get_neighbours(grid, width, height, x, y);
    int flagged = _count_flags(&n);
    int mines_needed = get_square(grid, width, height, x, y)->surrounding - flagged;

    if (!mines_needed) {
        for (int i = 0; i < n.n; i++) {
            if (!n.square[i]->uncovered && !n.square[i]->flag)
                select_square(&grid, width, height, n.x[i], n.y[i]);
        }
        return true;
    }
    return false;
}

static bool _1_1_pattern(Square *grid, int width, int height, int x1, int y1, int x2, int y2) {
    bool changed = false;
    Neighbours a = _get_neighbours(grid, width, height, x1, y1);
    Neighbours b = _get_neighbours(grid, width, height, x2, y2);
    if (_count_shared_covered(&a, &b) == _count_covered(&a)) {
        int minediff = (get_square(grid, width, height, x2, y2)->surrounding - _count_flags(&b)) -
            (get_square(grid, width, height, x1, y1)->surrounding - _count_flags(&a));
        if (!minediff) {
            Neighbours to_uncover = _get_unshared_covered(&b, &a);
            for (int i = 0; i < to_uncover.n; i++) {
                select_square(&grid, width, height, to_uncover.x[i], to_uncover.y[i]);
                changed = true;
            }
        }
    }
    else if (_count_shared_covered(&a, &b) == _count_covered(&b)) {
        int minediff = (get_square(grid, width, height, x1, y1)->surrounding - _count_flags(&a)) -
            (get_square(grid, width, height, x2, y2)->surrounding - _count_flags(&b));
        if (!minediff) {
            Neighbours to_uncover = _get_unshared_covered(&a, &b);
            for (int i = 0; i < to_uncover.n; i++) {
                select_square(&grid, width, height, to_uncover.x[i], to_uncover.y[i]);
                changed = true;
            }
        }
    }
    return changed;
}

static bool _1_2_pattern(Square *grid, int width, int height, int x1, int y1, int x2, int y2) {
    bool changed = false;
    Neighbours a = _get_neighbours(grid, width, height, x1, y1);
    Neighbours b = _get_neighbours(grid, width, height, x2, y2);

    if (get_square(grid, width, height, x1, y1)->surrounding - _count_flags(&a) 
            > get_square(grid, width, height, x2, y2)->surrounding - _count_flags(&b)) {
        int temp = x1;
        x1 = x2;
        x2 = temp;

        temp = y1;
        y1 = y2;
        y2 = temp;

        a = _get_neighbours(grid, width, height, x1, y1);
        b = _get_neighbours(grid, width, height, x2, y2);
    }
    Square *sq1 = get_square(grid, width, height, x1, y1);
    Square *sq2 = get_square(grid, width, height, x2, y2);

    int diff = (sq2->surrounding - _count_flags(&b)) - (sq1->surrounding - _count_flags(&a));
    if (diff == _count_unshared_covered(&b, &a)) {
        Neighbours to_flag = _get_unshared_covered(&b, &a);
        for (int i = 0; i < to_flag.n; i++) {
            to_flag.square[i]->flag = true;
            changed = true;
        }
        Neighbours to_uncover = _get_unshared_covered(&a, &b);
        for (int i = 0; i < to_uncover.n; i++) {
            select_square(&grid, width, height, to_uncover.x[i], to_uncover.y[i]);
            changed = true;
        }
    }
    return changed;
}

static bool _square_visited(Square **visited, int len, Square *square) {
    for (int i = 0; i < len; i++) {
        if (visited[i] == square) {
            return true;
        }
    }
    return false;
}

static bool _apply_basic_logic(Square *grid, int width, int height) {
    bool changed = false;
    FOR_EACH_IN_GRID(grid, width, height) {
        Square *square = get_square(grid, width, height, x, y);
        if (!square->uncovered || !square->surrounding)
            continue;

        if (_b1_pattern(grid, width, height, x, y))
            changed = true;

        if (_b2_pattern(grid, width, height, x, y))
            changed = true;
    }
    return changed;
}


static bool _apply_advanced_logic(Square *grid, int width, int height) {
    bool changed = false;
    Square **visited = calloc((size_t) width * height, sizeof(Square *));
    int index = 0;
    FOR_EACH_IN_GRID(grid, width, height) {
        Square *sq1 = get_square(grid, width, height, x, y);
        if (!sq1->uncovered)
            continue;
        visited[index++] = sq1;
        FOR_EACH_NEIGHBOUR(x, y, width, height) {
            Square *sq2 = get_square(grid, width, height, rx, ry);
            if (_square_visited(visited, index, sq2) || !sq2->uncovered)
                continue;
            if (_1_1_pattern(grid, width, height, x, y, rx, ry))
                changed = true;
            if (_1_2_pattern(grid, width, height, x, y, rx, ry))
                changed = true;
        }
    }
    free(visited);
    return changed;
}

bool solve_board(Square *grid, int width, int height, int x, int y) {
    Square *copy = malloc((size_t) width * height * sizeof(Square));
    if (!copy)
        return false;
    memcpy(copy, grid, (size_t) width * height * sizeof(Square));
    select_square(&copy, width, height, x, y);
    bool changed = true;
    while (!all_selected(copy, width, height) && changed) {
        changed = false;

        if (_apply_basic_logic(copy, width, height))
            changed = true;

        if (_apply_advanced_logic(copy, width, height))
            changed = true;
    }
    free(copy);
    return changed;
}
