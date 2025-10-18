#include "typing_test_view.h"
#include "constants.h"
#include "err.h"
#include "gap_buffer.h"
#include "helpers.h"
#include <limits.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Line {
    size_t start_i;
    size_t end_i;
} Line;

struct TypingTestView {
    WINDOW *win;
    size_t width;
    GapBuff *buff;
    size_t buff_len;
    size_t lines_len;
    size_t lines_cap;
    size_t lines_init_time_spare_cap;
    size_t cursor_i;
    size_t cursor_line_i;
    Line lines[];
};
#define WIN_HEIGHT 3
#define MIN_WIN_WIDTH 24

void ttv_initlines(Err **err, TypingTestView **v, size_t line_width,
                   const char *test_str, size_t str_len);
void ttv_update_lines_post_insert(TypingTestView *v);

void typing_test_view_init(Err **err, TypingTestView **tgt,
                           const char *test_str, size_t str_len) {
    if (!err || *err) {
        return;
    }

    // generous line capacity allocated before test with min spare capacity
    // ensured after init to reduce the chance of realloc during the test
    size_t lines_init_time_spare_cap = 128;
    size_t lines_cap = lines_init_time_spare_cap * 2;

    // Allocate memory for the view
    TypingTestView *v = ZALLOC(
        sizeof(*v) + (lines_cap * sizeof((((TypingTestView *)0)->lines[0]))));
    if (!v) {
        *err = ERR_MAKE("Unable to allocate memory for typing test view");
    }
    v->lines_cap = lines_cap;
    v->lines_init_time_spare_cap = lines_init_time_spare_cap;

    // Initialise cursor and cursor line indices
    v->cursor_i = 0;
    v->cursor_line_i = 0;

    // Initialise ncurses window
    v->width = (size_t)MIN_N(MAX_CHARS_PER_LINE, COLS);
    if (v->width < MIN_WIN_WIDTH) {
        *err = ERR_MAKE("Window width (%d) must be at least %d chars", v->width,
                        MIN_WIN_WIDTH);
        typing_test_view_destroy(&v);
        return;
    }

    v->win = newwin(WIN_HEIGHT, (int)v->width, 2, (COLS - (int)v->width) / 2);
    if (!v->win) {
        *err = ERR_MAKE("Unable to initialise ncurses window");
        typing_test_view_destroy(&v);
        return;
    }

    // Initialise buffer
    gap_buff_init(err, &v->buff, test_str, str_len);
    if (*err) {
        typing_test_view_destroy(&v);
        return;
    }
    v->buff_len = str_len;
    gap_buff_mvcursor(err, v->buff, (size_t)0);

    // Initialise line data for the view
    ttv_initlines(err, &v, v->width, test_str, str_len);
    if (*err) {
        typing_test_view_destroy(&v);
        return;
    }

    *tgt = v;
}

size_t typing_test_view_addch(TypingTestView *v, char *c, TTV_TYPEMODE m) {
    if (v->cursor_i >= v->buff_len - 1) {
        return v->cursor_i;
    }

    if (m == TTV_TYPEMODE_OVERTYPE) {
        if (gap_buff_insch(v->buff, c)) {
            ttv_update_lines_post_insert(v);
        };
    } else {
        gap_buff_stch(v->buff, c);
    }

    v->cursor_i++;
    if (v->lines[v->cursor_line_i].end_i < v->cursor_i) {
        v->cursor_line_i++;
    }
    return v->cursor_i;
}

size_t typing_test_view_delch(TypingTestView *v) { return v->cursor_i; }

