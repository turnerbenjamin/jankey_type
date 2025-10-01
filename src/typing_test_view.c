#include "typing_test_view.h"
#include "err.h"
#include "gap_buffer.h"
#include "helpers.h"
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

/*
 * Updates to the view buffer occur on words. TypingTestWord represents a word
 * in the view and includes the original test word, it's starting index in the
 * buffer and it's typed length.
 */
typedef struct TypingTestWord {
    const char *word;
    size_t len;
    size_t typed_len;
    size_t line_number;
    size_t buff_i;
} TypingTestWord;

/*
 * Lines need to be centered on the screen. In addition, it is only necessary
 * to render 3 lines, though the buffer may
 */
typedef struct TypingTestViewLine {
    size_t len;
    size_t start_pos;
    size_t start_i;
    size_t end_i;
} TypingTestViewLine;

struct TypingTestView {
    size_t view_width;
    WINDOW *win;
    GapBuff *test_buff;
    size_t *words_count;
    char *concatenated_words;
    size_t concatenated_words_len;
    size_t lines_count;
    TypingTestViewLine *line_data;
    TypingTestWord words[];
};

void typing_test_view_init(Err **err, TypingTestView **tgt, size_t width,
                           const char **words, size_t words_count) {
    if (*err)
        return;

    if (!words_count) {
        *err = ERR_MAKE("Words count must be greater than 0");
        return;
    }

    // Allocate memory for typing test view;
    size_t words_size = words_count * sizeof(TypingTestWord);
    TypingTestView *v = calloc((size_t)1, sizeof(TypingTestView) + words_size);
    if (!v) {
        *err = ERR_MAKE("Unable to allocate memory for typing test view");
        return;
    }

    if (width > (size_t)INT_MAX) {
        *err = ERR_MAKE("width cannot be greater than %d", INT_MAX);
    }
    v->view_width = width;

    // init ncurses window
    v->win = newwin(10, (int)width, 2, (COLS - (int)width) / 2);
    if (!v->win) {
        typing_test_view_destroy(&v);
        *err = ERR_MAKE("Failed to create window");
        return;
    }

    // Allocate memory for concatenated words
    size_t concatenated_words_buff_len = (size_t)8 * words_count;
    v->concatenated_words_len = 0;
    v->concatenated_words = calloc(concatenated_words_buff_len, sizeof(char));
    if (!v->concatenated_words) {
        *err = ERR_MAKE("Unable to allocate memory for buffer");
        typing_test_view_destroy(&v);
        return;
    }

    // Allocate memory for line data
    size_t lines_buff_len = 16;
    v->line_data = calloc(lines_buff_len, sizeof(*v->line_data));
    if (!v->line_data) {
        *err = ERR_MAKE("Unable to allocate memory for line data");
        typing_test_view_destroy(&v);
        return;
    }

    // Process words
    v->lines_count = 0;
    TypingTestViewLine *current_line = &v->line_data[v->lines_count];
    current_line->start_i = 0;
    size_t running_buff_i = 0;

    for (size_t i = 0; i < words_count; i++) {
        const char *raw_word = words[i];
        TypingTestWord *w = &v->words[i];
        w->word = raw_word;
        w->len = strlen(w->word);
        w->buff_i = running_buff_i;
        w->typed_len = 0;

        // Calculate space required. Words after the first are prefixed with a
        // space
        size_t space_required = i ? w->len + 1 : w->len;

        // Check current line for space. Increment if new line needed
        if (current_line->len + space_required > v->view_width) {
            // Complete current line
            current_line->start_pos =
                (v->view_width - current_line->len) / (size_t)2;
            current_line->end_i = running_buff_i;

            // Update line index and allocate more space if required
            v->lines_count++;
            if (v->lines_count >= lines_buff_len - 1) {
                lines_buff_len += 16;
                TypingTestViewLine *t =
                    realloc(v->line_data, lines_buff_len * sizeof(*t));
                if (!t) {
                    *err = ERR_MAKE("Unable to allocate memory for line data");
                    typing_test_view_destroy(&v);
                    return;
                } else {
                    v->line_data = t;
                }
            }
            current_line = &v->line_data[v->lines_count];
            current_line->start_i = running_buff_i + 1;
        }

        // Check space in concatenated string to copy the word and expand as
        // necessary
        if (running_buff_i + space_required > concatenated_words_buff_len) {
            concatenated_words_buff_len += 256;
            char *t = realloc(v->concatenated_words,
                              concatenated_words_buff_len *
                                  sizeof(*v->concatenated_words));
            if (!t) {
                *err = ERR_MAKE(
                    "Unable to allocate memory for concatenated words");
                typing_test_view_destroy(&v);
                return;
            } else {
                v->concatenated_words = t;
            }
        }

        // Add word, plus space where required to concatenated string
        if (i) {
            v->concatenated_words[running_buff_i++] = ' ';
        }

        size_t remaining_space = concatenated_words_buff_len - running_buff_i;
        size_t copied = string_copy(&v->concatenated_words[running_buff_i],
                                    remaining_space, raw_word, w->len);
        running_buff_i += copied;
        v->concatenated_words_len = running_buff_i;

        // Update length variables
        current_line->len += space_required;
        w->line_number = v->lines_count;
    }
    // finalise the last line
    if (current_line->len) {
        current_line->start_pos =
            (v->view_width - current_line->len) / (size_t)2;
        v->lines_count++;
        current_line->end_i = running_buff_i;
    }

    // Initialise test buffer
    gap_buff_init(err, &v->test_buff, v->concatenated_words,
                  v->concatenated_words_len);
    if (*err) {
        return;
    }

    *tgt = v;
}

void typing_test_view_render(Err **err, TypingTestView *v,
                             size_t current_word_i) {
    if (*err) {
        return;
    }

    // Render 3 lines around the current word
    TypingTestWord current_word = v->words[current_word_i];
    size_t first_line_i = current_word.line_number > 0
                              ? current_word.line_number - 1
                              : current_word.line_number;
    size_t last_line_i = first_line_i + 2;

    if (last_line_i > INT_MAX) {
        *err = ERR_MAKE("Line number cannot exceed int max");
        return;
    }

    for (size_t line_i = first_line_i; line_i <= last_line_i; line_i++) {
        TypingTestViewLine l = v->line_data[line_i];
        if (l.start_pos > INT_MAX) {
            *err = ERR_MAKE("Line number cannot exceed int max");
            return;
        }

        wmove(v->win, (int)(line_i - first_line_i), (int)l.start_pos);
        for (size_t char_i = l.start_i; char_i <= l.end_i; char_i++) {
            waddch(v->win, (unsigned)*gap_buff_getch(v->test_buff, char_i));
        }
    }
    wrefresh(v->win);
}

void typing_test_view_destroy(TypingTestView **tgt) {
    if (!tgt || !*tgt) {
        return;
    }

    TypingTestView *v = *tgt;

    if (v->win) {
        delwin(v->win);
        v->win = NULL;
    }

    if (v->test_buff) {
        gap_buff_destroy(&v->test_buff);
    }

    if (v->concatenated_words) {
        free(v->concatenated_words);
        v->concatenated_words = NULL;
    }

    if (v->line_data) {
        free(v->line_data);
        v->line_data = NULL;
    }

    free(*tgt);
    *tgt = NULL;
}
