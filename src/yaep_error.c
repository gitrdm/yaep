/**
 * @file yaep_error.c
 * @brief Implementation of thread-local error handling for YAEP.
 */

#include "yaep_error.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static yaep_error_update_hook_t yaep_error_update_hook = NULL;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
static _Thread_local yaep_error_context_t yaep_thread_error_ctx = {0};
#elif defined(__GNUC__) || defined(__clang__)
static __thread yaep_error_context_t yaep_thread_error_ctx = {0};
#elif defined(_MSC_VER)
static __declspec(thread) yaep_error_context_t yaep_thread_error_ctx = {0};
#else
#error "Thread-local storage not supported on this compiler"
#endif

yaep_error_context_t *
yaep_get_error_context(void)
{
    return &yaep_thread_error_ctx;
}

static void
yaep_update_grammar_if_needed(struct grammar *g,
                              const yaep_error_context_t *ctx)
{
    if (g != NULL && yaep_error_update_hook != NULL) {
        yaep_error_update_hook(g, ctx);
    }
}

int
yaep_vset_error(struct grammar *g, int code, const char *format, va_list args)
{
    yaep_error_context_t *ctx = yaep_get_error_context();

    ctx->error_code = code;
    ctx->grammar_ctx = g;

    vsnprintf(ctx->error_message, sizeof(ctx->error_message), format, args);

    ctx->error_message[sizeof(ctx->error_message) - 1] = '\0';

    yaep_update_grammar_if_needed(g, ctx);

    return code;
}

int
yaep_set_error(struct grammar *g, int code, const char *format, ...)
{
    va_list args;
    int result;

    va_start(args, format);
    result = yaep_vset_error(g, code, format, args);
    va_end(args);

    return result;
}

void
yaep_copy_error_to_grammar(struct grammar *g)
{
    yaep_error_context_t *ctx = yaep_get_error_context();

    assert(g != NULL);

    yaep_update_grammar_if_needed(g, ctx);
}

void
yaep_clear_error(void)
{
    yaep_error_context_t *ctx = yaep_get_error_context();

    ctx->error_code = 0;
    ctx->error_message[0] = '\0';
    ctx->grammar_ctx = NULL;
}

void
yaep_set_error_update_hook(yaep_error_update_hook_t hook)
{
    yaep_error_update_hook = hook;
}
