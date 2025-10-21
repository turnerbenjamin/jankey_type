
#include "typing_test.h"
#include "constants.h"
#include "err.h"
#include "typing_test_stats.h"
#include "typing_test_view.h"
#include "views/round_end_view.h"
#include "word_store.h"
#include <ctype.h>
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
    TypingTestStats *stats;
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

    tt_stats_init(err, &t->stats);
    if (*err) {
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

    tt_stats_reset(tt->stats);
    tt_stats_start(tt->stats);

    typing_test_view_render(err, tt->view);

    RoundEndView *round_end_view;
    ttv_roundend_init(err, &round_end_view);
    if (*err) {
        return;
    }
    ttv_roundend_render(err, round_end_view);
    while (true) {
        int ui = getch();
        if (ui == KEY_BACKSPACE || ui == 127 || ui == 8) {
            ttv_roundend_destroy(&round_end_view);
            clear();
            refresh();
            break;
        }
    }

    typing_test_view_render(err, tt->view);
    char c;
    size_t i = 0;
    size_t last_i = 0;
    while (true) {
        int ui = getch();
        if (ui < 0) {
            continue;
        }
        if (ui == KEY_BACKSPACE || ui == 127 || ui == 8) {
            if (i == 0) {
                continue;
            }
            char correct_char = tt->test_str[i - 1];
            i = typing_test_view_deletechar(tt->view, &correct_char);
        } else {
            c = (char)ui;
            if (!isprint(c)) {
                continue;
            }

            char correct_char = tt->test_str[i];
            unsigned format =
                correct_char == c ? COLOR_PAIR_GREEN : COLOR_PAIR_RED;
            TTV_TYPEMODE m =
                c == 'X' ? TTV_TYPEMODE_INSERT : TTV_TYPEMODE_OVERTYPE;

            i = typing_test_view_typechar(tt->view, &c, format, m);
            if (i == last_i) {
                break;
            }
        }
        typing_test_view_render(err, tt->view);
        last_i = i;
    }
    tt_stats_stop(tt->stats);

    double wpm = tt_stats_getwpm(tt->stats, tt->test_str_len);
    *err = ERR_MAKE("WPM: %.2lf", wpm);
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

    if (tt->stats) {
        tt_stats_destoy(&tt->stats);
    }

    free(tt);
    tt = NULL;
    *typing_test = NULL;
}
