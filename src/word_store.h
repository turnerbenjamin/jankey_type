#ifndef WORD_STORE_H
#define WORD_STORE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct WordStore {
    uint64_t word_count;
    char *words[];
} WordStore;

bool word_store_init(WordStore **ws, char *dict_path);
void word_store_cpy_rand(WordStore *ws, char **dest, size_t dest_len);
void word_store_destroy(WordStore **ws);

#endif
