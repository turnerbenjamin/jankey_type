#include "err.h"
#include "helpers.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

Err *err_make_va(const char *file, int line, const char *format, va_list args);

Err *err_make(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    Err *err = err_make_va(file, line, msg, args);
    va_end(args);
    return err;
}

Err *err_make_va(const char *file, int line, const char *format, va_list args) {
    if (!format) {
        return NULL;
    }

    Err *err = malloc(sizeof(Err));
    if (!err) {
        return NULL;
    }

    // Copy file name
    if (file) {
        size_t file_len = strlen(file) + 1;
        err->file = malloc(file_len);
        if (!err->file) {
            free(err);
            return NULL;
        }
        string_copy(err->file, file_len, file, file_len);
    } else {
        err->file = NULL;
    }

    err->line = line;

    // Allocate memory for message
    va_list args_copy;
    va_copy(args_copy, args);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    int msg_len = vsnprintf(NULL, (size_t)0, format, args_copy);

    va_end(args_copy);

    if (msg_len < 0) {
        free(err->file);
        free(err);
        return NULL;
    }

    err->msg = malloc((size_t)(msg_len + 1));
    if (!err->msg) {
        free(err->file);
        free(err);
        return NULL;
    }
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    vsnprintf(err->msg, (size_t)(msg_len + 1), format, args);

#pragma clang diagnostic pop
    return err;
}

void err_destroy(Err **err) {
    if (!err || !*err) {
        return;
    }

    Err *e = *err;

    if (e->file) {
        free(e->file);
        e->file = NULL;
    }

    if (e->msg) {
        free(e->msg);
        e->msg = NULL;
    }

    free(e);
    e = NULL;
    *err = NULL;
}

void err_print(const Err *err, FILE *fs) {
    if (!err || !fs) {
        return;
    }

    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    fprintf(fs, "Error");
    if (err->file && err->line) {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
        fprintf(fs, "in %s:%lu", err->file, (unsigned long)err->line);
    }
    if (err->msg) {
        // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
        fprintf(fs, ": %s", err->msg);
    }

    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    fprintf(stderr, "\n");
}
