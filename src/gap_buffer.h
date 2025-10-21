#ifndef GAP_BUFFER_H
#define GAP_BUFFER_H

#include "err.h"
#include <stdint.h>

typedef struct GapBuff GapBuff;
void gap_buff_init(Err **err, GapBuff **gap_buff, const char *init_buff,
                   size_t init_buff_len);
void gap_buff_mvcursor(Err **err, GapBuff *gap_buff, size_t i);
const char *gap_buff_nextchar(GapBuff *gb);
const char *gap_buff_getchar(GapBuff *gb, size_t i);

bool gap_buff_replacechar(GapBuff *buffer, char *c);
bool gap_buff_insertchar(GapBuff *buffer, char *c);

size_t gap_buff_getlen(GapBuff *buffer);

void gap_buff_destroy(GapBuff **gap_buff);

#endif
