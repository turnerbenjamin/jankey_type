
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
#include <threads.h>
#include <time.h>
#include <unistd.h>

struct TypingTest {
    TypingTestView *view;
    char *test_str;
    uint64_t test_str_len;
};

void tt_runbench(Err **err, TypingTest *tt);

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

    /*
    tt_runbench(err, tt);
    if (*err)
        return;
    */

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

void tt_runbench(Err **err, TypingTest *tt) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    size_t chars = 0;
    while (true) {
        char c = tt->test_str[chars++];
        typing_test_view_addch(tt->view, &c, TTV_TYPEMODE_OVERTYPE);
        if (chars == tt->test_str_len) {
            break;
        }
        typing_test_view_render(err, tt->view);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in seconds
    double seconds = (double)(end.tv_sec - start.tv_sec) +
                     (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0f;

    double words = ((double)chars / (double)5);

    double minutes = seconds / 60.0f;
    double wpm = words / minutes;

    *err =
        ERR_MAKE("Chars: %lu, Words: %lf, Seconds: %lf, WPM: %.2lf, BUFF: %lu",
                 chars, words, seconds, wpm, tt->test_str_len);
    return;
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
