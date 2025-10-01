#include "typing_test.h"
#include "constants.h"
#include "err.h"
#include "helpers.h"
#include "typing_test_view.h"
#include "word_store.h"
#include <limits.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void typing_test_init(Err **err, TypingTest **typing_test, WordStore *ws,
                      size_t word_count) {

    size_t words_buff_size = word_count * sizeof(((WordStore *)0)->words[0]);
    TypingTest *t = calloc((size_t)1, sizeof(TypingTest) + words_buff_size);
    if (!t) {
        *err = ERR_MAKE("Unable to allocate memory for typing test");
        return;
    }

    t->words_count = word_count;
    size_t view_width = MIN_N((size_t)COLS, MAX_CHARS_PER_LINE);

    word_store_randn(err, ws, t->words_count, t->words);
    if (*err) {
        return;
    }

    typing_test_view_init(err, &t->view, view_width, t->words, t->words_count);
    if (*err) {
        typing_test_destroy(&t);
        return;
    }

    *typing_test = t;
    return;
}

void typing_test_start(Err **err, TypingTest *tt) {
    if (!tt) {
        *err = ERR_MAKE("Typing test is null");
        return;
    }
    size_t current_word_i = 0;
    typing_test_view_render(err, tt->view, current_word_i);
    getch();
}

void typing_test_destroy(TypingTest **typing_test) {
    if (!typing_test) {
        return;
    }

    TypingTest *tt = *typing_test;
    if (!tt) {
        *typing_test = NULL;
        return;
    }

    if (tt->view) {
        typing_test_view_destroy(&tt->view);
    }

    free(tt);
    tt = NULL;
    *typing_test = NULL;
}
