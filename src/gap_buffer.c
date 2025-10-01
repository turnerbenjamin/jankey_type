#include "gap_buffer.h"
#include "err.h"
#include "helpers.h"
#include <stddef.h>
#include <stdlib.h>

#define MAX_GAP_SIZE (size_t)64
#define MIN_GAP_SIZE (size_t)8

typedef struct GapBuff {
    size_t gap_l;
    size_t gap_r;
    size_t cursor_pos;
    size_t buff_size;
    char buff[];
} GapBuff;

void gap_buff_init(Err **err, GapBuff **gap_buff, const char *init_buff,
                   size_t init_buff_len) {
    // Allocate memory for gap buffer
    size_t buff_size = (MAX_GAP_SIZE + init_buff_len) * sizeof(char);
    GapBuff *gb = ZALLOC(sizeof(*gb) + buff_size));
    if (!gb) {
        *err = ERR_MAKE("Unable to allocate memory for gap_buff");
        return;
    }
    gb->buff_size = buff_size;

    // Initialise cursor and gap at start of buffer
    gb->cursor_pos = 0;
    gb->gap_l = gb->cursor_pos;
    gb->gap_r = MAX_GAP_SIZE - 1;

    // Copy init data to buff
    for (size_t si = 0, ti = gb->gap_r; si < init_buff_len && ti < buff_size;
         si++, ti++) {
        gb->buff[ti] = init_buff[si];
    }

    *gap_buff = gb;
}

void gap_buff_mvcursor(Err **err, GapBuff *gb, size_t i) {
    size_t str_len = gb->buff_size - gb->gap_r + gb->gap_l;
    if (i >= str_len - 1) {
        *err = ERR_MAKE("index out of range");
        return;
    }
    gb->cursor_pos = i;
}

const char *gap_buff_getch(GapBuff *gb, size_t i) {
    size_t gap_len = (gb->gap_r - gb->gap_l) + 1;
    size_t str_len = gb->buff_size - gap_len;

    if (i >= str_len) {
        return NULL;
    }

    size_t char_i = i < gb->gap_l ? i : i + (gap_len - 1);
    gb->cursor_pos++;
    return &gb->buff[char_i];
}

void gap_buff_destroy(GapBuff **gap_buff) {
    if (!gap_buff || !*gap_buff) {
        return;
    }

    free(*gap_buff);
    *gap_buff = NULL;
}
