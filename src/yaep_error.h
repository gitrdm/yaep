/**
 * @file yaep_error.h
 * @brief Thread-safe error handling infrastructure for YAEP
 *
 * Provides thread-local error context management for modern C17 error handling.
 * Each thread maintains its own error state, enabling concurrent grammar
 * processing and explicit error propagation via return codes.
 */

#ifndef YAEP_ERROR_H
#define YAEP_ERROR_H

#include <stdarg.h>

#include "yaep.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef YAEP_MAX_ERROR_MESSAGE_LENGTH
#define YAEP_MAX_ERROR_MESSAGE_LENGTH 1024
#endif

/**
 * @brief Thread-local error context.
 *
 * Stores the most recent YAEP error for the current thread, including the
 * error code, formatted error message, and originating grammar pointer for
 * diagnostics. This structure is cleared at the start of each public API call
 * and populated when errors occur.
 */
typedef struct {
    int error_code;                                                /**< YAEP error code */
    char error_message[YAEP_MAX_ERROR_MESSAGE_LENGTH + 1];         /**< Human-readable message */
    struct grammar *grammar_ctx;                                   /**< Grammar context (optional) */
} yaep_error_context_t;

typedef void (*yaep_error_update_hook_t)(struct grammar *g,
                                         const yaep_error_context_t *ctx);

/**
 * @brief Retrieve the current thread's error context.
 *
 * @return Pointer to the thread-local error context (never NULL).
 */
yaep_error_context_t *yaep_get_error_context(void);

/**
 * @brief Record an error in the thread-local context.
 *
 * Formats and stores the provided error message, updates the grammar's
 * embedded error buffer when available, and returns the supplied error code.
 *
 * @param g Optional grammar pointer associated with the error.
 * @param code YAEP error code.
 * @param format printf-style format string for the error message.
 * @param ... Arguments corresponding to @p format.
 *
 * @return The provided error code for convenience.
 */
int yaep_set_error(struct grammar *g, int code, const char *format, ...);

int yaep_vset_error(struct grammar *g, int code, const char *format,
                    va_list args);

/**
 * @brief Copy the current thread error into the specified grammar.
 *
 * @param g Grammar receiving the error state (must be non-NULL).
 */
void yaep_copy_error_to_grammar(struct grammar *g);

/**
 * @brief Reset the thread-local error context to its initial state.
 */
void yaep_clear_error(void);

/**
 * @brief Register callback used to synchronize grammar error state.
 */
void yaep_set_error_update_hook(yaep_error_update_hook_t hook);

/* Note: The error boundary infrastructure (yaep_error_boundary_raise,
 * yaep_error_boundary_is_active, yaep_run_with_error_boundary) has been
 * removed as part of the C17 modernization. All error handling now uses
 * explicit return codes with thread-local error context. */

#ifdef __cplusplus
}
#endif

#endif /* YAEP_ERROR_H */
