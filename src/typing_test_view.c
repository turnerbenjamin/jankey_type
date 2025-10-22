#include "typing_test_view.h"
#include "constants.h"
#include "err.h"
#include "gap_buffer.h"
#include "helpers.h"
#include <ctype.h>
#include <limits.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

typedef struct Line {
    size_t start_i;
    size_t end_i;
} Line;

struct TypingTestView {
    WINDOW *win;
    size_t width;
    GapBuff *buff;
    size_t lines_len;
    size_t lines_cap;
    size_t lines_init_time_spare_cap;
    size_t cursor_i;
    size_t cursor_line_i;
    Line lines[];
};

#define WIN_HEIGHT MAX_TEST_WIN_ROWS

void ttv_calculate_lines(Err **err, TypingTestView *v);

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

    int x = (COLS - (int)(v->width)) / 2;
    int y = (LINES - (int)(WIN_HEIGHT)) / 2;
    v->win = newwin(WIN_HEIGHT, (int)v->width, y, x);
    if (!v->win) {
        *err = ERR_MAKE("Unable to initialise ncurses window");
        typing_test_view_destroy(&v);
        return;
    }

    // Initialise buffer
    gap_buff_init(err, &v->buff, test_str, str_len, COLOR_PAIR_WHITE);
    if (*err) {
        typing_test_view_destroy(&v);
        return;
    }
    gap_buff_mvcursor(err, v->buff, (size_t)0);

    // Initialise line data for the view
    ttv_calculate_lines(err, v);
    if (*err) {
        typing_test_view_destroy(&v);
        return;
    }

    *tgt = v;
}

const char *typing_test_view_charat(TypingTestView *v, size_t i) {
    return &gap_buff_getchar(v->buff, i)->value;
}

size_t typing_test_view_typechar(TypingTestView *v, char *c,
                                 unsigned color_pair_id, TTV_TYPEMODE m) {

    size_t buff_len = gap_buff_getlen(v->buff);
    if (v->cursor_i >= buff_len - 1) {
        return v->cursor_i;
    }

    if (m == TTV_TYPEMODE_OVERTYPE) {
        gap_buff_replacechar(v->buff, c, color_pair_id);
    } else {
        gap_buff_insertchar(v->buff, c, color_pair_id);
    }

    return ++v->cursor_i;
}

size_t typing_test_view_deletechar(TypingTestView *v, char *c) {
    if (v->cursor_i == 0) {
        return v->cursor_i;
    }

    Err *err;
    v->cursor_i--;
    gap_buff_mvcursor(&err, v->buff, v->cursor_i);

    if (c) {
        gap_buff_replacechar(v->buff, c, COLOR_PAIR_WHITE);
    }
    return v->cursor_i;
}

void typing_test_view_render(Err **err, TypingTestView *v) {

    curs_set(1);
    ttv_calculate_lines(err, v);

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
        //
        for (size_t char_count = 0; char_count < line_len; char_count++) {
            const FormattedChar *ch_ptr = gap_buff_nextchar(buff);
            wattron(win, COLOR_PAIR(ch_ptr->colour_pair));
            waddch(win,
                   (unsigned char)(ch_ptr->value == ' ' ? '_' : ch_ptr->value));
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

void ttv_calculate_lines(Err **err, TypingTestView *v) {
    size_t next_char_i = 0;
    size_t curr_line_i = 0;

    v->cursor_line_i = 0;
    Line *curr_line;
    size_t buff_len = gap_buff_getlen(v->buff);
    while (next_char_i < buff_len) {
        // Check sufficient space for line
        if (curr_line_i >= v->lines_cap) {
            v->lines_cap += v->lines_init_time_spare_cap;
            TypingTestView *t =
                realloc(v, sizeof(*t) + (v->lines_cap * sizeof(Line)));
            if (!t) {
                *err = ERR_MAKE("Unable to expand lines capacity");
                return;
            }
            v = t;
        }

        curr_line = &v->lines[curr_line_i];

        // Calculate rough end index
        size_t line_start_i = next_char_i;
        size_t line_end_i =
            MIN_N(next_char_i + MAX_CHARS_PER_LINE - 1, buff_len - 1);

        // Exit if no chars to add
        if (line_end_i == line_start_i) {
            break;
        }

        // Adjust line-end for word-wrapping
        bool non_alpha_char_found = false;
        if (line_end_i != buff_len - 1) {
            while (line_end_i > line_start_i) {
                const char c = gap_buff_getchar(v->buff, line_end_i)->value;
                bool is_alpha = isalpha(c);
                if (is_alpha) {
                    if (non_alpha_char_found) {
                        line_end_i = line_end_i + 1;
                        next_char_i = line_end_i + 1;
                        break;
                    }
                } else {
                    non_alpha_char_found = true;
                }
                line_end_i--;
            }
        }

        if (!non_alpha_char_found) {
            line_end_i =
                MIN_N(next_char_i + MAX_CHARS_PER_LINE - 1, buff_len - 1);
        }

        if (line_start_i <= v->cursor_i && line_end_i >= v->cursor_i) {
            v->cursor_line_i = curr_line_i;
        }

        curr_line->start_i = line_start_i;
        curr_line->end_i = line_end_i;
        curr_line_i++;
        next_char_i = line_end_i + 1;
    }
    v->lines_len = curr_line_i;
}