void typing_test_view_render(Err **err, TypingTestView *v) {

    // Cache values accessed frequently in the loop
    WINDOW *win = v->win;
    GapBuff *buff = v->buff;
    Line *lines = v->lines;
    size_t line_count = v->lines_len;
    size_t width = v->width;
    size_t current_line_number = v->cursor_line_i;

    // Determine lines to render based on the index to be centered
    size_t first_line_i;
    if (line_count > WIN_HEIGHT) {
        if (current_line_number == 0) {
            first_line_i = current_line_number;
        } else if (current_line_number == line_count - 1) {
            first_line_i = line_count - WIN_HEIGHT;
        } else {
            first_line_i = current_line_number - 1;
        }
    } else {
        first_line_i = 0;
    }

    size_t last_line_i = MIN_N(first_line_i + WIN_HEIGHT - 1, line_count - 1);

    // Render each visible line in the window
    for (size_t line_i = first_line_i; line_i <= last_line_i; line_i++) {
        // store current line and move cursor to start of win row
        Line current_line = lines[line_i];
        wmove(win, (int)(line_i - first_line_i), 0);

        // Calculate line length and center offset
        size_t line_len = current_line.end_i - current_line.start_i + 1;
        size_t center_offset = (width > line_len) ? (width - line_len) / 2 : 0;

        // Add leading spaces for centering
        for (size_t col_i = 0; col_i < center_offset; col_i++) {
            waddch(win, ' ');
        }

        // Position cursor in gap buffer to start of line
        gap_buff_mvcursor(err, buff, current_line.start_i);
        if (*err) {
            return;
        }

        // Render line from buffer
        size_t chars_in_line = current_line.end_i - current_line.start_i + 1;
        for (size_t char_count = 0; char_count < chars_in_line; char_count++) {
            const char *ch_ptr = gap_buff_nextch(buff);
            if (!ch_ptr)
                break;
            waddch(win, (unsigned char)*ch_ptr);
        }
        wclrtoeol(win);
    }

    // Clear any remaining rows in window
    for (int row = (int)(last_line_i - first_line_i + 1); row < WIN_HEIGHT;
         row++) {
        wmove(win, row, 0);
        wclrtoeol(win);
    }

    // Sync buff cursor with view cursor
    gap_buff_mvcursor(err, buff, v->cursor_i);

    // Sync window cursor with view cursor
    Line focussed_line = v->lines[v->cursor_line_i];
    size_t line_len = focussed_line.end_i - focussed_line.start_i + 1;
    size_t center_offset = (width > line_len) ? (width - line_len) / 2 : 0;
    size_t c_x = center_offset + v->cursor_i - focussed_line.start_i;

    int row_offset = (int)v->cursor_line_i - (int)first_line_i;
    int c_y = row_offset;
    wmove(v->win, c_y, (int)c_x);
    wrefresh(win);
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

    free(v);
    v = NULL;
    *tgt = NULL;
}

void ttv_initlines(Err **err, TypingTestView **typing_test_view,
                   size_t line_width, const char *test_str, size_t str_len) {
    TypingTestView *v = *typing_test_view;
    v->lines_len = 0;
    Line *current_line = &v->lines[v->lines_len];

    size_t current_word_len = 0;
    size_t start_of_current_word_i = 0;
    size_t current_line_len;

    char c;
    for (size_t i = 0; i < str_len; i++) {
        c = test_str[i];

        // Handle end of word
        if (c == ' ') {
            if (current_word_len) {
                current_line->end_i = i;
            }
            current_word_len = 0;
            continue;
        }

        // Increment current word len and set start i for current word
        if (!current_word_len) {
            start_of_current_word_i = i;
        }
        current_word_len++;

        // Throw error if a word exceeds the width of the line
        if (current_word_len + 1 > line_width) {
            *err = ERR_MAKE("A word has been found that exceeds the width of "
                            "the window: %s",
                            &test_str[start_of_current_word_i]);
            *typing_test_view = v;
            return;
        }

        // If current line cannot fit the current word, increment the
        // current line and set it's initial start position
        current_line_len = current_line->end_i - current_line->start_i + 1;
        if (current_line_len + current_word_len > line_width) {
            v->lines_len++;
            // Ensure space to add new line
            if (v->lines_len >= v->lines_cap - v->lines_init_time_spare_cap) {
                v->lines_cap += v->lines_init_time_spare_cap;
                TypingTestView *t =
                    realloc(v, sizeof(*t) + (v->lines_cap * sizeof(Line)));
                if (!t) {
                    *err = ERR_MAKE("Unable to expand lines capacity");
                    *typing_test_view = v;
                    return;
                }
                v = t;
            }

            // Initialise new line
            current_line = &v->lines[v->lines_len];

            current_line->start_i = start_of_current_word_i;
            current_line->end_i = current_line->start_i;
        }
    }
    // Complete final line
    if (current_word_len) {
        current_line->end_i = str_len - 1;
        v->lines_len++;
    }
    *typing_test_view = v;
}

void ttv_update_lines_post_insert(TypingTestView *v) {
    for (size_t line_i = v->cursor_line_i; line_i < v->lines_len; line_i++) {
    }
}
