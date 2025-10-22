#include "gap_buffer.h"
#include "err.h"
#include "helpers.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

static size_t min_gap_len = 128;

typedef struct GapBuff {
    size_t gap_l;
    size_t gap_r;
    size_t cursor_pos;
    size_t buff_len;
    FormattedChar buff[];
} GapBuff;

size_t gb_getbuffi(GapBuff *gb, size_t i);
void gb_mvgapaftercursor(GapBuff *gb);
void gb_setup(Err **err, GapBuff *gb, const char *init_buff,
              size_t init_buff_len, unsigned default_format);

void gap_buff_init(Err **err, GapBuff **gap_buff, const char *seed_buff,
                   size_t seed_buff_len, unsigned default_format) {

    // Buffer is reussed for each test, generous buffer size allocated to
    // minimise reallocations
    size_t required_len = seed_buff_len + min_gap_len;
    size_t initial_buff_len = required_len * 2;
    size_t buff_size = initial_buff_len * sizeof(((GapBuff *)0)->buff[0]);

    GapBuff *gb = ZALLOC(sizeof(*gb) + buff_size);
    if (!gb) {
        *err = ERR_MAKE("Unable to allocate memory for gap_buff");
        return;
    }
    gb->buff_len = initial_buff_len;

    gb_setup(err, gb, seed_buff, seed_buff_len, default_format);
    *gap_buff = gb;
}

void gap_buff_reset(Err **err, GapBuff *gb, const char *seed_buff,
                    size_t seed_buff_len, unsigned default_format) {
    gb_setup(err, gb, seed_buff, seed_buff_len, default_format);
}

void gb_setup(Err **err, GapBuff *gb, const char *init_buff,
              size_t init_buff_len, unsigned default_format) {
    size_t required_len = init_buff_len + min_gap_len;
    if (gb->buff_len < required_len) {
        gb->buff_len = required_len * 2;
        GapBuff *t =
            realloc(gb, sizeof(*t) + (gb->buff_len * sizeof(t->buff[0])));
        if (!t) {
            *err = ERR_MAKE("Unable to reallocate gap buffer");
            gap_buff_destroy(&gb);
            return;
        } else {
            gb = t;
            t = NULL;
        }
    }

    // Init cursor position and gap at the end of the buffer
    gb->cursor_pos = init_buff_len - 1;
    gb->gap_l = init_buff_len;
    gb->gap_r = gb->buff_len - 1;

    for (size_t i = 0; i < init_buff_len; i++) {
        FormattedChar c = {.value = init_buff[i],
                           .colour_pair = default_format};
        gb->buff[i] = c;
    }
}

size_t gap_buff_getlen(GapBuff *gb) {
    size_t gap_len = gb->gap_r - gb->gap_l + 1;
    return gb->buff_len - gap_len;
}

// Move cursor position in text
void gap_buff_mvcursor(Err **err, GapBuff *gb, size_t i) {
    size_t str_len = gap_buff_getlen(gb);
    if (i > str_len - 1) {
        *err = ERR_MAKE("index:%lu out of range:0-%lu", i, str_len - 1);
        return;
    }
    gb->cursor_pos = i;
}

// Overtype char
bool gap_buff_replacechar(GapBuff *gb, const char *c, unsigned color_pair_id) {
    size_t logical_cursor_pos = gb_getbuffi(gb, gb->cursor_pos);
    gb->buff[logical_cursor_pos] =
        (FormattedChar){.value = *c, .colour_pair = color_pair_id};
    return true;
}

// Insert char
bool gap_buff_insertchar(GapBuff *gb, const char *c, unsigned color_pair_id) {
    size_t gap_len = gb->gap_r - gb->gap_l + 1;
    if (!gap_len) {
        return false;
    }
    gb_mvgapaftercursor(gb);
    gb->buff[gb->gap_l++] =
        (FormattedChar){.value = *c, .colour_pair = color_pair_id};
    return true;
}

const FormattedChar *gap_buff_nextchar(GapBuff *gb) {
    size_t buff_i = gb_getbuffi(gb, gb->cursor_pos++);
    return &gb->buff[buff_i];
}

const FormattedChar *gap_buff_getchar(GapBuff *gb, size_t i) {
    size_t buff_i = gb_getbuffi(gb, i);
    return &gb->buff[buff_i];
}

void gap_buff_destroy(GapBuff **gap_buff) {
    if (!gap_buff || !*gap_buff) {
        return;
    }

    free(*gap_buff);
    *gap_buff = NULL;
}

size_t gb_getbuffi(GapBuff *gb, size_t i) {
    size_t gap_len = gb->gap_r - gb->gap_l + 1;
    if (i < gb->gap_l) {
        return i;
    } else {
        return i + gap_len;
    }
}

void gb_mvgapaftercursor(GapBuff *gb) {
    size_t cursor_pos = gb->cursor_pos;
    size_t gap_l_tgt = cursor_pos + 1;

    // Return if already synced
    if (gb->gap_l == gap_l_tgt) {
        return;
    }

    bool mv_left = gap_l_tgt < gb->gap_l;

    // Move cursor in relevant direction
    while (gb->gap_l != gap_l_tgt) {
        if (mv_left) {
            gb->gap_l--;
            gb->buff[gb->gap_r] = gb->buff[gb->gap_l];
            gb->gap_r--;
        } else {
            gb->gap_l++;
            gb->buff[gb->gap_l] = gb->buff[gb->gap_r];
            gb->gap_r++;
        }
    }
}
