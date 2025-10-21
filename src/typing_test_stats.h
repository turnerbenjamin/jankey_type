#ifndef TYPING_TEST_STATS_H
#define TYPING_TEST_STATS_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L // For clock_gettime and CLOCK_MONOTONIC
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "err.h"

typedef struct TypingTestStats TypingTestStats;

void tt_stats_init(Err **err, TypingTestStats **stats);
void tt_stats_start(TypingTestStats *stats);
void tt_stats_stop(TypingTestStats *stats);
void tt_stats_reset(TypingTestStats *stats);
double tt_stats_getwpm(TypingTestStats *stats, size_t chars_typed);
void tt_stats_destoy(TypingTestStats **stats);

#endif
