#include "typing_test.h"
#include "config.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t typing_test_calc_lines(TypingTest *tt);

bool typing_test_init(TypingTest **typing_test, WordStore *ws,
                      uint64_t word_count) {
    TypingTest *t = calloc(
        1, sizeof(*t) + (sizeof(((TypingTest *)0)->words[0]) * word_count));
    if (!t) {
        fputs("Unable to allocate memory for typing test\n", stderr);
        return false;
    }

    t->words_count = word_count;
    word_store_cpy_rand(ws, t->words, t->words_count);
    t->window = newwin(LINES - 2, COLS, 2, 0);
    if (!t->window) {
        fputs("Failed to create window\n", stderr);
        free(t);
        return false;
    }

    t->lines_count = typing_test_calc_lines(t);
    if (!t->lines_count) {
        typing_test_destroy(&t);
        return false;
    }

    *typing_test = t;
    return true;
}

void typing_test_start(TypingTest *typing_test) {
    wclear(typing_test->window);
    wmove(typing_test->window, 0, 0);

    for (size_t word_i = 0, line_i = 0, words_in_line = 0;
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

uint64_t typing_test_calc_lines(TypingTest *tt) {
    int win_max_x = getmaxx(tt->window);
    int max_x = win_max_x < MAX_CHARS_PER_LINE ? win_max_x : MAX_CHARS_PER_LINE;
    int max_y = getmaxy(tt->window);

    uint64_t current_line = 0;
    uint64_t current_line_len = 0;
    uint64_t current_word = 0;

    Line *ls = calloc(max_y, sizeof(*ls));
    if (!ls) {
        fputs("Unable to allocate memory for lines\n", stderr);
        return 0;
    }

    while (current_word < tt->words_count) {
        uint64_t word_len = strlen(tt->words[current_word]);
        if (word_len > max_x) {
            fputs("Word found that exceeds max width of testing window\n",
                  stderr);
            free(ls);
            return 0;
        }

        uint64_t needed_space = word_len + (current_line_len > 0 ? 1 : 0);
        if (current_line_len + needed_space > max_x) {
            ls[current_line].start_pos = (win_max_x - current_line_len) / 2;
            current_line++;
            if (current_line >= max_y) {
                fputs("Too few window rows to display test\n", stderr);
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
        ls[current_line].start_pos = (win_max_x - current_line_len) / 2;
        current_line++;
    }

    if (!current_line) {
        fputs("No lines found\n", stderr);
        free(ls);
        return 0;
    }
    tt->lines = ls;
    return current_line;
}
