#include "helpers.h"
#include <stdbool.h>

size_t string_copy(char *tgt, size_t tgt_size, const char *src,
                   size_t src_size) {
    size_t i = 0;
    for (; i < src_size && i < tgt_size; i++) {
        char c = src[i];
        if (!c) {
            break;
        }
        tgt[i] = src[i];
    }
    return i;
}
