#define _POSIX_C_SOURCE 199309L
#include "typing_test.h"
#include "constants.h"
#include "err.h"
#include "helpers.h"
#include "typing_test_stats.h"
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
    bool test_started;
    double typed_char_count;
    double correct_char_count;
};

size_t tt_update(TypingTest *tt, TypingTestStats *stats, size_t index,
                 int input);

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

    // Initialise test data
    tt->test_started = false;
    tt->typed_char_count = 0.;
    tt->correct_char_count = 0.;

    // Ensure window clear
    clear();
    refresh();

    // Init stats for new test
    tt_stats_reset(stats);

    // Render initial view of test string
    typing_test_view_render(err, tt->view);

    // Set delay to target 60 fps refresh rate
    struct timespec delay = {.tv_sec = 0, .tv_nsec = 16666667};

    // Set non-blocking input to allow polling of all queued input each cycle
    timeout(0);

    size_t i = 0;
    size_t last_index = 0;
    bool do_continue = true;
    while (do_continue) {
        int ui;
        bool input_received = false;
        while ((ui = getch()) >= 0) {
            input_received = true;
            i = tt_update(tt, stats, i, ui);
            if (i == last_index) {
                do_continue = false;
                tt_stats_stop(stats);
                break;
            }
            last_index = i;
        }
        if (input_received) {
            typing_test_view_render(err, tt->view);
        }
        if (do_continue) {
            nanosleep(&delay, NULL);
        }
    }
    tt_stats_setwpm(stats, tt->test_str_len);
    tt_stats_setAccuracy(
        stats, (tt->correct_char_count / tt->typed_char_count) * 100.);

    *state = JANKEY_STATE_DISPLAYING_POST_TEST_MODAL;
    timeout(-1);
}

size_t tt_update(TypingTest *tt, TypingTestStats *stats, size_t index,
                 int input) {
    if (input == KEY_BACKSPACE || input == 127 || input == 8) {
        if (index > 0) {
            char correct_char = tt->test_str[index - 1];
            index = typing_test_view_deletechar(tt->view, &correct_char);
        }
    } else {
        if (!tt->test_started) {
            tt_stats_start(stats);
            tt->test_started = true;
        }

        char c = (char)input;
        tt->typed_char_count++;
        char correct_char = tt->test_str[index];
        if (correct_char == c) {
            tt->correct_char_count++;
        }
        unsigned format = correct_char == c ? COLOR_PAIR_GREEN : COLOR_PAIR_RED;
        TTV_TYPEMODE m = c == 'X' ? TTV_TYPEMODE_INSERT : TTV_TYPEMODE_OVERTYPE;

        index = typing_test_view_typechar(tt->view, &c, format, m);
    }
    return index;
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
