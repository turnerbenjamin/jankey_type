#ifndef TYPING_TEST_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L // For clock_gettime and CLOCK_MONOTONIC
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#define TYPING_TEST_H

#include "err.h"
#include "word_store.h"
#include <ncurses.h>

typedef struct TypingTest TypingTest;

void typing_test_init(Err **err, TypingTest **typing_test, WordStore *ws,
                      size_t word_count);
void typing_test_start(Err **err, TypingTest *typing_test);
void typing_test_destroy(TypingTest **typing_test);

#endif
