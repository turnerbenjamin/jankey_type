#include "typing_test_stats.h"
#include "err.h"
#include "helpers.h"
#include <threads.h>
#include <time.h>
#include <unistd.h>

struct TypingTestStats {
    struct timespec start;
    struct timespec end;
    double elapsedSec;
};

void tt_stats_init(Err **err, TypingTestStats **stats) {

    TypingTestStats *s = ZALLOC(sizeof(*s));
    if (!s) {
        *err = ERR_MAKE("Unable to allocate memory for typing stats");
    }
    tt_stats_reset(s);
    *stats = s;
}

void tt_stats_reset(TypingTestStats *stats) {
    stats->start.tv_sec = 0;
    stats->start.tv_nsec = 0;

    stats->end.tv_sec = 0;
    stats->end.tv_nsec = 0;

    stats->elapsedSec = 0.;
}

void tt_stats_start(TypingTestStats *stats) {
    clock_gettime(CLOCK_MONOTONIC, &stats->start);
}

void tt_stats_stop(TypingTestStats *stats) {
    clock_gettime(CLOCK_MONOTONIC, &stats->end);
    stats->elapsedSec +=
        (double)(stats->end.tv_sec - stats->start.tv_sec) +
        (double)(stats->end.tv_nsec - stats->start.tv_nsec) / 1000000000.0f;
}

double tt_stats_getwpm(TypingTestStats *stats, size_t chars_typed) {
    double words = ((double)chars_typed / (double)5);
    double minutes = stats->elapsedSec / 60.0f;
    return words / minutes;
}
void tt_stats_destoy(TypingTestStats **stats) {
    if (!stats || !*stats) {
        return;
    }
    free(*stats);
    *stats = NULL;
}
