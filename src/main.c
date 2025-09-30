#include "config.h"
#include "err.h"
#include "typing_test.h"
#include "word_store.h"
#include <stdlib.h>
#include <time.h>

void init_ncurses(void);

int main() {
    endwin();
    srand((unsigned int)time(NULL));

    WordStore *ws;
    Err *err;
    word_store_init(&err, &ws, "dict/en_gb.txt");
    if (err) {
        word_store_destroy(&ws);

        endwin();

        err_print(err, stderr);
        err_destroy(&err);
        return EXIT_FAILURE;
    }

    init_ncurses();

    TypingTest *tt;
    typing_test_init(&err, &tt, ws, WORDS_PER_TEST);
    if (err) {
        typing_test_destroy(&tt);
        word_store_destroy(&ws);

        endwin();

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

void init_ncurses() {
    initscr();
    clear();
    refresh();
    cbreak();
    noecho();
}
