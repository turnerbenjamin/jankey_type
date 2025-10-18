#ifndef TYPING_TEST_VIEW_H
#define TYPING_TEST_VIEW_H

#include "err.h"
#include <stdint.h>

typedef struct TypingTestView TypingTestView;

typedef enum TTV_TYPEMODE {
    TTV_TYPEMODE_INSERT,
    TTV_TYPEMODE_OVERTYPE
} TTV_TYPEMODE;

void typing_test_view_init(Err **err, TypingTestView **view_ptr,
                           const char *test_str, size_t test_str_len);

size_t typing_test_view_addch(TypingTestView *view, char *c, TTV_TYPEMODE mode);

size_t typing_test_view_delch(TypingTestView *view);

void typing_test_view_render(Err **err, TypingTestView *view);

void typing_test_view_destroy(TypingTestView **view);

#endif
