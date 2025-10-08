/**
 * @file yaep_macros.h
 * @brief Error propagation helpers for the C17 modernization effort.
 */

#ifndef YAEP_MACROS_H
#define YAEP_MACROS_H

#define YAEP_CHECK(expr)                 \
    do {                                 \
        int _yaep_err = (expr);          \
        if (_yaep_err != 0) {            \
            return _yaep_err;            \
        }                                \
    } while (0)

#define YAEP_CHECK_ALLOC(ptr)                                    \
    do {                                                         \
        if ((ptr) == NULL) {                                     \
            return yaep_set_error(grammar, YAEP_NO_MEMORY,        \
                                  "allocation failed");         \
        }                                                        \
    } while (0)

#define YAEP_ERROR(code, ...)                                    \
    return yaep_set_error(grammar, (code), __VA_ARGS__)

#endif /* YAEP_MACROS_H */
