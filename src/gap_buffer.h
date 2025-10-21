#ifndef GAP_BUFFER_H
#define GAP_BUFFER_H

#include "err.h"
#include <stdint.h>

typedef struct GapBuff GapBuff;
typedef struct FormattedChar {
    char value;
    unsigned colour_pair;
} FormattedChar;

void gap_buff_init(Err **err, GapBuff **gap_buff, const char *init_buff,
                   size_t init_buff_len, unsigned defaultFormat);
void gap_buff_mvcursor(Err **err, GapBuff *gap_buff, size_t i);
const FormattedChar *gap_buff_nextchar(GapBuff *gb);
const FormattedChar *gap_buff_getchar(GapBuff *gb, size_t i);

bool gap_buff_replacechar(GapBuff *buffer, const char *c,
                          unsigned color_pair_id);
bool gap_buff_insertchar(GapBuff *buffer, const char *c,
                         unsigned color_pair_id);

size_t gap_buff_getlen(GapBuff *buffer);

void gap_buff_destroy(GapBuff **gap_buff);

#endif
