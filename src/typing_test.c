#include "typing_test.h"
#include "constants.h"
#include "err.h"
#include "helpers.h"
#include <limits.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int typing_test_calc_lines(Err **err, TypingTest *tt);

bool lines_add_word_to_line(Err **err, Line **line, char *word, size_t max_len);
void lines_destroy(Line **lines);

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

    Line *l;
    for (int i = 0; i < typing_test->lines_count; i++) {
        l = &typing_test->lines[i];
        wmove(typing_test->window, i, l->start_pos);
        waddstr(typing_test->window, l->str);
    }

    curs_set(1);
    wrefresh(typing_test->window);

    int current_line_i = 0;
    Line *current_line = &typing_test->lines[current_line_i];
    wmove(typing_test->window, current_line_i, current_line->start_pos);

    char *current_char = current_line->str;
    int input = -1;
    char candidate_char;
    bool is_typing_err = false;

    while (current_line && current_char) {
        input = wgetch(typing_test->window);
        if (input < 0 || input > CHAR_MAX) {
            continue;
        } else {
            candidate_char = (char)input;
        }

        if (is_typing_err) {
            waddch(typing_test->window, (unsigned)*current_char);
            continue;
        }

        // Handle correct input
        if (candidate_char == *current_char) {
            wattron(typing_test->window, COLOR_PAIR(1));
            waddch(typing_test->window, (unsigned)candidate_char);
            wattroff(typing_test->window, COLOR_PAIR(1));
            current_char++;

            // Handle end of line
            if (!*current_char) {
                if (current_line_i < typing_test->lines_count - 1) {
                    current_line = &typing_test->lines[++current_line_i];
                    current_char = current_line->str;
                    wmove(typing_test->window, current_line_i,
                          current_line->start_pos);
                } else {
                    break;
                }
            }
        }
        // Handle incorrect input
        else {
            wattron(typing_test->window, COLOR_PAIR(0));
            waddch(typing_test->window, (unsigned)candidate_char);
            wattroff(typing_test->window, COLOR_PAIR(0));
            is_typing_err = true;
        }
        wrefresh(typing_test->window);
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
        return -1;
    }

    size_t win_max_x = (size_t)MAX_N(0, getmaxx(tt->window));
    size_t max_x =
        (size_t)MAX_N(0, MIN_N(getmaxx(tt->window), MAX_CHARS_PER_LINE));

    if (win_max_x < 1 || max_x < 1) {
        *err = ERR_MAKE("Window dimensions are invalid");
        return -1;
    }

    int lines_buffer_len = 16;
    Line *lines = calloc((size_t)lines_buffer_len, sizeof(*lines));
    if (!lines) {
        *err = ERR_MAKE("Unable to allocate memory for lines");
        return -1;
    }

    int line_count = 0;
    Line *current_line = NULL;
    int current_word_i = 0;
    char *current_word;

    Err *w_line_err = NULL;
    while (current_word_i < tt->words_count) {
        current_line = &lines[line_count];
        current_word = tt->words[current_word_i];

        // If word success fully added, continue
        if (lines_add_word_to_line(&w_line_err, &current_line, current_word,
                                   max_x) &&
            !w_line_err) {
            current_line->word_count++;
            current_word_i++;
            continue;
        }

        // If err thrown, clean up and return
        if (w_line_err) {
            lines_destroy(&lines);
            *err = w_line_err;
            return -1;
        }

        // If line cannot be added ensure memory allocated for new line
        if (current_word_i >= lines_buffer_len - 1) {
            lines_buffer_len += 16;
            Line *t = realloc(lines, (size_t)lines_buffer_len * sizeof(*lines));
            if (!t) {
                lines_destroy(&lines);
                *err = ERR_MAKE("Error expanding lines buffer");
                return -1;
            } else {
                lines = t;
            }
        }
        line_count++;
    }

    // If partly written line, increment line count
    if (current_line && current_line->word_count > 0) {
        line_count++;
    }

    // If no line count throw an error
    if (!line_count) {
        *err = ERR_MAKE("No lines found");
        lines_destroy(&lines);
        return -1;
    }

    // Calculate start positions
    for (int i = 0; i < line_count; i++) {
        Line *l = &lines[i];

        if (l->str_len < 0) {
            *err = ERR_MAKE("Line string length must not be lower than 0");
        }

        size_t start_pos = (win_max_x - (size_t)l->str_len) / (size_t)2;
        if (start_pos > INT_MAX) {
            *err =
                ERR_MAKE("Unable to convert start pos (%ul) to int", start_pos);
            lines_destroy(&lines);
            return -1;
        }
        l->start_pos = (int)start_pos;
    }

    if (line_count > INT_MAX) {
        *err =
            ERR_MAKE("Unable to convert line count (%ul) to int", line_count);
        lines_destroy(&lines);
        return 0;
    }

    tt->lines = lines;
    return (int)line_count;
}

bool lines_add_word_to_line(Err **err, Line **line, char *word,
                            size_t max_len) {
    // Check max len is at least 1 to avoid calloc error
    if (max_len < 1) {
        *err = ERR_MAKE("Max len must be greater than 0");
        return false;
    }

    Line *l = *line;
    if (!l) {
        *err = ERR_MAKE("Line is null");
        return false;
    }

    // If line string is null allocate memory for string
    char *s = l->str;
    if (!s) {
        s = calloc(max_len + 1, sizeof(*s));
        l->str = s;
    }
    if (!l) {
        *err = ERR_MAKE("Unable to allocate memory for line string");
        return false;
    }

    // Check that the word is not greater that the max width for a line
    bool do_prefix_with_space = l->str_len != 0;
    size_t word_len = strlen(word);
    size_t space_required =
        do_prefix_with_space ? word_len + (size_t)1 : word_len;

    if (space_required > max_len) {
        *err = ERR_MAKE("Word found (%s) which exceeds max length (%ul)", word,
                        max_len);
        return false;
    }

    if (l->str_len < 0) {
        *err = ERR_MAKE("String length cannot be less than 0");
        return false;
    }
    // If word cannot fit in line return false; this is not an error condition
    if (space_required > max_len - (size_t)l->str_len) {
        return false;
    }

    // Add prefix if required to line string
    if (do_prefix_with_space) {
        s[l->str_len++] = ' ';
    }

    // Copy word to line string
    for (size_t src_i = (size_t)0; src_i < word_len; src_i++) {
        s[l->str_len++] = word[src_i];
    }

    // Add null char to end of string
    s[l->str_len] = '\0';
    return true;
}

void lines_destroy(Line **lines) {
    if (!lines || !*lines) {
        return;
    }

    Line *ls = *lines;
    if (ls->str) {
        free(ls->str);
        ls->str = NULL;
    }

    free(ls);
    ls = NULL;
    lines = NULL;
}
