#include "word_store.h"
#include "err.h"
#include "helpers.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void words_free(char **words, size_t count);
bool word_store_read_line(Err **err, FILE *s, char **lp);

void word_store_init(Err **err, WordStore **ws, const char *dict_path) {
    FILE *s = fopen(dict_path, "r");
    if (!s) {
        *err = ERR_MAKE("Failed to open dict path: %s", dict_path);
        return;
    }

    size_t buff_len = 256;
    char **words = calloc(buff_len, sizeof(*words));
    if (!words) {
        fclose(s);
        *err = ERR_MAKE("Unable to allocate memory for words list");
        return;
    }

    size_t count = 0;
    Err *f_err = NULL;
    while (word_store_read_line(&f_err, s, &words[count])) {
        count++;
        if (count >= buff_len) {
            buff_len += 256;
            char **t = realloc(words, buff_len * sizeof(*words));
            if (!t) {
                f_err = ERR_MAKE("Error resizing buffer");
                break;
            } else {
                words = t;
            }
        }
    }

    if (f_err || ferror(s)) {
        words_free(words, count);
        words = NULL;
        if (ferror(s)) {
            *err = ERR_MAKE("File error");
            err_destroy(&f_err);
        } else {
            *err = f_err;
        }
        fclose(s);
        s = NULL;
        return;
    }

    fclose(s);
    s = NULL;

    // Allocate memory for word store
    size_t word_store_size = sizeof(WordStore) + (count * sizeof(*words));
    WordStore *ts = calloc((size_t)1, word_store_size);
    if (!ts) {
        words_free(words, count);
        words = NULL;
        *err = ERR_MAKE("Unable to allocate memory for word store");
        return;
    }

    // Copy words to word store;
    ts->word_count = count;
    for (size_t i = 0; i < count; i++) {
        ts->words[i] = words[i];
    }
    free(words);
    words = NULL;

    *ws = ts;
    return;
}

void word_store_randn(Err **err, WordStore *ws, size_t buff_size,
                      const char *buff[buff_size]) {
    if (*err) {
        return;
    }
    size_t rand_i = 0;
    for (size_t i = 0; i < buff_size; i++) {
        rand_i = (size_t)rand() % (ws->word_count + 1);
        buff[i] = ws->words[rand_i];
    }
}

size_t word_store_rands(Err **err, WordStore *ws, size_t word_count,
                        char **tgt) {
    if (*err) {
        return 0;
    }

    size_t buff_cap = (size_t)8 * word_count;
    size_t buff_len = 0;
    char *buff = calloc(buff_cap, sizeof(*buff));
    if (!buff) {
        *err = ERR_MAKE("Unable to allocate memory for buffer");
        return 0;
    }

    size_t rand_i = 0;
    for (size_t i = 0; i < word_count; i++) {
        rand_i = (size_t)rand() % (ws->word_count + 1);
        const char *w = ws->words[rand_i];

        size_t word_len = strlen(w);
        size_t space_needed = i ? word_len + 1 : word_len;

        if (buff_len + space_needed >= buff_cap) {
            buff_cap += 256;
            char *t = realloc(buff, buff_cap * sizeof(*t));
            if (!t) {
                free(buff);
                buff = NULL;
                *err = ERR_MAKE("Unable to expand buffer");
                return 0;
            }
            buff = t;
        }

        if (i) {
            buff[buff_len++] = ' ';
        }

        string_copy(&buff[buff_len], word_len, w, word_len);
        buff_len += word_len;
    }

    *tgt = buff;
    return buff_len;
}

void word_store_destroy(WordStore **word_store) {
    if (!word_store) {
        return;
    }
    WordStore *ws = *word_store;
    if (!ws) {
        return;
    }

    for (size_t i = 0; i < ws->word_count; i++) {
        free((ws)->words[i]);
    }
    free(ws);
    ws = NULL;
    *word_store = NULL;
}

bool word_store_read_line(Err **err, FILE *s, char **lp) {
    if (feof(s)) {
        return false;
    }
    if (ferror(s)) {
        *err = ERR_MAKE("Error reading file");
        return false;
    }

    size_t buff_len = 64;
    char *buff = calloc(buff_len, sizeof(*buff));
    if (!buff) {
        *err = ERR_MAKE("Unable to allocate memory for buffer");
        return false;
    }

    int c = 0;
    size_t n = 0;
    while ((c = fgetc(s)) != EOF && c != '\n') {

        // expand buffer if needed
        if (n >= buff_len - 1) {
            buff_len += 64;
            char *t = realloc(buff, buff_len * sizeof(*buff));
            if (!t) {
                free(buff);
                buff = NULL;
                *err = ERR_MAKE("Unable to allocate memory for buffer");
                return false;
            } else {
                buff = t;
            }
        }

        buff[n++] = (char)c;
    }

    if (n == 0 && c == EOF) {
        free(buff);
        return false;
    }

    buff[n] = '\0';
    *lp = buff;
    return true;
}

void words_free(char **words, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free(words[i]);
    }
    free(words);
}
