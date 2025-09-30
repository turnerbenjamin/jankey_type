#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>

#define MAX_N(a, b) ((a) > (b) ? (a) : (b))
#define MIN_N(a, b) ((a) > (b) ? (b) : (a))

size_t string_copy(char *tgt, size_t tgt_size, const char *src,
                   size_t src_size);

#endif
