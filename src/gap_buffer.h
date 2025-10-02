#ifndef GAP_BUFFER_H
#define GAP_BUFFER_H

#include "err.h"
#include <stdint.h>

typedef struct GapBuff GapBuff;
void gap_buff_init(Err **err, GapBuff **gap_buff, const char *init_buff,
                   size_t init_buff_len);
void gap_buff_mvcursor(Err **err, GapBuff *gap_buff, size_t i);
const char *gap_buff_nextch(GapBuff *gb);
const char *gap_buff_getch(GapBuff *gb, size_t i);
void gap_buff_destroy(GapBuff **gap_buff);

#endif
