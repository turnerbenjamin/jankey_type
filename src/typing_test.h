#ifndef TYPING_TEST_H
#define TYPING_TEST_H

#include "word_store.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Line {
    uint64_t start_pos;
    uint64_t word_count;
} Line;

typedef struct TypingTest {
    size_t words_count;
    WINDOW *window;
    Line *lines;
    uint64_t lines_count;
    char *words[];

} TypingTest;

bool typing_test_init(TypingTest **typing_test, WordStore *ws,
                      uint64_t word_count);
void typing_test_start(TypingTest *typing_test);
void typing_test_destroy(TypingTest **typing_test);

#endif
