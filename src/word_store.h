#ifndef WORD_STORE_H
#define WORD_STORE_H

#include "err.h"
#include <err.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct WordStore {
    uint64_t word_count;
    char *words[];
} WordStore;

void word_store_init(Err **err, WordStore **ws, const char *dict_path);
void word_store_cpy_rand(WordStore *ws, char **dest, int dest_len);
void word_store_destroy(WordStore **ws);

#endif
