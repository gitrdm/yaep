/**
 * @file yaep_cleanup.h
 * @brief Automatic resource cleanup helpers built on compiler attributes.
 *
 * This header exposes macros and helper functions that enable
 * RAII-style cleanup for YAEP internals when compiled with compilers
 * supporting GCC/Clang's `cleanup` attribute. The helpers gracefully
 * degrade to no-ops on toolchains without attribute support so callers
 * can fall back to manual cleanup code paths.
 */

#ifndef YAEP_CLEANUP_H
#define YAEP_CLEANUP_H

#include "allocate.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) || defined(__clang__)
#define YAEP_HAVE_CLEANUP_ATTRIBUTE 1
#define YAEP_AUTO_CLEANUP(func) __attribute__((cleanup(func)))
#else
#define YAEP_HAVE_CLEANUP_ATTRIBUTE 0
#define YAEP_AUTO_CLEANUP(func)
#endif

static inline void
cleanup_allocator(YaepAllocator **alloc_ptr)
{
    if (alloc_ptr != NULL && *alloc_ptr != NULL) {
        yaep_alloc_del(*alloc_ptr);
        *alloc_ptr = NULL;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* YAEP_CLEANUP_H */
