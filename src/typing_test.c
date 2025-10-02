#include "typing_test.h"
#include "err.h"
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

struct TypingTest {
    TypingTestView *view;
    char *test_str;
    uint64_t test_str_len;
};

void typing_test_init(Err **err, TypingTest **typing_test, WordStore *ws,
                      size_t word_count) {

    size_t words_buff_size = word_count * sizeof(((WordStore *)0)->words[0]);
    TypingTest *t = calloc((size_t)1, sizeof(TypingTest) + words_buff_size);
    if (!t) {
        *err = ERR_MAKE("Unable to allocate memory for typing test");
        return;
    }

    t->test_str_len = word_store_rands(err, ws, word_count, &t->test_str);
    if (*err) {
        return;
    }

    typing_test_view_init(err, &t->view, t->test_str, t->test_str_len);
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

    while (true) {
        typing_test_view_render(err, tt->view);
        int c = getch();
        if (c == KEY_BACKSPACE) {
            typing_test_delch(tt->view);
        } else {
            typing_test_addch(tt->view);
        }
    }
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
