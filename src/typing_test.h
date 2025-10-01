#ifndef TYPING_TEST_H
#define TYPING_TEST_H

#include "err.h"
#include "typing_test_view.h"
#include "word_store.h"
#include <ncurses.h>

typedef struct TypingTest {
    TypingTestView *view;
    size_t words_count;
    const char *words[];
} TypingTest;

void typing_test_init(Err **err, TypingTest **typing_test, WordStore *ws,
                      size_t word_count);
void typing_test_start(Err **err, TypingTest *typing_test);
void typing_test_destroy(TypingTest **typing_test);

#endif
