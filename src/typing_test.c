
#include "typing_test.h"
#include "constants.h"
#include "err.h"
#include "helpers.h"
#include "typing_test_stats.h"
#include "typing_test_view.h"
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
    char *test_str;
    uint64_t test_str_len;
};

void typing_test_init(Err **err, TypingTest **typing_test) {

    TypingTest *t = ZALLOC(sizeof(*t));
    if (!t) {
        *err = ERR_MAKE("Unable to allocate memory for typing test");
        return;
    }

    t->test_str_len = 0;
    t->test_str = NULL;
    t->view = NULL;

    *typing_test = t;
    return;
}

void typing_test_run(Err **err, JankeyState *state, TypingTest *tt,
                     WordStore *ws, size_t word_count, TypingTestStats *stats) {
    if (!tt) {
        *err = ERR_MAKE("Typing test is null");
        return;
    }

    // Initialise new test string
    tt->test_str_len = word_store_rands(err, ws, word_count, &tt->test_str);
    if (*err) {
        return;
    }

    // Initialise/reset test view
    if (!tt->view) {
        typing_test_view_init(err, &tt->view, tt->test_str, tt->test_str_len);
        if (*err) {
            typing_test_destroy(&tt);
            return;
        }
    } else {
        typing_test_view_reset(err, tt->view, tt->test_str, tt->test_str_len);
    }

    // Ensure window clear
    clear();
    refresh();

    // Init stats for new test
    tt_stats_reset(stats);

    // Render initial view of test string
    typing_test_view_render(err, tt->view);

    char c;
    size_t i = 0;
    size_t last_i = 0;
    bool test_started = false;
    double typed_char_count = 0.;
    double correct_char_count = 0.;
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
            if (!test_started) {
                tt_stats_start(stats);
                test_started = true;
            }

            c = (char)ui;
            if (!isprint(c)) {
                continue;
            }
            typed_char_count++;
            char correct_char = tt->test_str[i];
            if (correct_char == c) {
                correct_char_count++;
            }
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
    tt_stats_stop(stats);
    tt_stats_setwpm(stats, tt->test_str_len);
    tt_stats_setAccuracy(stats, (correct_char_count / typed_char_count) * 100.);

    *state = JANKEY_STATE_DISPLAYING_POST_TEST_MODAL;
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
