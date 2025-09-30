#include "typing_test.h"
#include "config.h"
#include "err.h"
#include <limits.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int typing_test_calc_lines(Err **err, TypingTest *tt);

void typing_test_init(Err **err, TypingTest **typing_test, WordStore *ws,
                      int word_count) {
    size_t tt_size = (sizeof(TypingTest) +
                      sizeof(((TypingTest *)0)->words[0]) * (size_t)word_count);
    TypingTest *t = calloc((size_t)1, tt_size);
    if (!t) {
        *err = ERR_MAKE("Unable to allocate memory for typing test");
        return;
    }

    t->words_count = word_count;
    word_store_cpy_rand(ws, t->words, t->words_count);

    t->window = newwin(LINES - 2, COLS, 2, 0);
    if (!t->window) {
        free(t);
        *err = ERR_MAKE("Failed to create window");
        return;
    }

    t->lines_count = typing_test_calc_lines(err, t);
    if (*err) {
        free(t);
        return;
    }
    if (!t->lines_count) {
        typing_test_destroy(&t);
        *err = ERR_MAKE("Calc lines returned 0 lines");
        return;
    }

    *typing_test = t;
    return;
}

void typing_test_start(Err **err, TypingTest *typing_test) {
    if (!typing_test) {
        *err = ERR_MAKE("Typing test is null");
        return;
    }

    wclear(typing_test->window);
    wmove(typing_test->window, 0, 0);

    for (int word_i = 0, line_i = 0, words_in_line = 0;
         word_i < typing_test->words_count && line_i < typing_test->lines_count;
         word_i++, words_in_line++) {
        if (words_in_line >= typing_test->lines[line_i].word_count) {
            words_in_line = 0;
            line_i++;
        }
        if (!words_in_line) {
            wmove(typing_test->window, line_i,
                  typing_test->lines[line_i].start_pos);
        }
        waddstr(typing_test->window, typing_test->words[word_i]);
        if (words_in_line < typing_test->lines[line_i].word_count - 1) {
            waddstr(typing_test->window, " ");
        }
    }
    wrefresh(typing_test->window);
    refresh();
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

    if (tt->lines) {
        free(tt->lines);
        tt->lines = NULL;
    }

    if (tt->window) {
        delwin(tt->window);
        tt->window = NULL;
    }

    free(tt);
    tt = NULL;
    *typing_test = NULL;
}

int typing_test_calc_lines(Err **err, TypingTest *tt) {
    if (!tt) {
        *err = ERR_MAKE("Typing test is null");
        return 0;
    }

    size_t win_max_x = (size_t)MAX_N(0, getmaxx(tt->window));
    size_t max_x =
        (size_t)MAX_N(0, MIN_N(getmaxx(tt->window), MAX_CHARS_PER_LINE));
    size_t max_y = (size_t)MAX_N(0, getmaxy(tt->window));

    if (win_max_x < 1 || max_x < 1 || max_y < 1) {
        *err = ERR_MAKE("Window dimensions are invalid");
        return 0;
    }

    size_t current_line = 0;
    size_t current_line_len = 0;
    int current_word = 0;

    Line *ls = calloc(max_y, sizeof(*ls));
    if (!ls) {
        *err = ERR_MAKE("Unable to alocate memory for lines");
        return 0;
    }

    while (current_word < tt->words_count) {
        unsigned long word_len = strlen(tt->words[current_word]);
        if (word_len > max_x) {
            *err = ERR_MAKE(
                "Word found that exceeds max width of testing window: %ul",
                max_x);
            free(ls);
            return 0;
        }

        uint64_t needed_space = word_len + (current_line_len > 0 ? 1 : 0);
        if (current_line_len + needed_space > max_x) {
            size_t start_pos = (win_max_x - current_line_len) / 2;
            if (start_pos > INT_MAX) {
                *err = ERR_MAKE("Unable to convert start pos (%ul) to int",
                                start_pos);
                free(ls);
                return 0;
            }
            ls[current_line].start_pos = (int)start_pos;
            current_line++;
            if (current_line >= max_y) {
                *err = ERR_MAKE("Too few window rows (%ld) to display test",
                                max_y);
                free(ls);
                return 0;
            }

            current_line_len = 0;
            needed_space = word_len;
        }

        ls[current_line].word_count++;
        current_line_len += needed_space;
        current_word++;
    }

    if (current_line_len) {
        size_t start_pos = (win_max_x - current_line_len) / 2;
        if (start_pos > INT_MAX) {
            *err =
                ERR_MAKE("Unable to convert start pos (%ul) to int", start_pos);
            free(ls);
            return 0;
        }
        ls[current_line].start_pos = (int)start_pos;
        current_line++;
    }

    if (!current_line) {
        *err = ERR_MAKE("No lines found");
        free(ls);
        return 0;
    }
    tt->lines = ls;

    if (current_line > INT_MAX) {
        *err = ERR_MAKE("Unable to convert current line (%ul) to int",
                        current_line);
        free(ls);
        return 0;
    }
    return (int)current_line;
}
