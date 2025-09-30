#include "word_store.h"
#include <stdio.h>
#include <stdlib.h>

bool word_store_read_line(FILE *s, char **lp);
void words_free(char **words, size_t count);

bool word_store_init(WordStore **ws, char *dict_path) {

    FILE *s = fopen(dict_path, "r");
    if (!s) {
        fputs("Failed to open dict path: ", stderr);
        fputs(dict_path, stderr);
        fputs("\n", stderr);
        return false;
    }

    size_t buff_len = 256;
    char **words = calloc(buff_len, sizeof(*words));
    if (!words) {
        fputs("Unable to allocate memory for words\n", stderr);
        fclose(s);
        return false;
    }

    size_t count = 0;
    bool is_err = false;

    while (word_store_read_line(s, &words[count])) {
        count++;
        if (count >= buff_len) {
            buff_len += 256;
            char **t = realloc(words, buff_len * sizeof(*words));
            if (!t) {
                is_err = true;
                break;
            } else {
                words = t;
            }
        }
    }

    if (is_err || ferror(s)) {
        words_free(words, count);
        words = NULL;
        fclose(s);
        fputs("Error encountered reading dictionary\n", stderr);
        return false;
    }
    fclose(s);
    s = NULL;

    // Allocate memory for word store
    size_t words_size = count * sizeof(*words);
    WordStore *ts = calloc(1, sizeof(*ts) + words_size);
    if (!ts) {
        fputs("Unable to allocate memory for word store\n", stderr);
        words_free(words, count);
        words = NULL;
        return false;
    }

    // Copy words to word store;
    ts->word_count = count;
    for (size_t i = 0; i < count; i++) {
        ts->words[i] = words[i];
    }
    free(words);
    words = NULL;

    *ws = ts;
    return true;
}

void word_store_cpy_rand(WordStore *ws, char **dest, size_t dest_len) {
    size_t rand_i;
    for (size_t i = 0; i < dest_len; i++) {
        rand_i = rand() % (ws->word_count + 1);
        dest[i] = ws->words[rand_i];
    }
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

bool word_store_read_line(FILE *s, char **lp) {
    if (feof(s) || ferror(s)) { // Add this check
        return false;
    }

    size_t buff_len = 64;
    char *buff = calloc(buff_len, sizeof(*buff));
    if (!buff) {
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
};

void words_free(char **words, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free(words[i]);
    }
    free(words);
}
