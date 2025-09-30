#ifndef TYPING_TEST_H
#define TYPING_TEST_H

#include "err.h"
#include "word_store.h"
#include <ncurses.h>
#include <stdbool.h>

typedef struct Line {
    int start_pos;
    int word_count;
} Line;

typedef struct TypingTest {
    int words_count;
    WINDOW *window;
    Line *lines;
    int lines_count;
    char *words[];

} TypingTest;

void typing_test_init(Err **err, TypingTest **typing_test, WordStore *ws,
                      int word_count);
void typing_test_start(Err **err, TypingTest *typing_test);
void typing_test_destroy(TypingTest **typing_test);

#endif
