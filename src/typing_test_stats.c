#include "typing_test_stats.h"
#include "err.h"
#include "helpers.h"
#include "time.h"
#include <threads.h>
#include <unistd.h>

typedef struct TypingTestStats {
    struct timespec start;
    struct timespec end;
    double elapsedSec;
    double wpm;
    double accuracy;
} TypingTestStats;

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
    stats->wpm = 0;
    stats->accuracy = 0;
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

void tt_stats_setwpm(TypingTestStats *s, size_t chars_typed) {
    double words = ((double)chars_typed / (double)5);
    double minutes = s->elapsedSec / 60.0f;
    s->wpm = words / minutes;
}
void tt_stats_setAccuracy(TypingTestStats *s, double a) { s->accuracy = a; }

double tt_stats_getwpm(TypingTestStats *s) { return s->wpm; }
double tt_stats_getAccuracy(TypingTestStats *s) { return s->accuracy; }
double tt_stats_getSecondsElapsed(TypingTestStats *s) { return s->elapsedSec; }

void tt_stats_destoy(TypingTestStats **stats) {
    if (!stats || !*stats) {
        return;
    }
    free(*stats);
    *stats = NULL;
}
