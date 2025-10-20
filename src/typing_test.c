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
#include <time.h>

struct TypingTest {
    TypingTestView *view;
    char *test_str;
    uint64_t test_str_len;
};

// void tt_runbench(Err **err, TypingTest *tt);

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

    typing_test_view_render(err, tt->view);
    char c;
    size_t i = 0;
    while (true) {
        int ui = getch();
        if (ui < 0) {
            continue;
        }
        c = (char)ui;

        TTV_TYPEMODE m = c == 'x' ? TTV_TYPEMODE_INSERT : TTV_TYPEMODE_OVERTYPE;

        size_t new_i = typing_test_view_addch(tt->view, &c, m);
        if (new_i == i) {
            break;
        }
        i++;
        typing_test_view_render(err, tt->view);
    }
}

/*
void tt_runbench(Err **err, TypingTest *tt) {
    long t1 = clock();
    size_t cursor_p = 0;
    size_t next_cursor_p = 0;
    size_t chars = 0;
    while (true) {
        next_cursor_p = typing_test_view_addch(tt->view);
        if (next_cursor_p == cursor_p) {
            break;
        }
        typing_test_view_render(err, tt->view);
        cursor_p = next_cursor_p;
        chars++;
    }
    long t = clock() - t1;

    double words = ((double)chars / (double)5);
    double words_per_tick = words / (double)t;
    double wpm = words_per_tick * (double)CLOCKS_PER_SEC * (double)60;

    *err = ERR_MAKE("Ticks: %ld, Chars: %ul, WPM: %.2lf", t, chars, wpm);
    return;
}
*/

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
