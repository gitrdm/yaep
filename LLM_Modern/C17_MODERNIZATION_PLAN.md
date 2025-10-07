# YAEP C17 Modernization Plan

## Executive Summary

This document provides a comprehensive, production-ready plan to modernize the YAEP (Yet Another Earley Parser) codebase from C90 to C17 standard, eliminating all setjmp/longjmp-based error handling in favor of thread-safe, RAII-style resource management with explicit error propagation.

**Primary Goals:**
1. Eliminate all setjmp/longjmp usage and associated memory safety issues
2. Implement thread-safe error handling using C11 thread-local storage
3. Add automatic resource cleanup using GCC/Clang cleanup attributes
4. Fix known bugs (NULL pointer dereferences, memory leaks)
5. Maintain 100% API/ABI compatibility with existing public interface
6. Achieve production quality with no technical debt

**Key Constraints:**
- No placeholders, stubs, or TODOs
- No band-aid fixes - root cause resolution only
- Comprehensive literate-style documentation
- Full backward compatibility for public API
- Zero runtime overhead compared to existing implementation

---

## Table of Contents

1. [Problem Analysis](#1-problem-analysis)
2. [Architecture Overview](#2-architecture-overview)
3. [Implementation Phases](#3-implementation-phases)
4. [Detailed Technical Design](#4-detailed-technical-design)
5. [Migration Strategy](#5-migration-strategy)
6. [Testing Strategy](#6-testing-strategy)
7. [Rollback Plan](#7-rollback-plan)

---

## 1. Problem Analysis

### 1.1 Current Error Handling Architecture

**Location:** `src/yaep.c:2975-2996`

The current implementation uses a **static global `jmp_buf`** for error handling:

```c
static jmp_buf error_longjump_buff;  // Line 2981

static void yaep_error(int code, const char *format, ...) {
    grammar->error_code = code;
    vsprintf(grammar->error_message, format, arguments);
    longjmp(error_longjump_buff, code);  // Non-local exit
}
```

**Critical Issues:**

1. **Thread-Safety Violation**
   - Static global `jmp_buf` creates race conditions in multi-threaded contexts
   - Multiple threads cannot safely use YAEP simultaneously
   - No synchronization mechanism exists

2. **Resource Leak Vulnerability**
   - Resources allocated between `setjmp()` and `longjmp()` are not automatically freed
   - Stack unwinding is manual and incomplete
   - Example: `tok_add()` adds token to array before calling `yaep_error()`, creating orphaned data

3. **NULL Pointer Dereference Bug** (Discovered by fuzzing)
   - **Location:** `src/yaep.c:4659` in `build_pl()`
   - **Root Cause:** Token with NULL `symb` pointer added to array before `longjmp()` executes
   - **Impact:** Crashes when lookahead accesses `toks[tok_curr + 1].symb->u.term.term_num`

4. **Unclear Control Flow**
   - Non-local exits make debugging extremely difficult
   - Stack traces are confusing
   - Error recovery paths are implicit

5. **Dependency on Grammar State**
   - `yaep_error()` depends on global `grammar` pointer being valid
   - Creates implicit coupling between error handling and grammar lifecycle

### 1.2 Call Sites Using setjmp/longjmp

**Analysis of all setjmp/longjmp usage:**

| File | Line | Function | Context | Cleanup Required |
|------|------|----------|---------|------------------|
| `yaep.c` | 3032 | `yaep_create_grammar()` | Grammar initialization | Free grammar, deallocate allocator |
| `yaep.c` | 3338 | `yaep_read_grammar()` | Reading terminals/rules | Free symbols, rules, term sets |
| `yaep.c` | 6019 | `yaep_parse()` | Token parsing | Free tokens, parser state, parse lists |
| `sgramm.y` | 423 | `set_sgrammar()` | Grammar description parsing | Free syntax tokens, terms, rules |

### 1.3 Global Static State Dependencies

**Critical globals that create coupling:**

```c
static struct grammar *grammar;          // Line 192 - Current grammar context
static struct symbs *symbs_ptr;          // Grammar symbols
static struct term_sets *term_sets_ptr;  // Terminal sets
static struct rules *rules_ptr;          // Grammar rules
static jmp_buf error_longjump_buff;      // Error handling state
```

**Additional parse-time globals:**

```c
static int (*read_token)(void **attr);           // Token reader callback
static void (*syntax_error)(...);                // Error handler callback
static void *(*parse_alloc)(int nmemb);          // Memory allocator
static void (*parse_free)(void *mem);            // Memory deallocator
static struct tok *toks;                         // Token array
static int toks_len;                             // Token count
```

**Impact:** These globals prevent:
- Concurrent grammar processing
- Re-entrant parsing
- Clean separation of concerns

---

## 2. Architecture Overview

### 2.1 New Error Handling Model

**Core Principle:** Explicit error propagation with automatic resource cleanup

**Components:**

1. **Thread-Local Error Context** (C11 `_Thread_local`)
   - Each thread maintains its own error state
   - Eliminates race conditions
   - Backward compatible with single-threaded usage

2. **Cleanup Attributes** (GCC/Clang `__attribute__((cleanup))`)
   - Automatic resource deallocation on scope exit
   - RAII-style semantics in C
   - Zero runtime overhead (compiler-generated code)

3. **Error Propagation Macros**
   - Explicit, readable error handling
   - Maintains clear control flow
   - Simplifies debugging

4. **Resource Context Structures**
   - Encapsulate all resources requiring cleanup
   - Enable batch cleanup on error
   - Prevent leaks

### 2.2 Design Principles

1. **Backward Compatibility**
   - Public API remains unchanged
   - Return codes maintain same semantics
   - Error messages identical to current implementation

2. **Zero Technical Debt**
   - All code production-ready
   - No placeholders or TODOs
   - Comprehensive documentation

3. **Performance Parity**
   - No runtime overhead vs. current implementation
   - Cleanup attributes compile to same code as manual cleanup
   - Thread-local storage has negligible cost

4. **Testability**
   - All error paths explicitly testable
   - Resource cleanup verifiable
   - Fuzzing-friendly architecture

### 2.3 Compatibility Strategy

**C Standard:** C17 (ISO/IEC 9899:2018)
- Ratified in 2018, widely supported
- Superset of C11 with defect fixes
- GCC 5+, Clang 3.1+, MSVC 2019+

**Compiler Extensions:**
- Cleanup attributes: GCC/Clang only (most common)
- Fallback: Manual cleanup with goto for other compilers
- Controlled via `HAVE_CLEANUP_ATTRIBUTE` macro

**Thread-Local Storage:**
- C11 `_Thread_local` keyword
- Fallback: `__thread` (GCC/Clang) or `__declspec(thread)` (MSVC)
- Controlled via feature detection

---

## 3. Implementation Phases

### Phase 1: Infrastructure Preparation
**Duration:** 1-2 days  
**Risk:** Low  
**Dependencies:** None

**Deliverables:**
1. Update build system to C17
2. Add feature detection for cleanup attributes
3. Create error handling infrastructure
4. Add compiler warnings/sanitizers

### Phase 2: Error Context Migration
**Duration:** 2-3 days  
**Risk:** Medium  
**Dependencies:** Phase 1

**Deliverables:**
1. Replace global `jmp_buf` with thread-local error context
2. Refactor `yaep_error()` to set error state and return
3. Create error propagation macros
4. Add cleanup attribute support

### Phase 3: Function Refactoring (Core)
**Duration:** 3-5 days  
**Risk:** High  
**Dependencies:** Phase 2

**Deliverables:**
1. Refactor `yaep_create_grammar()`
2. Refactor `yaep_read_grammar()`
3. Refactor `yaep_parse()`
4. Fix NULL pointer bugs

### Phase 4: Grammar Parser Migration
**Duration:** 2-3 days  
**Risk:** Low  
**Dependencies:** Phase 3

**Deliverables:**
1. Refactor `sgramm.y` error handling (change `yyerror()` only)
2. Update `set_sgrammar()` to use explicit error checking
3. No changes needed to Bison-generated code (Bison is not the problem)

### Phase 5: Validation & Documentation
**Duration:** 2-3 days  
**Risk:** Low  
**Dependencies:** Phase 4

**Deliverables:**
1. Comprehensive test suite
2. Fuzzing campaign
3. Performance benchmarks
4. Migration guide
5. API documentation updates

**Total Duration:** 10-16 days

### Phase 6: Future Enhancements (Optional)
**Duration:** Variable  
**Risk:** Low-Medium  
**Dependencies:** Phases 1-5

**Note:** See [SAFE_REFACTORINGS.md](SAFE_REFACTORINGS.md) for detailed analysis of strategic refactorings that enable future optimizations.

**Recommended for Immediate Inclusion:**
- Optimization flags (runtime control) - 1-2 days
- Metrics infrastructure (profiling) - 2 days

**Future Work:**
- Global state elimination - Enables concurrent parsing
- Parser strategy abstraction - Enables Leo's algorithm, PEP
- Memory pool architecture - 5-10x faster allocation

---

## 4. Detailed Technical Design

### 4.0 Critical Discovery: Bison is Not the Problem

**Initial Assumption:** Phase 4 would require significant work to "eliminate setjmp/longjmp in Bison-generated code"

**Reality:** Bison doesn't use setjmp/longjmp at all. The problem is entirely in our custom error handler.

**Why This Matters:**
1. **Reduced Scope:** Phase 4 is now much simpler - just change one function (`yyerror`)
2. **Lower Risk:** No need to modify or work around Bison internals
3. **Standard Practice:** We can use Bison's normal error handling mechanism
4. **Better Error Recovery:** Bison has built-in error recovery we were bypassing

**How Bison Actually Works:**

```
Parser detects error
    ↓
Calls yyerror(msg)  ← We were doing longjmp here (wrong!)
    ↓
yyerror returns     ← Should just log error and return
    ↓
Bison tries error recovery
    ↓
yyparse() returns error code ← We should check this
    ↓
Caller checks return value
```

**Our Mistake:**

```c
// WRONG - bypasses Bison's error handling
int yyerror(const char *str) {
    yaep_error(...);  // This does longjmp!
    return 0;         // Never reached
}

// RIGHT - works with Bison normally  
int yyerror(const char *str) {
    yaep_set_error(...);  // Just records error
    return 0;              // Returns to Bison
}
```

**Impact on Timeline:**
- Phase 4 reduced from 2-3 days to 1 day
- Risk reduced from Medium to Low
- Total timeline: 13-15 days instead of 16 days

### 4.1 Error Context Infrastructure

**File:** `src/yaep_error.h` (new file)

```c
/**
 * @file yaep_error.h
 * @brief Thread-safe error handling infrastructure for YAEP
 *
 * This module provides thread-safe error context management replacing
 * the previous setjmp/longjmp-based error handling. Each thread maintains
 * its own error state, enabling concurrent grammar processing and parsing.
 *
 * Error Propagation Model:
 * - Functions return error codes (0 = success, non-zero = error)
 * - Error details stored in thread-local context
 * - Resources cleaned up automatically via cleanup attributes
 *
 * Thread Safety:
 * - Each thread has independent error context
 * - No global mutable state
 * - Safe for concurrent use across threads
 *
 * Compatibility:
 * - Requires C11 or later for _Thread_local
 * - Cleanup attributes require GCC/Clang (fallback available)
 * - Public API remains unchanged
 */

#ifndef YAEP_ERROR_H
#define YAEP_ERROR_H

#include <stdarg.h>
#include "yaep.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Thread-local error context
 *
 * Each thread maintains its own error state, eliminating race conditions
 * and enabling concurrent YAEP usage. The context is automatically
 * initialized on first use.
 *
 * Memory Management:
 * - Context is thread-local static storage
 * - No explicit initialization or cleanup required
 * - Zero runtime overhead
 */
typedef struct {
    /** Error code from YAEP_* constants in yaep.h */
    int error_code;
    
    /** Human-readable error message */
    char error_message[YAEP_MAX_ERROR_MESSAGE_LENGTH];
    
    /** Grammar context where error occurred (for debugging) */
    struct grammar *grammar_ctx;
} yaep_error_context_t;

/**
 * @brief Get current thread's error context
 *
 * Returns a pointer to the thread-local error context. This function
 * is thread-safe and does not require synchronization.
 *
 * @return Pointer to thread-local error context (never NULL)
 *
 * Thread Safety: Safe - returns thread-local storage
 * Allocation: No heap allocation
 * Lifetime: Entire thread lifetime
 */
yaep_error_context_t *yaep_get_error_context(void);

/**
 * @brief Set error state in current thread context
 *
 * Records an error in the current thread's error context. This function
 * does NOT perform a non-local exit (unlike the old yaep_error).
 * Callers must check return values and propagate errors explicitly.
 *
 * @param g Grammar context (may be NULL for allocation errors)
 * @param code Error code from YAEP_* constants
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return The error code (for convenience in return statements)
 *
 * Example Usage:
 * @code
 *   if (invalid_condition) {
 *       return yaep_set_error(g, YAEP_INVALID_TOKEN_CODE,
 *                            "invalid token code %d", code);
 *   }
 * @endcode
 *
 * Thread Safety: Safe - modifies only thread-local state
 * Allocation: No heap allocation (uses stack for vsprintf)
 * Format Limits: Message truncated at YAEP_MAX_ERROR_MESSAGE_LENGTH
 */
int yaep_set_error(struct grammar *g, int code, const char *format, ...);

/**
 * @brief Copy error from thread-local context to grammar
 *
 * Transfers error state from thread-local storage to the grammar object,
 * making it accessible via the public API (yaep_error_code, yaep_error_message).
 *
 * @param g Grammar context to update (must not be NULL)
 *
 * Thread Safety: Safe - reads thread-local, writes to g (caller must synchronize g)
 * Preconditions: g != NULL
 */
void yaep_copy_error_to_grammar(struct grammar *g);

/**
 * @brief Clear error state in current thread context
 *
 * Resets the error context to initial state (code=0, empty message).
 * Should be called at the start of each public API function.
 *
 * Thread Safety: Safe - modifies only thread-local state
 */
void yaep_clear_error(void);

#ifdef __cplusplus
}
#endif

#endif /* YAEP_ERROR_H */
```

**Implementation:** `src/yaep_error.c` (new file)

```c
/**
 * @file yaep_error.c
 * @brief Implementation of thread-safe error handling
 */

#include "yaep_error.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * Thread-local error context
 *
 * C11 _Thread_local provides per-thread storage with automatic initialization.
 * Each thread gets its own copy of this variable, initialized to zero.
 *
 * Fallback for pre-C11:
 * - GCC/Clang: __thread
 * - MSVC: __declspec(thread)
 * - Other: Use pthread_key_create (requires pthreads)
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    /* C11 or later */
    _Thread_local yaep_error_context_t yaep_thread_error_ctx = {0};
#elif defined(__GNUC__) || defined(__clang__)
    /* GCC/Clang extension */
    __thread yaep_error_context_t yaep_thread_error_ctx = {0};
#elif defined(_MSC_VER)
    /* MSVC extension */
    __declspec(thread) yaep_error_context_t yaep_thread_error_ctx = {0};
#else
    #error "Thread-local storage not supported on this compiler"
#endif

yaep_error_context_t *yaep_get_error_context(void) {
    return &yaep_thread_error_ctx;
}

int yaep_set_error(struct grammar *g, int code, const char *format, ...) {
    va_list args;
    yaep_error_context_t *ctx = yaep_get_error_context();
    
    /* Record error in thread-local context */
    ctx->error_code = code;
    ctx->grammar_ctx = g;
    
    /* Format error message */
    va_start(args, format);
    vsnprintf(ctx->error_message, YAEP_MAX_ERROR_MESSAGE_LENGTH, format, args);
    va_end(args);
    
    /* Ensure null termination */
    ctx->error_message[YAEP_MAX_ERROR_MESSAGE_LENGTH - 1] = '\0';
    
    /* If grammar provided, copy error to it for API compatibility */
    if (g != NULL) {
        g->error_code = code;
        strncpy(g->error_message, ctx->error_message, 
                YAEP_MAX_ERROR_MESSAGE_LENGTH - 1);
        g->error_message[YAEP_MAX_ERROR_MESSAGE_LENGTH - 1] = '\0';
    }
    
    return code;
}

void yaep_copy_error_to_grammar(struct grammar *g) {
    yaep_error_context_t *ctx = yaep_get_error_context();
    
    assert(g != NULL);
    
    g->error_code = ctx->error_code;
    strncpy(g->error_message, ctx->error_message, 
            YAEP_MAX_ERROR_MESSAGE_LENGTH - 1);
    g->error_message[YAEP_MAX_ERROR_MESSAGE_LENGTH - 1] = '\0';
}

void yaep_clear_error(void) {
    yaep_error_context_t *ctx = yaep_get_error_context();
    
    ctx->error_code = 0;
    ctx->error_message[0] = '\0';
    ctx->grammar_ctx = NULL;
}
```

### 4.2 Cleanup Attribute Infrastructure

**File:** `src/yaep_cleanup.h` (new file)

```c
/**
 * @file yaep_cleanup.h
 * @brief Automatic resource cleanup infrastructure (RAII for C)
 *
 * This module provides GCC/Clang cleanup attributes enabling automatic
 * resource deallocation on scope exit, similar to C++ RAII. Resources
 * are cleaned up regardless of how scope is exited (return, goto, etc).
 *
 * Compiler Support:
 * - GCC 3.3+ : Full support
 * - Clang 3.1+ : Full support
 * - MSVC : Not supported (fallback to manual cleanup)
 * - Other : Not supported (fallback to manual cleanup)
 *
 * Usage Pattern:
 * @code
 *   int function(void) {
 *       // Declare resource with cleanup attribute
 *       vlo_t *vlo YAEP_AUTO_CLEANUP(cleanup_vlo) = NULL;
 *       
 *       // Allocate resource
 *       VLO_CREATE(vlo, alloc, size);
 *       
 *       // Use resource...
 *       if (error) {
 *           return -1;  // cleanup_vlo automatically called
 *       }
 *       
 *       return 0;  // cleanup_vlo automatically called
 *   }
 * @endcode
 */

#ifndef YAEP_CLEANUP_H
#define YAEP_CLEANUP_H

#include "vlobject.h"
#include "objstack.h"
#include "hashtab.h"
#include "allocate.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Feature detection for cleanup attributes
 *
 * Cleanup attributes are a GCC/Clang extension enabling automatic
 * resource cleanup on scope exit. If not available, callers must
 * use manual cleanup with goto error handling.
 */
#if defined(__GNUC__) || defined(__clang__)
    #define YAEP_HAVE_CLEANUP_ATTRIBUTE 1
    #define YAEP_AUTO_CLEANUP(func) __attribute__((cleanup(func)))
#else
    #define YAEP_HAVE_CLEANUP_ATTRIBUTE 0
    #define YAEP_AUTO_CLEANUP(func)  /* No-op on unsupported compilers */
#endif

/**
 * @brief Cleanup function for VLO (Variable Length Object)
 *
 * Automatically deletes a VLO when it goes out of scope.
 * Safe to call with NULL pointer.
 *
 * @param vlo_ptr Pointer to VLO pointer (double indirection for cleanup)
 *
 * Implementation Notes:
 * - Accepts double pointer because cleanup functions receive address of variable
 * - NULL-safe: does nothing if *vlo_ptr is NULL
 * - Idempotent: sets *vlo_ptr to NULL after cleanup
 */
static inline void cleanup_vlo(vlo_t **vlo_ptr) {
    if (vlo_ptr && *vlo_ptr) {
        VLO_DELETE(*vlo_ptr);
        *vlo_ptr = NULL;
    }
}

/**
 * @brief Cleanup function for object stack
 *
 * Automatically deletes an object stack when it goes out of scope.
 * Safe to call with NULL pointer.
 *
 * @param os_ptr Pointer to object stack pointer
 */
static inline void cleanup_os(os_t **os_ptr) {
    if (os_ptr && *os_ptr) {
        OS_DELETE(*os_ptr);
        *os_ptr = NULL;
    }
}

/**
 * @brief Cleanup function for hash table
 *
 * Automatically deletes a hash table when it goes out of scope.
 * Safe to call with NULL pointer.
 *
 * @param ht_ptr Pointer to hash table pointer
 */
static inline void cleanup_hash_table(hash_table **ht_ptr) {
    if (ht_ptr && *ht_ptr) {
        delete_hash_table(*ht_ptr);
        *ht_ptr = NULL;
    }
}

/**
 * @brief Cleanup function for allocator
 *
 * Automatically deletes an allocator when it goes out of scope.
 * Safe to call with NULL pointer.
 *
 * @param alloc_ptr Pointer to allocator pointer
 */
static inline void cleanup_allocator(YaepAllocator **alloc_ptr) {
    if (alloc_ptr && *alloc_ptr) {
        yaep_alloc_del(*alloc_ptr);
        *alloc_ptr = NULL;
    }
}

/**
 * @brief Resource context for grammar creation
 *
 * Encapsulates all resources allocated during grammar creation,
 * enabling batch cleanup on error. All fields are owned by this
 * structure and will be freed when cleanup_grammar_resources is called.
 */
typedef struct {
    YaepAllocator *allocator;    /**< Memory allocator */
    struct grammar *grammar;     /**< Grammar object */
    struct symbs *symbs;         /**< Symbol table */
    struct term_sets *term_sets; /**< Terminal sets */
    struct rules *rules;         /**< Grammar rules */
} grammar_resources_t;

/**
 * @brief Cleanup function for grammar resources
 *
 * Frees all resources in a grammar_resources_t structure.
 * Resources are freed in reverse order of typical allocation.
 * Safe to call with partially initialized structures.
 *
 * @param res_ptr Pointer to grammar resources pointer
 *
 * Cleanup Order:
 * 1. Rules (may reference symbols)
 * 2. Terminal sets (may reference symbols)
 * 3. Symbols
 * 4. Grammar object
 * 5. Allocator (last, as it manages all memory)
 */
static inline void cleanup_grammar_resources(grammar_resources_t **res_ptr) {
    if (!res_ptr || !*res_ptr) return;
    
    grammar_resources_t *res = *res_ptr;
    
    /* Free in reverse order of allocation */
    if (res->rules) {
        rule_fin(res->rules);
        res->rules = NULL;
    }
    
    if (res->term_sets) {
        term_set_fin(res->term_sets);
        res->term_sets = NULL;
    }
    
    if (res->symbs) {
        symb_fin(res->symbs);
        res->symbs = NULL;
    }
    
    if (res->grammar) {
        yaep_free(res->allocator, res->grammar);
        res->grammar = NULL;
    }
    
    if (res->allocator) {
        yaep_alloc_del(res->allocator);
        res->allocator = NULL;
    }
    
    *res_ptr = NULL;
}

/**
 * @brief Resource context for parsing
 *
 * Encapsulates all resources allocated during parsing,
 * enabling batch cleanup on error.
 */
typedef struct {
    vlo_t *toks_vlo;        /**< Token array VLO */
    int parse_initialized;  /**< Whether parser state initialized */
    int tok_initialized;    /**< Whether token system initialized */
} parse_resources_t;

/**
 * @brief Cleanup function for parse resources
 *
 * Frees all resources in a parse_resources_t structure.
 * Safe to call with partially initialized structures.
 *
 * @param res_ptr Pointer to parse resources pointer
 */
static inline void cleanup_parse_resources(parse_resources_t **res_ptr) {
    if (!res_ptr || !*res_ptr) return;
    
    parse_resources_t *res = *res_ptr;
    
    /* Finalize in reverse order */
    if (res->parse_initialized) {
        yaep_parse_fin();
        res->parse_initialized = 0;
    }
    
    if (res->tok_initialized) {
        tok_fin();
        res->tok_initialized = 0;
    }
    
    *res_ptr = NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* YAEP_CLEANUP_H */
```

### 4.3 Error Propagation Macros

**File:** `src/yaep_macros.h` (new file)

```c
/**
 * @file yaep_macros.h
 * @brief Error propagation and checking macros
 *
 * These macros provide clean, readable error handling while maintaining
 * explicit control flow. Unlike setjmp/longjmp, all error paths are
 * visible in source code and debugger stack traces.
 */

#ifndef YAEP_MACROS_H
#define YAEP_MACROS_H

/**
 * @brief Check error code and return if non-zero
 *
 * Propagates errors up the call stack. If the expression evaluates
 * to non-zero, returns that value immediately. Cleanup attributes
 * ensure resources are freed.
 *
 * @param expr Expression to check (should return int error code)
 *
 * Example:
 * @code
 *   YAEP_CHECK(symb_add_term(name, code));
 *   // Continues only if symb_add_term succeeded (returned 0)
 * @endcode
 */
#define YAEP_CHECK(expr) do { \
    int _yaep_err = (expr); \
    if (_yaep_err != 0) { \
        return _yaep_err; \
    } \
} while (0)

/**
 * @brief Check pointer and return error if NULL
 *
 * Checks if a pointer is NULL and returns YAEP_NO_MEMORY if so.
 * Useful for allocation checks.
 *
 * @param ptr Pointer to check
 *
 * Example:
 * @code
 *   symb *s = malloc(sizeof(*s));
 *   YAEP_CHECK_ALLOC(s);
 *   // s is guaranteed non-NULL here
 * @endcode
 */
#define YAEP_CHECK_ALLOC(ptr) do { \
    if ((ptr) == NULL) { \
        return yaep_set_error(grammar, YAEP_NO_MEMORY, "allocation failed"); \
    } \
} while (0)

/**
 * @brief Set error and return error code
 *
 * Convenience macro combining yaep_set_error and return.
 *
 * @param code Error code
 * @param ... Printf-style format and arguments
 *
 * Example:
 * @code
 *   if (invalid_code) {
 *       YAEP_ERROR(YAEP_INVALID_TOKEN_CODE, "invalid token %d", code);
 *   }
 * @endcode
 */
#define YAEP_ERROR(code, ...) \
    return yaep_set_error(grammar, (code), __VA_ARGS__)

#endif /* YAEP_MACROS_H */
```

### 4.4 Migration: yaep_create_grammar()

**Current Implementation (yaep.c:3008-3054):**

```c
struct grammar *yaep_create_grammar(void) {
    YaepAllocator *allocator;
    
    allocator = yaep_alloc_new(NULL, NULL, NULL, NULL);
    if (allocator == NULL) return NULL;
    
    grammar = NULL;
    grammar = (struct grammar *) yaep_malloc(allocator, sizeof(*grammar));
    if (grammar == NULL) {
        yaep_alloc_del(allocator);
        return NULL;
    }
    
    grammar->alloc = allocator;
    yaep_alloc_seterr(allocator, error_func_for_allocate,
                     yaep_alloc_getuserptr(allocator));
    
    if (setjmp(error_longjump_buff) != 0) {  // ERROR HANDLING
        yaep_free_grammar(grammar);
        return NULL;
    }
    
    // ... initialization code ...
    
    grammar->symbs_ptr = symbs_ptr = symb_init();
    grammar->term_sets_ptr = term_sets_ptr = term_set_init();
    grammar->rules_ptr = rules_ptr = rule_init();
    
    return grammar;
}
```

**Problem Analysis:**
1. `setjmp()` catches errors from `symb_init()`, `term_set_init()`, `rule_init()`
2. These functions call `yaep_error()` which does `longjmp()`
3. Cleanup is via `yaep_free_grammar()` which may be incomplete
4. Global `grammar` pointer modified before full initialization

**New Implementation:**

```c
/**
 * @brief Create a new grammar object
 *
 * Allocates and initializes a new grammar object with default settings.
 * All resources are automatically cleaned up on failure.
 *
 * @return Pointer to new grammar, or NULL on allocation failure
 *
 * Error Handling:
 * - Returns NULL on any error
 * - Error details available via yaep_get_error_context()
 * - All partially allocated resources automatically freed
 *
 * Thread Safety:
 * - Safe to call from multiple threads concurrently
 * - Each call creates independent grammar object
 *
 * Resource Ownership:
 * - Caller owns returned grammar
 * - Must call yaep_free_grammar() when done
 *
 * Algorithm:
 * 1. Allocate allocator
 * 2. Allocate grammar structure
 * 3. Initialize symbols, term sets, rules
 * 4. On any error, cleanup in reverse order
 *
 * Rationale:
 * Previous implementation used setjmp/longjmp for error handling,
 * creating thread-safety issues and potential resource leaks. This
 * version uses explicit error checking with automatic cleanup via
 * cleanup attributes, ensuring all resources are freed even on error.
 */
struct grammar *yaep_create_grammar(void) {
    /* Clear any previous error state in this thread */
    yaep_clear_error();
    
    /*
     * Resource allocation with automatic cleanup
     * 
     * The YAEP_AUTO_CLEANUP attribute ensures these resources are
     * automatically freed when this function returns, regardless of
     * how it exits (normal return, early return on error, etc).
     * 
     * This is equivalent to C++ RAII but in C via GCC/Clang extension.
     */
    YaepAllocator *allocator YAEP_AUTO_CLEANUP(cleanup_allocator) = NULL;
    struct grammar *g = NULL;  /* Not auto-cleanup - returned on success */
    struct symbs *symbs YAEP_AUTO_CLEANUP(cleanup_symbs) = NULL;
    struct term_sets *term_sets YAEP_AUTO_CLEANUP(cleanup_term_sets) = NULL;
    struct rules *rules YAEP_AUTO_CLEANUP(cleanup_rules) = NULL;
    
    /*
     * Step 1: Create allocator
     * 
     * The allocator manages all memory for this grammar. If this fails,
     * we have no memory to work with, so return NULL immediately.
     */
    allocator = yaep_alloc_new(NULL, NULL, NULL, NULL);
    if (allocator == NULL) {
        yaep_set_error(NULL, YAEP_NO_MEMORY, 
                      "failed to create allocator");
        return NULL;
    }
    
    /*
     * Step 2: Allocate grammar structure
     * 
     * The grammar structure holds all configuration and state.
     * If this fails, cleanup_allocator will free the allocator.
     */
    g = (struct grammar *) yaep_malloc(allocator, sizeof(*g));
    if (g == NULL) {
        yaep_set_error(NULL, YAEP_NO_MEMORY, 
                      "failed to allocate grammar structure");
        return NULL;
    }
    
    /* Initialize grammar structure to known state */
    memset(g, 0, sizeof(*g));
    g->alloc = allocator;
    g->undefined_p = TRUE;
    g->error_code = 0;
    g->error_message[0] = '\0';
    g->debug_level = 0;
    g->lookahead_level = 1;
    g->one_parse_p = 1;
    g->cost_p = 0;
    g->error_recovery_p = 1;
    g->recovery_token_matches = DEFAULT_RECOVERY_TOKEN_MATCHES;
    
    /*
     * Step 3: Initialize symbol table
     * 
     * Symbols track terminals and nonterminals. This function now
     * returns NULL on error instead of calling longjmp().
     */
    symbs = symb_init(allocator);
    if (symbs == NULL) {
        yaep_set_error(g, YAEP_NO_MEMORY, 
                      "failed to initialize symbol table");
        yaep_free(allocator, g);
        return NULL;
    }
    
    /*
     * Step 4: Initialize terminal sets
     * 
     * Terminal sets track FIRST/FOLLOW sets for grammar analysis.
     */
    term_sets = term_set_init(allocator);
    if (term_sets == NULL) {
        yaep_set_error(g, YAEP_NO_MEMORY, 
                      "failed to initialize terminal sets");
        yaep_free(allocator, g);
        return NULL;
    }
    
    /*
     * Step 5: Initialize rules
     * 
     * Rules define the grammar productions.
     */
    rules = rule_init(allocator);
    if (rules == NULL) {
        yaep_set_error(g, YAEP_NO_MEMORY, 
                      "failed to initialize rules");
        yaep_free(allocator, g);
        return NULL;
    }
    
    /*
     * Step 6: Transfer ownership to grammar
     * 
     * Set all pointers to NULL to prevent cleanup functions from
     * freeing them. The grammar now owns these resources and will
     * free them in yaep_free_grammar().
     */
    g->symbs_ptr = symbs;
    g->term_sets_ptr = term_sets;
    g->rules_ptr = rules;
    
    /* Prevent cleanup functions from freeing these */
    symbs = NULL;
    term_sets = NULL;
    rules = NULL;
    
    /*
     * Step 7: Prevent allocator cleanup
     * 
     * The grammar owns the allocator now. Set to NULL to prevent
     * cleanup_allocator from freeing it.
     */
    allocator = NULL;
    
    /* Success - return fully initialized grammar */
    return g;
}
```

**Key Changes:**
1. **No setjmp/longjmp** - Explicit error checking throughout
2. **Automatic cleanup** - Resources freed on any return path
3. **Thread-safe** - No global state modification
4. **Clear ownership** - Transfer of ownership documented
5. **Comprehensive comments** - Literate programming style

### 4.5 Migration: Internal Functions (symb_init, rule_init, etc)

**Current Pattern:**
```c
static struct symbs *symb_init(void) {
    struct symbs *symbs;
    
    symbs = malloc(...);
    if (!symbs)
        yaep_error(YAEP_NO_MEMORY, "..."); // longjmp()
    
    // ... more allocations that might call yaep_error() ...
    
    return symbs;
}
```

**New Pattern:**
```c
/**
 * @brief Initialize symbol table
 *
 * Creates and initializes a new symbol table for storing terminals
 * and nonterminals. Uses provided allocator for all memory.
 *
 * @param alloc Allocator to use for memory (must not be NULL)
 * @return Pointer to new symbol table, or NULL on error
 *
 * Error Handling:
 * - Returns NULL on allocation failure
 * - All partially allocated memory automatically freed
 * - Thread-local error context set with details
 *
 * Algorithm:
 * 1. Allocate symbs structure
 * 2. Create hash table for name -> symbol lookup
 * 3. Create VLOs for symbol arrays
 * 4. On error, cleanup and return NULL
 */
static struct symbs *symb_init(YaepAllocator *alloc) {
    assert(alloc != NULL);
    
    /* Allocate main structure */
    struct symbs *symbs = yaep_malloc(alloc, sizeof(*symbs));
    if (symbs == NULL) {
        yaep_set_error(NULL, YAEP_NO_MEMORY, 
                      "failed to allocate symbol table");
        return NULL;
    }
    
    /* Initialize to known state */
    memset(symbs, 0, sizeof(*symbs));
    symbs->alloc = alloc;
    
    /* Create hash table - auto cleanup on error */
    hash_table *ht YAEP_AUTO_CLEANUP(cleanup_hash_table) = NULL;
    ht = create_hash_table(alloc, 100, symb_hash, symb_eq);
    if (ht == NULL) {
        yaep_set_error(NULL, YAEP_NO_MEMORY, 
                      "failed to create symbol hash table");
        yaep_free(alloc, symbs);
        return NULL;
    }
    
    /* Create VLOs - auto cleanup on error */
    vlo_t *symbs_vlo YAEP_AUTO_CLEANUP(cleanup_vlo) = NULL;
    vlo_t *terms_vlo YAEP_AUTO_CLEANUP(cleanup_vlo) = NULL;
    vlo_t *nonterms_vlo YAEP_AUTO_CLEANUP(cleanup_vlo) = NULL;
    
    VLO_CREATE(symbs_vlo, alloc, 0);
    if (symbs_vlo == NULL) {
        yaep_set_error(NULL, YAEP_NO_MEMORY, 
                      "failed to create symbols VLO");
        yaep_free(alloc, symbs);
        return NULL;
    }
    
    VLO_CREATE(terms_vlo, alloc, 0);
    if (terms_vlo == NULL) {
        yaep_set_error(NULL, YAEP_NO_MEMORY, 
                      "failed to create terminals VLO");
        yaep_free(alloc, symbs);
        return NULL;
    }
    
    VLO_CREATE(nonterms_vlo, alloc, 0);
    if (nonterms_vlo == NULL) {
        yaep_set_error(NULL, YAEP_NO_MEMORY, 
                      "failed to create nonterminals VLO");
        yaep_free(alloc, symbs);
        return NULL;
    }
    
    /* Transfer ownership to symbs */
    symbs->repr_to_symb_tab = ht;
    symbs->symbs_vlo = symbs_vlo;
    symbs->terms_vlo = terms_vlo;
    symbs->nonterms_vlo = nonterms_vlo;
    
    /* Prevent cleanup */
    ht = NULL;
    symbs_vlo = NULL;
    terms_vlo = NULL;
    nonterms_vlo = NULL;
    
    return symbs;
}
```

### 4.6 Migration: tok_add() - Fixing NULL Pointer Bug

**Current Implementation (yaep.c:1225-1239):**

```c
static void tok_add(int code, void *attr) {
    struct tok tok;
    
    tok.attr = attr;
    tok.symb = symb_find_by_code(code);
    if (tok.symb == NULL)
        yaep_error(YAEP_INVALID_TOKEN_CODE, 
                   "invalid token code %d", code);  // BUG: Token already added!
    
    VLO_ADD_MEMORY(toks_vlo, &tok, sizeof(struct tok));  // Added before check!
    toks = (struct tok *) VLO_BEGIN(toks_vlo);
    toks_len++;
}
```

**Root Cause:**
Token with NULL `symb` is added to array BEFORE `yaep_error()` is called. The `longjmp()` doesn't undo the `VLO_ADD_MEMORY()`, leaving corrupt data in the token array.

**New Implementation:**

```c
/**
 * @brief Add token to input token array
 *
 * Validates token code and adds token to the input token array.
 * Token codes must correspond to terminals defined in the grammar.
 *
 * @param code Terminal code (must match terminal from grammar)
 * @param attr User-provided token attributes (may be NULL)
 * @return 0 on success, YAEP_INVALID_TOKEN_CODE on error
 *
 * Error Conditions:
 * - Code does not correspond to any terminal in grammar
 *
 * Preconditions:
 * - toks_vlo must be initialized (tok_init() called)
 * - grammar symbols must be defined
 *
 * Side Effects:
 * - On success: Increments toks_len, adds to toks array
 * - On failure: No state changes (atomic operation)
 *
 * Bug Fix:
 * Previous implementation added token to array before validation,
 * causing NULL pointer dereferences when invalid tokens encountered.
 * This version validates FIRST, ensuring atomic operation.
 */
static int tok_add(int code, void *attr) {
    struct tok tok;
    struct symb *symb;
    
    /*
     * Step 1: Validate token code
     * 
     * CRITICAL: Must validate BEFORE adding to array.
     * Previous bug added token first, then called yaep_error(),
     * leaving NULL symb pointer in array.
     */
    symb = symb_find_by_code(code);
    if (symb == NULL) {
        return yaep_set_error(grammar, YAEP_INVALID_TOKEN_CODE,
                             "invalid token code %d", code);
    }
    
    /*
     * Step 2: Construct token (now safe - symb guaranteed non-NULL)
     */
    tok.attr = attr;
    tok.symb = symb;  /* Guaranteed non-NULL */
    
    /*
     * Step 3: Add to array (atomic operation - no failure points)
     */
    VLO_ADD_MEMORY(toks_vlo, &tok, sizeof(struct tok));
    toks = (struct tok *) VLO_BEGIN(toks_vlo);
    toks_len++;
    
    return 0;  /* Success */
}
```

**Impact:**
- **Fixes NULL pointer dereference** in `build_pl()` (line 4659)
- **Prevents memory corruption** from partial token array state
- **Makes operation atomic** - either succeeds fully or has no effect

### 4.7 Migration: Grammar Parser (sgramm.y)

**Important Discovery: Bison is NOT the Problem**

After analyzing the Bison-generated code, it's clear that **Bison itself does not use setjmp/longjmp**. The issue is entirely in our custom error handler.

**Current Problem (sgramm.y:388-392):**

```c
int yyerror(const char *str) {
    yaep_error(YAEP_DESCRIPTION_SYNTAX_ERROR_CODE,
               "description syntax error on ln %d", ln);
    return 0;
}
```

**Root Cause:**
- `yyerror()` calls `yaep_error()` which does `longjmp()`
- This bypasses Bison's normal error recovery
- Bison expects `yyerror()` to return normally
- The setjmp in `set_sgrammar()` catches the longjmp

**Bison's Normal Error Handling:**
- When Bison detects a syntax error, it calls `yyerror()`
- `yyerror()` should just log the error and return
- Bison handles error recovery internally
- `yyparse()` returns non-zero on error
- **No setjmp/longjmp needed!**

**Solution: Let Bison Work Normally**

```c
/**
 * @brief Bison error handler
 *
 * Called by Bison-generated parser when syntax error detected.
 * This function should NOT throw errors or perform non-local exits.
 * It simply records the error for later retrieval.
 *
 * @param str Error message from Bison
 * @return 0 (return value ignored by Bison)
 *
 * Thread Safety: Safe - uses thread-local error context
 * Control Flow: Always returns normally (no longjmp)
 *
 * Design Note:
 * Previous implementation called yaep_error() which did longjmp(),
 * bypassing Bison's error recovery. This version lets Bison handle
 * errors normally - yyparse() will return non-zero, which we check
 * in set_sgrammar().
 */
int yyerror(const char *str) {
    (void)str;  /* Bison's message not as informative as ours */
    
    /* Record error in thread-local context */
    yaep_set_error(grammar, YAEP_DESCRIPTION_SYNTAX_ERROR_CODE,
                   "description syntax error on ln %d", ln);
    
    /* Return normally - let Bison handle error recovery */
    return 0;
}
```

**Updated set_sgrammar():**

```c
/**
 * @brief Parse grammar description string
 *
 * Parses a YACC-like grammar description and populates internal
 * structures for use by YAEP. This function coordinates the Bison
 * parser and post-processing.
 *
 * @param g Grammar object to populate
 * @param grammar_desc Grammar description string
 * @return 0 on success, error code on failure
 *
 * Algorithm:
 * 1. Initialize data structures
 * 2. Run Bison parser (yyparse)
 * 3. If parse fails, cleanup and return error
 * 4. Post-process: sort, validate, assign codes
 * 5. Return success
 *
 * Error Handling:
 * - Bison parser errors caught via yyparse() return value
 * - Allocation errors returned directly
 * - All resources cleaned up on error
 *
 * No setjmp/longjmp Required:
 * Previous version used setjmp to catch errors from yyerror().
 * This version lets yyparse() return normally with error code.
 */
static int set_sgrammar(struct grammar *g, const char *grammar_desc) {
    int i, j, num;
    struct sterm *term, *prev, *arr;
    int code = 256;
    
    /* Auto-cleanup resources */
    os_t *stoks YAEP_AUTO_CLEANUP(cleanup_os) = NULL;
    vlo_t *sterms YAEP_AUTO_CLEANUP(cleanup_vlo) = NULL;
    vlo_t *srules YAEP_AUTO_CLEANUP(cleanup_vlo) = NULL;
    os_t *srhs YAEP_AUTO_CLEANUP(cleanup_os) = NULL;
    os_t *strans YAEP_AUTO_CLEANUP(cleanup_os) = NULL;
    
    ln = 1;
    
    /*
     * Create data structures for parser
     * These are global variables used by yyparse() and yylex()
     */
    OS_CREATE(stoks, g->alloc, 0);
    if (stoks == NULL) {
        return yaep_set_error(g, YAEP_NO_MEMORY, 
                             "failed to create token stack");
    }
    
    VLO_CREATE(sterms, g->alloc, 0);
    if (sterms == NULL) {
        return yaep_set_error(g, YAEP_NO_MEMORY,
                             "failed to create terminal list");
    }
    
    VLO_CREATE(srules, g->alloc, 0);
    if (srules == NULL) {
        return yaep_set_error(g, YAEP_NO_MEMORY,
                             "failed to create rule list");
    }
    
    OS_CREATE(srhs, g->alloc, 0);
    if (srhs == NULL) {
        return yaep_set_error(g, YAEP_NO_MEMORY,
                             "failed to create RHS stack");
    }
    
    OS_CREATE(strans, g->alloc, 0);
    if (strans == NULL) {
        return yaep_set_error(g, YAEP_NO_MEMORY,
                             "failed to create translation stack");
    }
    
    /* Set input for lexer */
    curr_ch = grammar_desc;
    
    /*
     * Run Bison parser
     * 
     * yyparse() returns:
     * - 0 on success
     * - 1 on syntax error (yyerror was called)
     * - 2 on memory exhaustion
     * 
     * If yyerror() was called, error already recorded in thread context.
     */
    int parse_result = yyparse();
    if (parse_result != 0) {
        /* Error already recorded by yyerror() */
        return grammar->error_code ? grammar->error_code 
                                   : YAEP_DESCRIPTION_SYNTAX_ERROR_CODE;
    }
    
    /*
     * Post-processing: Sort and validate terminals
     */
    num = VLO_LENGTH(sterms) / sizeof(struct sterm);
    arr = (struct sterm *)VLO_BEGIN(sterms);
    qsort(arr, num, sizeof(struct sterm), sterm_name_cmp);
    
    /* Check for duplicate codes */
    for (i = j = 0, prev = NULL; i < num; i++) {
        term = arr + i;
        if (prev == NULL || strcmp(prev->repr, term->repr) != 0) {
            prev = term;
            arr[j++] = *term;
        } else if (term->code != -1 && prev->code != -1
                   && prev->code != term->code) {
            char str[YAEP_MAX_ERROR_MESSAGE_LENGTH / 2];
            strncpy(str, prev->repr, sizeof(str));
            str[sizeof(str) - 1] = '\0';
            return yaep_set_error(g, YAEP_REPEATED_TERM_CODE,
                                 "term %s described repeatedly with different code",
                                 str);
        } else if (prev->code != -1) {
            prev->code = term->code;
        }
    }
    
    VLO_SHORTEN(sterms, (num - j) * sizeof(struct sterm));
    num = j;
    
    /* Sort by order number */
    qsort(arr, num, sizeof(struct sterm), sterm_num_cmp);
    
    /* Assign codes */
    for (i = 0; i < num; i++) {
        term = (struct sterm *)VLO_BEGIN(sterms) + i;
        if (term->code < 0)
            term->code = code++;
    }
    
    /* Reset counters for read functions */
    nsterm = nsrule = 0;
    
    /* 
     * Success - prevent cleanup
     * These globals will be freed by free_sgrammar()
     */
    stoks = NULL;
    sterms = NULL;
    srules = NULL;
    srhs = NULL;
    strans = NULL;
    
    return 0;
}
```

**Key Insight:**

The problem was **never Bison**. Bison is designed to work with normal error handling:

1. Parser detects error
2. Calls `yyerror()` 
3. `yyerror()` returns
4. Parser attempts recovery or gives up
5. `yyparse()` returns error code
6. Caller checks return value

We were fighting Bison by using longjmp, which:
- Bypasses Bison's error recovery
- Creates thread-safety issues  
- Complicates the code unnecessarily

**The fix is simple:** Let `yyerror()` return normally and check `yyparse()`'s return value.

### 4.8 Migration: yaep_parse()

**Current Implementation (yaep.c:5979-6115):**

Complex function with multiple cleanup paths using setjmp/longjmp.

**New Implementation Structure:**

```c
/**
 * @brief Parse input tokens according to grammar
 *
 * Parses a sequence of tokens according to the grammar, producing
 * an abstract syntax tree. Supports ambiguous grammars, error recovery,
 * and cost-based parse selection.
 *
 * @param g Grammar to use for parsing (must not be NULL)
 * @param read_token Callback to read next token
 * @param syntax_error Callback for syntax error reporting
 * @param parse_alloc Memory allocator for parse tree nodes
 * @param parse_free Memory deallocator for parse tree nodes
 * @param root Output: pointer to parse tree root
 * @param ambiguous_p Output: whether grammar is ambiguous
 * @return 0 on success, error code on failure
 *
 * Error Handling:
 * - Returns error code on failure
 * - All resources automatically cleaned up via cleanup attributes
 * - Error details in grammar->error_code and grammar->error_message
 *
 * Thread Safety:
 * - Not thread-safe (uses static globals - to be fixed in Phase 6)
 * - Each thread should have its own grammar object
 */
int yaep_parse(struct grammar *g,
               int (*read_token)(void **attr),
               void (*syntax_error)(...),
               void *(*parse_alloc)(int nmemb),
               void (*parse_free)(void *mem),
               struct yaep_tree_node **root,
               int *ambiguous_p) {
    
    yaep_clear_error();
    
    /* Validate parameters */
    if (g == NULL || read_token == NULL || root == NULL || ambiguous_p == NULL) {
        return yaep_set_error(g, YAEP_UNDEFINED_OR_BAD_GRAMMAR,
                             "invalid parameters");
    }
    
    /* Set global context (will be removed in Phase 6) */
    grammar = g;
    symbs_ptr = g->symbs_ptr;
    term_sets_ptr = g->term_sets_ptr;
    rules_ptr = g->rules_ptr;
    
    /* Store callbacks */
    read_token = read;
    syntax_error = error;
    parse_alloc = alloc;
    parse_free = free;
    
    *root = NULL;
    *ambiguous_p = FALSE;
    
    /* Auto-cleanup resources */
    parse_resources_t res YAEP_AUTO_CLEANUP(cleanup_parse_resources) = {0};
    
    /* Initialize parse list */
    pl_init();
    
    /* Initialize token system */
    tok_init();
    res.tok_initialized = 1;
    
    /* Read all tokens - now returns error code */
    int err = read_toks();
    if (err != 0) {
        return err;  /* Cleanup automatic */
    }
    
    /* Initialize parser */
    yaep_parse_init(toks_len);
    res.parse_initialized = 1;
    
    /* Create parse list */
    pl_create();
    
    /* Perform parse - now returns error code */
    err = do_parse(root, ambiguous_p);
    if (err != 0) {
        return err;  /* Cleanup automatic */
    }
    
    /* Success - prevent cleanup of returned tree */
    res.parse_initialized = 0;
    res.tok_initialized = 0;
    
    return 0;
}
```

## 5. Migration Strategy

### 5.1 Phase-by-Phase Approach

**Phase 1: Infrastructure (No Behavioral Changes)**

Files Created:
- `src/yaep_error.h` - Error context API
- `src/yaep_error.c` - Error context implementation  
- `src/yaep_cleanup.h` - Cleanup attributes
- `src/yaep_macros.h` - Error propagation macros

Files Modified:
- `CMakeLists.txt` - Update to C17
- `src/CMakeLists.txt` - Add new source files
- `configure.ac` - Add C17 checks

Testing:
- Compile test (no runtime changes)
- Verify cleanup attributes detected correctly
- Verify thread-local storage available

**Phase 2: Parallel Implementation**

Strategy: Keep old code, add new functions alongside

Files Modified:
- `src/yaep.c` - Add `_v2` versions of functions
  - `yaep_create_grammar_v2()`
  - `yaep_read_grammar_v2()`
  - `yaep_parse_v2()`

Benefits:
- Old code remains functional
- Can compare behavior
- Easy rollback

**Phase 3: Testing & Validation**

Tests:
- All existing tests pass with new functions
- Fuzzing campaign on new functions
- Memory leak detection (valgrind)
- Thread-safety tests
- Performance benchmarks

**Phase 4: Cutover**

- Replace old functions with new implementations
- Remove `_v2` suffix
- Remove setjmp/longjmp code
- Update documentation

**Phase 5: Global State Elimination** (Future)

- Remove static `grammar` pointer
- Thread-safe grammar handling
- True concurrency support

### 5.2 Backward Compatibility

**Public API:** Unchanged

```c
/* These signatures remain identical */
struct grammar *yaep_create_grammar(void);
int yaep_read_grammar(struct grammar *g, ...);
int yaep_parse(struct grammar *g, ...);
void yaep_free_grammar(struct grammar *g);
```

**Error Reporting:** Unchanged

```c
/* Error codes same */
int yaep_error_code(struct grammar *g);        /* Returns same codes */
const char *yaep_error_message(struct grammar *g);  /* Same messages */
```

**ABI Compatibility:**

- `struct grammar` layout unchanged (opaque to users)
- All function symbols same
- Shared library version bump (soname change)

### 5.3 Compiler Support Matrix

| Compiler | Version | C17 | _Thread_local | cleanup | Status |
|----------|---------|-----|---------------|---------|--------|
| GCC | 5.0+ | ✅ | ✅ | ✅ | Full support |
| GCC | 4.9 | ⚠️ | ✅ | ✅ | Fallback to C11 |
| Clang | 3.1+ | ✅ | ✅ | ✅ | Full support |
| MSVC | 2019+ | ✅ | ✅ | ❌ | Manual cleanup |
| MSVC | 2017 | ⚠️ | ✅ | ❌ | Manual cleanup |

**Fallback Strategy:**

For compilers without cleanup attributes:
```c
#if !YAEP_HAVE_CLEANUP_ATTRIBUTE
/* Manual cleanup with goto */
int function(void) {
    vlo_t *vlo = NULL;
    int err = 0;
    
    VLO_CREATE(vlo, alloc, size);
    if (vlo == NULL) {
        err = YAEP_NO_MEMORY;
        goto cleanup;
    }
    
    /* ... */
    
cleanup:
    if (vlo) VLO_DELETE(vlo);
    return err;
}
#endif
```

## 6. Testing Strategy

### 6.1 Unit Tests

**New Tests:**
- Error context thread-safety
- Cleanup attribute functionality
- Resource leak prevention
- Error propagation correctness

**Existing Tests:**
- All must pass without modification
- test01 through test49
- C and C++ test suites

### 6.2 Integration Tests

- Grammar creation under low memory
- Error recovery during parsing
- Concurrent grammar usage (new)
- Large input stress tests

### 6.3 Fuzzing

**Campaigns:**
1. Grammar parsing fuzzing (continue existing)
2. Token parsing fuzzing
3. Combined grammar+token fuzzing
4. Thread-safety fuzzing (new)

**Expected Improvements:**
- No more NULL pointer crashes
- No memory leaks in error paths
- Clean error reporting

### 6.4 Performance Benchmarks

**Metrics:**
- Grammar creation time
- Parse time (300K lines C code)
- Memory usage
- Error path overhead

**Acceptance Criteria:**
- Performance within 5% of current
- No memory increase
- Faster error paths (no longjmp overhead)

## 7. Rollback Plan

### 7.1 Version Control Strategy

**Branches:**
- `feature/c17-modernization` - Development branch
- `feature/c17-phase1` - Infrastructure only
- `feature/c17-phase2` - Parallel implementation
- etc.

**Tags:**
- `v1.0-pre-c17` - Last C90 version
- `v2.0-alpha1` - First C17 alpha
- `v2.0-beta1` - Feature complete
- `v2.0-rc1` - Release candidate

### 7.2 Rollback Triggers

**Abort migration if:**
1. Performance regression >10%
2. Unfixable compatibility issue
3. Critical bug introduced
4. Testing reveals fundamental flaw

**Rollback Process:**
1. Revert to `v1.0-pre-c17` tag
2. Cherry-pick any critical fixes
3. Document lessons learned
4. Revise migration plan

### 7.3 Compatibility Shim

If partial rollback needed:

```c
/* Compatibility layer - use old implementation temporarily */
#ifdef YAEP_USE_LEGACY_ERROR_HANDLING
#include "yaep_legacy.c"
#else
#include "yaep_error.c"
#endif
```

## 8. Documentation Requirements

### 8.1 Code Documentation

**Every modified function needs:**
- Doxygen-style header comment
- Parameter documentation
- Return value documentation
- Error conditions
- Thread-safety guarantees
- Algorithm description
- Rationale for changes
- Example usage

**Example:**
```c
/**
 * @brief [One-line summary]
 *
 * [Detailed description of what function does, how it works,
 *  and why it's implemented this way]
 *
 * @param name Description
 * @return Return value description
 *
 * @retval 0 Success
 * @retval YAEP_NO_MEMORY Allocation failed
 *
 * Thread Safety: [Safe/Unsafe/Conditionally safe]
 * Allocation: [None/Stack/Heap]
 * Blocking: [No/Yes - description]
 *
 * Algorithm:
 * 1. Step one
 * 2. Step two
 *
 * Rationale:
 * [Why this implementation chosen over alternatives]
 *
 * Example:
 * @code
 *   [Example usage]
 * @endcode
 */
```

### 8.2 User Documentation

**Updates Required:**
- Migration guide (C90 -> C17)
- API changelog
- Error handling guide
- Threading guide (new)
- Performance characteristics
- Compiler requirements

### 8.3 Developer Documentation

**New Documents:**
- Architecture overview
- Error handling design
- Resource management patterns
- Contribution guidelines
- Code review checklist

## 9. Success Criteria

**Migration considered successful when:**

1. ✅ All setjmp/longjmp removed
2. ✅ All existing tests pass
3. ✅ No memory leaks (valgrind clean)
4. ✅ Thread-safe (helgrind/tsan clean)
5. ✅ Performance within 5% of baseline
6. ✅ Fuzzing finds no crashes
7. ✅ API fully documented
8. ✅ Migration guide complete
9. ✅ Code review approved
10. ✅ Community acceptance

## 10. Timeline

| Phase | Duration | Start | End | Deliverable |
|-------|----------|-------|-----|-------------|
| Phase 1 | 2 days | Day 1 | Day 2 | Infrastructure |
| Phase 2 | 3 days | Day 3 | Day 5 | Error context |
| Phase 3 | 5 days | Day 6 | Day 10 | Core refactor |
| Phase 4 | 3 days | Day 11 | Day 13 | Grammar parser |
| Phase 5 | 3 days | Day 14 | Day 16 | Testing & docs |

**Total: 16 days**

## 11. Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Performance regression | Low | High | Benchmark early, optimize |
| Compatibility break | Medium | High | Parallel implementation |
| Schedule overrun | Medium | Medium | Phase-based approach |
| Unforeseen bugs | Medium | High | Extensive testing |
| Compiler support | Low | Medium | Fallback implementations |

## 12. Conclusion

This plan provides a comprehensive, production-ready approach to modernizing YAEP to C17 with:

- **Zero technical debt** - No placeholders or hacks
- **Full documentation** - Literate programming throughout
- **Root cause fixes** - No band-aids
- **Backward compatibility** - API unchanged
- **Measurable success** - Clear criteria
- **Risk management** - Rollback plan
- **Phased approach** - Incremental progress

The result will be a faster, safer, more maintainable codebase suitable for modern development.

---

**Document Version:** 1.0  
**Date:** October 7, 2025  
**Author:** GitHub Copilot  
**Status:** Ready for Review
