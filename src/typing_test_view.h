#ifndef TYPING_TEST_VIEW_H
#define TYPING_TEST_VIEW_H

#include "err.h"
#include "gap_buffer.h"
#include <stdint.h>

/**
 *  View will have a fixed width based on readability. Issues with
 *  resizing are not this view's responsibility
 *
 *  It will be possible to set the test text. This will clear any current
 *  buffer and set it with the test text
 *
 *  It will be posible to add chars to the buffer. When adding chars you can
 *  specify color and mode (insert/replace)
 *
 *  The text will be centered on the screen but the consumer moves through the
 *  buffer not the screen positions. There will be a maximum number of visible
 *  lines. These will be centered on the cursor positon
 *
 *  Strategy: Use
 **/

typedef struct TypingTestView TypingTestView;

void typing_test_view_init(Err **err, TypingTestView **tgt, size_t w,
                           const char **words, size_t words_count);
void typing_test_view_render(Err **err, TypingTestView *view,
                             size_t current_word_i);
void typing_test_view_destroy(TypingTestView **typing_test_view);

#endif
