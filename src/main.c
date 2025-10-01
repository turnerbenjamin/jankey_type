#include "constants.h"
#include "err.h"
#include "typing_test.h"
#include "word_store.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

void init_ncurses(Err **err);

int main() {
    Err *err = NULL;

    init_ncurses(&err);
    if (err) {
        endwin();
        err_print(err, stderr);
        err_destroy(&err);
        return EXIT_FAILURE;
    }
    srand((unsigned int)time(NULL));

    WordStore *ws;
    word_store_init(&err, &ws, "dict/en_gb.txt");
    if (err) {
        word_store_destroy(&ws);

        endwin();

        err_print(err, stderr);
        err_destroy(&err);
        return EXIT_FAILURE;
    }

    TypingTest *tt;
    typing_test_init(&err, &tt, ws, WORDS_PER_TEST);
    if (err) {
        endwin();

        typing_test_destroy(&tt);
        word_store_destroy(&ws);

        err_print(err, stderr);
        err_destroy(&err);

        return EXIT_FAILURE;
    }

    typing_test_start(&err, tt);
    if (err) {
        typing_test_destroy(&tt);
        word_store_destroy(&ws);

        endwin();

        err_print(err, stderr);
        err_destroy(&err);
        return EXIT_FAILURE;
    }

    typing_test_destroy(&tt);
    word_store_destroy(&ws);
    err_destroy(&err);

    endwin();

    return EXIT_SUCCESS;
}

void init_ncurses(Err **err) {
    initscr();

    if (!has_colors()) {
        *err = ERR_MAKE("Colors not available for the terminal");
        return;
    }
    start_color();

    clear();
    refresh();
    cbreak();
    noecho();

    if (init_extended_pair(1, COLOR_GREEN, COLOR_BLACK) == ERR) {
        *err = ERR_MAKE("Unable to initialise pair");
        return;
    }
    if (init_extended_pair(2, COLOR_RED, COLOR_BLACK) == ERR) {
        *err = ERR_MAKE("Unable to initialise pair");
        return;
    }
}
