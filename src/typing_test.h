#ifndef TYPING_TEST_H
#define TYPING_TEST_H

#include "constants.h"
#include "err.h"
#include "word_store.h"
#include <ncurses.h>

typedef struct TypingTest TypingTest;

void typing_test_init(Err **err, TypingTest **typing_test, WordStore *ws,
                      size_t word_count);
void typing_test_run(Err **err, JankeyState *state, TypingTest *tt);
void typing_test_destroy(TypingTest **typing_test);

#endif
