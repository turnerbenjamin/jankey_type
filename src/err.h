#ifndef ERR_H
#define ERR_H

#include <stdint.h>
#include <stdio.h>

typedef struct Err {
    char *msg;
    int line;
    char *file;
} Err;

#define RESET_ERR(error)                                                       \
    {                                                                          \
        err_destroy(&(error));                                                 \
        (error) = NULL;                                                        \
    }

#define ERR_MAKE(msg, ...) err_make(__FILE__, __LINE__, (msg), ##__VA_ARGS__)

Err *err_make(const char *file, int line, const char *msg, ...);
void err_print(const Err *err, FILE *fs);
void err_destroy(Err **err);

#endif
