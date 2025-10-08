# Safe Refactorings for Future Enhancements

## Executive Summary

While modernizing YAEP to C17 and eliminating setjmp/longjmp, we have identified **strategic refactorings** that will facilitate future optimizations like Leo's algorithm, improved caching, and concurrent parsing—without changing current behavior.

**Philosophy:** Make the code more modular and extensible now, enabling future features with minimal disruption.

**Constraints:**
- No behavioral changes (output identical)
- No performance regression
- Maintain API compatibility
- Production-ready code only

---

## Table of Contents

1. [Global State Elimination (Phase 6)](#1-global-state-elimination-phase-6)
2. [Context Structure Consolidation](#2-context-structure-consolidation)
3. [Parser Algorithm Abstraction](#3-parser-algorithm-abstraction)
4. [Optimization Framework](#4-optimization-framework)
5. [Memory Pool Architecture](#5-memory-pool-architecture)
6. [Metrics and Profiling Infrastructure](#6-metrics-and-profiling-infrastructure)
7. [Implementation Phases](#7-implementation-phases)

---

## 1. Global State Elimination (Phase 6)

### Current Problem

**Massive global state** (from `src/yaep.c:192-208`):

```c
/* Grammar state */
static struct grammar *grammar;
static struct symbs *symbs_ptr;
static struct term_sets *term_sets_ptr;
static struct rules *rules_ptr;

/* Parse state */
static int (*read_token)(void **attr);
static void (*syntax_error)(...);
static void *(*parse_alloc)(int nmemb);
static void (*parse_free)(void *mem);

/* Plus ~30 more static variables throughout the file */
static struct tok *toks;
static vlo_t *toks_vlo;
static struct sit ***sit_table;
static vlo_t *sit_table_vlo;
static os_t *sits_os;
static struct set *new_set;
static struct set_core *new_core;
static struct sit **new_sits;
/* ... and many more ... */
```

**Impact:**
- ❌ Prevents concurrent parsing (different threads, different grammars)
- ❌ Prevents reentrant calls
- ❌ Makes testing difficult (global state persists)
- ❌ Blocks future optimizations (can't have per-grammar caches)
- ❌ Hard to reason about lifetime and ownership

### Solution: Parse Context Structure

**Create comprehensive parse context:**

```c
/**
 * @brief Complete parsing context
 *
 * Encapsulates ALL state needed for parsing, eliminating global variables.
 * This enables concurrent parsing, reentrancy, and per-context optimizations.
 *
 * Design Principles:
 * - Zero global mutable state
 * - Clear ownership semantics
 * - Explicit lifetime management
 * - Cache-friendly layout (hot data together)
 *
 * Future Extensions:
 * - Per-context optimization state (Leo items, cached sets, etc.)
 * - Per-context metrics (for profiling)
 * - Thread-local caches
 */
typedef struct yaep_parse_context {
    /* Grammar (immutable during parse) */
    struct grammar *grammar;
    struct symbs *symbs;
    struct term_sets *term_sets;
    struct rules *rules;
    
    /* Callbacks (immutable during parse) */
    int (*read_token)(void **attr);
    void (*syntax_error)(int, void *, int, void *, int, void *);
    void *(*parse_alloc)(int nmemb);
    void (*parse_free)(void *mem);
    
    /* Token state */
    struct tok *toks;
    int toks_len;
    int tok_curr;
    vlo_t *toks_vlo;
    
    /* Situation state */
    struct sit ***sit_table;
    vlo_t *sit_table_vlo;
    os_t *sits_os;
    int n_all_sits;
    
    /* Set state */
    struct set *new_set;
    struct set_core *new_core;
    struct sit **new_sits;
    vlo_t *sets_vlo;
    
    /* Parse list state */
    vlo_t *pl_vlo;
    int pl_curr;
    
    /* Statistics (for debugging/profiling) */
    struct {
        int n_sets;
        int n_situations;
        int n_cores;
        int cache_hits;
        int cache_misses;
    } stats;
    
    /* Future extension point: optimization state */
    void *optimization_state;  /* Cast to specific optimizer */
    
} yaep_parse_context_t;
```

**Migration Strategy:**

```c
/* Phase 6a: Create context, still use globals */
yaep_parse_context_t *ctx = yaep_create_parse_context(grammar);
grammar = ctx->grammar;  /* Set global from context */
/* ... existing code ... */

/* Phase 6b: Pass context to internal functions */
static int tok_add(yaep_parse_context_t *ctx, int code, void *attr);

/* Phase 6c: Remove globals entirely */
/* All functions take context as first parameter */
```

**Benefits:**
- ✅ Enables concurrent parsing (each context independent)
- ✅ Enables reentrancy
- ✅ Clear ownership (context owns all parse state)
- ✅ Easier testing (isolated state)
- ✅ Foundation for per-context optimizations
- ✅ Cache-friendly (locality of reference)

**Complexity:** High (touches every function)  
**Risk:** Medium (extensive changes, but mechanical)  
**Timeline:** 5-7 days  
**Dependencies:** Phase 1-5 complete

---

## 2. Context Structure Consolidation

### Current Problem

**Fragmented data structures:**
- `struct grammar` - grammar definition
- `struct symbs` - symbol table
- `struct rules` - rule list  
- `struct term_sets` - terminal sets
- All separate, passed around individually

**Issues:**
- Many function parameters (error-prone)
- Unclear ownership
- Inefficient (cache misses)

### Solution: Hierarchical Contexts

```c
/**
 * @brief Immutable grammar context
 *
 * Contains everything needed to describe a grammar, immutable after
 * grammar finalization. Can be shared across multiple parse contexts.
 *
 * Thread Safety: Immutable after finalization - safe to share
 * Lifetime: Until yaep_free_grammar()
 */
typedef struct {
    /* Configuration (immutable) */
    int lookahead_level;
    int debug_level;
    int one_parse_p;
    int cost_p;
    int error_recovery_p;
    int recovery_token_matches;
    
    /* Grammar data (immutable) */
    struct symbs *symbs;
    struct rules *rules;
    struct term_sets *term_sets;
    
    /* Cached analysis (computed once, immutable) */
    struct {
        int *first_sets;      /* FIRST sets (precomputed) */
        int *follow_sets;     /* FOLLOW sets (precomputed) */
        int *nullable;        /* Nullable nonterminals */
        int max_rule_len;     /* Longest rule */
    } analysis;
    
    /* Allocator */
    YaepAllocator *alloc;
    
    /* Error state (mutable, but only via API) */
    int error_code;
    char error_message[YAEP_MAX_ERROR_MESSAGE_LENGTH];
    
} yaep_grammar_context_t;
```

**Benefits:**
- ✅ Single pointer to pass around (instead of 3-4)
- ✅ Clear immutability (const after finalization)
- ✅ Better cache locality
- ✅ Can share grammar across parses
- ✅ Extensible (add cached analysis)

**Complexity:** Medium  
**Risk:** Low (mostly mechanical refactoring)  
**Timeline:** 2-3 days  
**Dependencies:** Phase 6

---

## 3. Parser Algorithm Abstraction

### Current Situation

**Algorithm is hardcoded throughout:**
- Earley algorithm directly implemented
- No abstraction for different strategies
- Makes experimentation difficult

**From doc/Internals.txt:**
> "We don't use Leo Joop's approach"
> "We don't use PEP (Practical Earley Parser)"

These optimizations would require significant refactoring if added later.

### Solution: Strategy Pattern for Parsing

```c
/**
 * @brief Parser strategy interface
 *
 * Abstraction for different parsing algorithms/optimizations.
 * Allows experimenting with Leo's algorithm, PEP, or custom
 * optimizations without changing the core.
 *
 * Design:
 * - Default: Current Earley implementation
 * - Leo: Earley with Leo's right-recursion optimization
 * - PEP: Practical Earley Parser variant
 * - Custom: User-defined strategies
 */
typedef struct yaep_parser_strategy {
    const char *name;
    
    /**
     * @brief Initialize parser with this strategy
     * @return 0 on success, error code on failure
     */
    int (*init)(yaep_parse_context_t *ctx);
    
    /**
     * @brief Build initial set (Scanner phase)
     */
    int (*scanner)(yaep_parse_context_t *ctx, int tok_num);
    
    /**
     * @brief Prediction phase
     */
    int (*predictor)(yaep_parse_context_t *ctx, struct set *set);
    
    /**
     * @brief Completion phase
     */
    int (*completer)(yaep_parse_context_t *ctx, struct set *set);
    
    /**
     * @brief Check if parse complete
     */
    int (*check_complete)(yaep_parse_context_t *ctx, 
                         struct yaep_tree_node **root);
    
    /**
     * @brief Cleanup strategy state
     */
    void (*finalize)(yaep_parse_context_t *ctx);
    
    /* Strategy-specific state */
    void *strategy_state;
    
} yaep_parser_strategy_t;

/* Builtin strategies */
extern const yaep_parser_strategy_t yaep_earley_standard;
extern const yaep_parser_strategy_t yaep_earley_leo;  /* Future */
extern const yaep_parser_strategy_t yaep_pep;         /* Future */
```

**Usage:**

```c
/* Default - current behavior */
yaep_parse(grammar, ...);  /* Uses yaep_earley_standard */

/* Future: Enable Leo optimization */
yaep_set_parser_strategy(grammar, &yaep_earley_leo);
yaep_parse(grammar, ...);  /* Uses Leo optimization */
```

**Implementation Plan:**

**Phase A: Extract Current Algorithm**
```c
/* Wrap current implementation in strategy interface */
static int earley_scanner(yaep_parse_context_t *ctx, int tok_num) {
    /* Current scanner code */
}

const yaep_parser_strategy_t yaep_earley_standard = {
    .name = "Earley (standard)",
    .scanner = earley_scanner,
    .predictor = earley_predictor,
    .completer = earley_completer,
    /* ... */
};
```

**Phase B: Add Strategy Selection**
```c
/* Add to grammar context */
struct grammar {
    /* ... existing fields ... */
    const yaep_parser_strategy_t *strategy;
};
```

**Phase C: Future - Add Leo Optimization**
```c
/**
 * Leo's optimization for right-recursive grammars
 * 
 * Reduces O(n²) to O(n) for right recursion by caching
 * "deterministic" reduction paths.
 *
 * See: "A general context-free parsing algorithm running
 * in linear time on every LR(k) grammar without using
 * lookahead" - Joop M.I.M. Leo (1991)
 */
static int leo_completer(yaep_parse_context_t *ctx, struct set *set) {
    /* Check if this is a deterministic reduction */
    if (is_deterministic_reduction(ctx, set)) {
        /* Use Leo item instead of full Earley item */
        return complete_with_leo_item(ctx, set);
    } else {
        /* Fall back to standard Earley */
        return earley_completer(ctx, set);
    }
}
```

**Benefits:**
- ✅ Easy to add Leo optimization later
- ✅ Can A/B test algorithms
- ✅ Easier to maintain (clear separation)
- ✅ Users can provide custom strategies
- ✅ No performance cost (function pointers with likely branch prediction)

**Complexity:** Medium  
**Risk:** Low (current code becomes default strategy)  
**Timeline:** 3-4 days  
**Dependencies:** Phase 6 (needs context structure)

---

## 4. Optimization Framework

### Current Situation

**Ad-hoc optimizations:**
```c
#define USE_SET_HASH_TABLE      /* Enabled */
/* #define TRANSITIVE_TRANSITION */  /* Disabled */
```

Optimizations are compile-time only, can't be toggled at runtime.

### Solution: Runtime Optimization Flags

```c
/**
 * @brief Optimization flags
 *
 * Controls various optimizations at runtime. Allows profiling
 * and A/B testing without recompilation.
 */
typedef enum {
    YAEP_OPT_NONE           = 0,
    YAEP_OPT_SET_CACHE      = (1 << 0),  /* Use set hash table */
    YAEP_OPT_DISTANCE_CACHE = (1 << 1),  /* Cache distance vectors */
    YAEP_OPT_LEO            = (1 << 2),  /* Leo's algorithm (future) */
    YAEP_OPT_PARALLEL       = (1 << 3),  /* Parallel set building (future) */
    
    YAEP_OPT_ALL = 0xFFFF,
    YAEP_OPT_DEFAULT = (YAEP_OPT_SET_CACHE | YAEP_OPT_DISTANCE_CACHE)
} yaep_optimization_flags_t;

/**
 * @brief Set optimization flags for grammar
 *
 * @param g Grammar context
 * @param flags Bitwise OR of optimization flags
 * @return 0 on success, error if incompatible flags
 *
 * Example:
 * @code
 *   // Enable set caching and Leo optimization
 *   yaep_set_optimizations(g, YAEP_OPT_SET_CACHE | YAEP_OPT_LEO);
 * @endcode
 */
int yaep_set_optimizations(struct grammar *g, 
                          yaep_optimization_flags_t flags);
```

**Benefits:**
- ✅ Runtime toggling (no recompilation)
- ✅ A/B testing in production
- ✅ Per-grammar optimization tuning
- ✅ Easier debugging (disable optimizations)
- ✅ Profiling-guided optimization

**Complexity:** Low  
**Risk:** Low  
**Timeline:** 1-2 days  
**Dependencies:** None

---

## 5. Memory Pool Architecture

### Current Problem

**Fragmented allocation:**
- Many small allocations (situations, sets, cores)
- Lots of malloc/free overhead
- Poor cache locality
- Allocation callbacks for each allocation

### Solution: Arena/Pool Allocator

```c
/**
 * @brief Memory pool for parse-time allocations
 *
 * Amortizes allocation cost by allocating large blocks and
 * sub-allocating from them. Much faster than malloc/free for
 * small, short-lived objects.
 *
 * Characteristics:
 * - Fast allocation: O(1) pointer bump
 * - Fast deallocation: Free entire pool at once
 * - Good cache locality: Sequential allocations are adjacent
 * - Low overhead: No per-allocation metadata
 *
 * Use Cases:
 * - Situations (many small allocations)
 * - Set cores
 * - Parse tree nodes
 * - Temporary work buffers
 */
typedef struct yaep_memory_pool {
    YaepAllocator *base_alloc;  /* Underlying allocator */
    
    struct pool_block {
        struct pool_block *next;
        size_t size;
        size_t used;
        char data[];  /* Flexible array member (C99) */
    } *blocks;
    
    /* Current block for fast allocation */
    struct pool_block *current;
    
    /* Statistics */
    size_t total_allocated;
    size_t total_used;
    int n_blocks;
    
} yaep_memory_pool_t;

/**
 * @brief Allocate from pool
 *
 * Fast O(1) allocation by bumping pointer in current block.
 * If block full, allocate new block (amortized O(1)).
 *
 * @param pool Memory pool
 * @param size Bytes to allocate
 * @return Pointer to allocated memory, or NULL on error
 *
 * Performance: ~10x faster than malloc for small allocations
 * Memory Overhead: ~1% (block headers only)
 */
static inline void *yaep_pool_alloc(yaep_memory_pool_t *pool, size_t size) {
    /* Align to pointer size */
    size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
    
    /* Check if current block has space */
    if (pool->current->used + size <= pool->current->size) {
        void *ptr = pool->current->data + pool->current->used;
        pool->current->used += size;
        return ptr;
    }
    
    /* Need new block */
    return yaep_pool_alloc_slow(pool, size);
}

/**
 * @brief Reset pool (free all allocations)
 *
 * Much faster than freeing individual allocations.
 * Keeps blocks for reuse.
 *
 * @param pool Memory pool to reset
 *
 * Performance: O(1) - just reset used counters
 */
void yaep_pool_reset(yaep_memory_pool_t *pool);
```

**Usage Pattern:**

```c
/* Create pool for parse */
yaep_memory_pool_t *pool = yaep_pool_create(alloc, 64*1024);

/* Allocate situations from pool */
struct sit *sit = yaep_pool_alloc(pool, sizeof(*sit));

/* Parse complete - free everything at once */
yaep_pool_destroy(pool);  /* Much faster than individual frees */
```

**Benefits:**
- ✅ 5-10x faster allocation for small objects
- ✅ Better cache locality
- ✅ Simpler cleanup (free pool, not individual objects)
- ✅ Lower memory overhead
- ✅ Reduces allocator contention in multi-threaded scenarios

**Complexity:** Medium  
**Risk:** Low (opt-in, fallback to regular malloc)  
**Timeline:** 3-4 days  
**Dependencies:** None (orthogonal)

---

## 6. Metrics and Profiling Infrastructure

### Current Situation

**Limited observability:**
- Some debug output (`debug_level`)
- No runtime metrics
- Hard to identify bottlenecks
- Can't measure optimization impact

### Solution: Comprehensive Metrics

```c
/**
 * @brief Parse metrics
 *
 * Detailed metrics for profiling and optimization.
 * Zero overhead when disabled (compile-time flag).
 */
typedef struct yaep_parse_metrics {
    /* Algorithm metrics */
    struct {
        int n_sets;              /* Total Earley sets created */
        int n_situations;        /* Total situations created */
        int n_cores;             /* Total cores created */
        int n_predictor_calls;   /* Predictor invocations */
        int n_completer_calls;   /* Completer invocations */
        int n_scanner_calls;     /* Scanner invocations */
    } algorithm;
    
    /* Cache metrics */
    struct {
        int set_cache_hits;      /* Set reuse from cache */
        int set_cache_misses;    /* Set not in cache */
        int dist_cache_hits;     /* Distance vector reuse */
        int dist_cache_misses;   /* Distance vector computed */
    } cache;
    
    /* Memory metrics */
    struct {
        size_t peak_memory;      /* Peak memory usage */
        size_t current_memory;   /* Current memory usage */
        int n_allocations;       /* Total allocations */
        int n_frees;             /* Total frees */
    } memory;
    
    /* Timing metrics (if enabled) */
    struct {
        uint64_t scanner_ns;     /* Time in scanner */
        uint64_t predictor_ns;   /* Time in predictor */
        uint64_t completer_ns;   /* Time in completer */
        uint64_t total_ns;       /* Total parse time */
    } timing;
    
} yaep_parse_metrics_t;

/**
 * @brief Get parse metrics
 *
 * Returns metrics from last parse. Useful for profiling and
 * optimization tuning.
 *
 * @param g Grammar that was parsed
 * @param metrics Output: metrics structure
 * @return 0 on success, error if metrics disabled
 *
 * Example:
 * @code
 *   yaep_parse_metrics_t metrics;
 *   yaep_get_metrics(g, &metrics);
 *   printf("Cache hit rate: %.2f%%\n", 
 *          100.0 * metrics.cache.set_cache_hits / 
 *                 (metrics.cache.set_cache_hits + 
 *                  metrics.cache.set_cache_misses));
 * @endcode
 */
int yaep_get_metrics(struct grammar *g, yaep_parse_metrics_t *metrics);

/**
 * @brief Enable/disable metrics collection
 *
 * Metrics have ~2-5% overhead when enabled.
 */
void yaep_set_metrics_enabled(struct grammar *g, int enabled);
```

**Benefits:**
- ✅ Identify bottlenecks
- ✅ Measure optimization impact
- ✅ Production monitoring
- ✅ Regression detection
- ✅ Tuning guidance

**Complexity:** Low  
**Risk:** Low  
**Timeline:** 2 days  
**Dependencies:** None

---

## 7. Implementation Phases

### Recommended Phasing

**Phase 6: Global State Elimination**
- Duration: 5-7 days
- Risk: Medium
- Impact: Enables all future work
- Dependencies: Phases 1-5 complete

**Phase 7: Context Consolidation**
- Duration: 2-3 days
- Risk: Low
- Impact: Better cache locality, cleaner API
- Dependencies: Phase 6

**Phase 8: Optimization Framework**
- Duration: 1-2 days
- Risk: Low
- Impact: Runtime optimization control
- Dependencies: None (can do anytime)

**Phase 9: Metrics Infrastructure**
- Duration: 2 days
- Risk: Low
- Impact: Profiling, optimization validation
- Dependencies: None (can do anytime)

**Phase 10: Parser Strategy Abstraction**
- Duration: 3-4 days
- Risk: Low
- Impact: Enables Leo, PEP, custom algorithms
- Dependencies: Phase 6-7

**Phase 11: Memory Pool (Optional)**
- Duration: 3-4 days
- Risk: Low
- Impact: 5-10x faster allocation
- Dependencies: None (orthogonal)

**Total Additional: 16-22 days**

### Incremental Approach

**Immediate (Include in C17 Migration):**
- ✅ Optimization flags (Phase 8) - 1-2 days
- ✅ Metrics infrastructure (Phase 9) - 2 days

**Near-term (After C17 Complete):**
- Global state elimination (Phase 6) - Essential for concurrency
- Context consolidation (Phase 7) - Natural follow-on

**Medium-term (When Ready for Advanced Features):**
- Parser strategy abstraction (Phase 10)
- Memory pool architecture (Phase 11)

---

## 8. Specific Feature Enablement

### Leo's Algorithm

**Requirements:**
- ✅ Parser strategy abstraction (Phase 10)
- ✅ Global state elimination (Phase 6)
- Extension point for Leo items

**Estimated Effort:** 5-7 days after prerequisites  
**Expected Benefit:** O(n) instead of O(n²) for right-recursive grammars

### Concurrent Parsing

**Requirements:**
- ✅ Global state elimination (Phase 6)
- ✅ Thread-safe error handling (already in C17 plan)
- Per-thread parse contexts

**Estimated Effort:** 3-4 days after Phase 6  
**Expected Benefit:** N-way parallelism for N grammars

### PEP (Practical Earley Parser)

**Requirements:**
- ✅ Parser strategy abstraction (Phase 10)
- Additional bookkeeping for deterministic reductions

**Estimated Effort:** 7-10 days after Phase 10  
**Expected Benefit:** 2-3x speedup on typical grammars

### Incremental Parsing

**Requirements:**
- ✅ Global state elimination (Phase 6)
- ✅ Memory pool (Phase 11) - for efficient state snapshots
- State serialization

**Estimated Effort:** 10-14 days  
**Expected Benefit:** Reparse only changed portions

---

## 9. Risk Assessment

| Refactoring | Complexity | Risk | Reversible? | Testing Burden |
|-------------|-----------|------|-------------|----------------|
| Global state elimination | High | Medium | Yes (branch) | High |
| Context consolidation | Medium | Low | Yes | Medium |
| Parser strategy | Medium | Low | Yes | Medium |
| Optimization flags | Low | Low | Yes | Low |
| Memory pool | Medium | Low | Yes (opt-in) | Medium |
| Metrics | Low | Low | Yes | Low |

**Mitigation Strategies:**
- Incremental migration (phases)
- Parallel implementation (keep old code)
- Extensive testing at each phase
- Performance benchmarking
- Easy rollback (branches, feature flags)

---

## 10. Conclusion

These refactorings are **strategic investments** that:

1. **Don't change current behavior** - All output identical
2. **Enable future optimizations** - Leo, PEP, parallelism
3. **Improve code quality** - Better structure, clearer ownership
4. **Are low-risk** - Incremental, testable, reversible
5. **Pay for themselves** - Better maintainability

**Recommendation:**

**Include in C17 Migration:**
- Optimization flags (1-2 days)
- Metrics infrastructure (2 days)

**Do Next (Phase 6-7):**
- Global state elimination (5-7 days)
- Context consolidation (2-3 days)

**Future (When Ready for Advanced Features):**
- Parser strategy abstraction
- Memory pool architecture
- Specific algorithms (Leo, PEP, etc.)

This creates a **solid foundation** for future enhancements without technical debt.

---

**Document Version:** 1.0  
**Date:** October 7, 2025  
**Status:** Ready for Review
