#include "config.h"
#include "typing_test.h"
#include "word_store.h"
#include <stdlib.h>
#include <time.h>

void init_ncurses() {
    initscr();
    clear();
    refresh();
    cbreak();
    noecho();
}

int main() {
    srand(time(NULL));

    WordStore *ws;
    if (!word_store_init(&ws, "dict/en_gb.txt")) {
        word_store_destroy(&ws);
        return EXIT_FAILURE;
    }

    init_ncurses();
    TypingTest *tt;
    if (!typing_test_init(&tt, ws, WORDS_PER_TEST)) {
        typing_test_destroy(&tt);
        word_store_destroy(&ws);
        return EXIT_FAILURE;
    }

    typing_test_start(tt);

    typing_test_destroy(&tt);
    word_store_destroy(&ws);
    endwin();

    return EXIT_SUCCESS;
}
