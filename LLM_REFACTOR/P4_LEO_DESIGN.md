# Phase P4: Leo's Right-Recursion Optimization - Design Document

**Date:** 2025-10-09  
**Phase:** P4 - Leo's Right-Recursion Optimization  
**Reference:** Leo, Joop (1991). "A general context-free parsing algorithm running in linear time on every LR(k) grammar without using lookahead"  
**Branch:** `earley-optimization`

---

## Executive Summary

Implement Leo's optimization to reduce complexity from **O(n³) to O(n²)** for right-recursive grammars, and from **O(n²) to O(n)** for LR(k) grammars.

**Expected Impact:**
- Right-recursive grammars: **10-100x speedup**
- LR(k) grammars: **Linear time** parsing
- Typical grammars: **10-30% speedup**

---

## Problem Statement

### Standard Earley Parser Issues

**Right-Recursive Grammar Example:**
```
S → S 'a' | 'b'
```

**Input:** `b a a a ... a` (n symbols)

**Standard Earley Behavior:**
- Each completion propagates through ALL previous items
- Creates O(n²) items for n input symbols
- Total complexity: O(n³)

**Example (input "b a a a"):**
```
Set 0: [S → •S 'a', 0]  [S → •'b', 0]
Set 1: [S → 'b'•, 0]    [S → S•'a', 0]  ← Complete S
Set 2: [S → S 'a'•, 0]  [S → S•'a', 0]  ← Complete S again
Set 3: [S → S 'a'•, 0]  [S → S•'a', 0]  ← Complete S again
Set 4: [S → S 'a'•, 0]  [S → S•'a', 0]  ← Complete S again
```

**Problem:** Each completion re-derives items that were already derived.

---

## Leo's Solution

### Core Insight

**Observation:** In right-recursive grammars, we repeatedly complete the same nonterminal,  
creating chains of items that can be collapsed into a single "Leo item".

**Leo Item:** Represents the **topmost** item in a right-recursive chain,  
eliminating the need to explicitly create and process intermediate items.

### Algorithm Overview

**Key Concepts:**

1. **Deterministic Reduction Path:**
   - A sequence of reductions where each step has only ONE possible reduction
   - Example: A → B, B → C, C → D (only one rule for each nonterminal)

2. **Topmost Item:**
   - The final item in a deterministic reduction path
   - Represents the entire chain compactly

3. **Leo Item:**
   - Special marker storing the topmost item
   - Avoids creating intermediate items
   - Updated during completion instead of creating new items

### Conditions for Leo Optimization

Leo item can be created when:

1. **Complete item:** `[A → α•, i]` where dot is at end
2. **Unique parent:** Exactly ONE item `[B → γ•A β, j]` waiting for A
3. **Deterministic:** No other items in set `i` expect A at the dot
4. **Right-recursive pattern:** This configuration repeats

**If conditions met:**
- Create Leo item `LEO(B → γ A •β, j, A)` in current set
- Skip creating standard Earley item
- On next completion, use Leo item directly

---

## Data Structures

### Leo Item Structure

```c
/**
 * Leo item for right-recursion optimization
 *
 * Represents the topmost item in a deterministic reduction path,
 * eliminating redundant intermediate items in right-recursive grammars.
 *
 * INVARIANT:
 * - Only created when exactly one item waits for the completed nonterminal
 * - Stored in Leo item table indexed by (set, nonterminal)
 * - Replaces chain of standard Earley items
 *
 * EXAMPLE:
 * Grammar: S → S 'a' | 'b'
 * Input: b a a a
 *
 * Without Leo:
 *   Set 1: [S → S•'a', 0]
 *   Set 2: [S → S•'a', 0]  [S → S 'a'•, 0]
 *   Set 3: [S → S•'a', 0]  [S → S 'a'•, 0]  [S → S 'a'•, 1]
 *   ... (O(n²) items)
 *
 * With Leo:
 *   Set 1: LEO(S → S•'a', 0, S)
 *   Set 2: LEO(S → S•'a', 0, S)
 *   Set 3: LEO(S → S•'a', 0, S)
 *   ... (O(n) Leo items instead of O(n²) standard items)
 *
 * REFERENCE: Leo (1991) Section 3, Definition 3.1
 */
struct leo_item {
    /** The situation (rule + position) this Leo item represents */
    struct sit *sit;
    
    /** Origin set where this derivation chain started */
    int origin;
    
    /** The nonterminal symbol this Leo item is for */
    struct symb *symbol;
    
    /** Link for hash table chaining */
    struct leo_item *next;
};
```

### Leo Item Table

```c
/**
 * Leo item table - Hash table storing Leo items
 *
 * KEY: (current_set_index, nonterminal_symbol)
 * VALUE: Leo item representing topmost item for that nonterminal
 *
 * USAGE:
 * - During completion, check if Leo item exists for completed nonterminal
 * - If exists, use Leo item instead of creating standard items
 * - If doesn't exist but conditions met, create new Leo item
 *
 * SIZE: O(|N| × sets) where |N| = number of nonterminals
 * In practice: Small (most nonterminals don't have Leo items)
 */
static hash_table_t *leo_item_table;
```

---

## Algorithm Details

### Detection Phase (During Completion)

**When completing item `[A → α•, i]` in set `k`:**

```c
// STEP 1: Find all items waiting for A in set i
items_waiting_for_A = find_items_with_dot_before(set[i], A);

// STEP 2: Check if exactly ONE item waiting
if (count(items_waiting_for_A) == 1) {
    parent_item = items_waiting_for_A[0];  // [B → γ•A β, j]
    
    // STEP 3: Check if this creates right-recursive pattern
    // (i.e., advancing past A puts us in a deterministic reduction path)
    if (is_deterministic_after_A(parent_item)) {
        // CONDITIONS MET: Create/update Leo item
        create_or_update_leo_item(set[k], A, parent_item, j);
        
        // SKIP creating standard Earley items
        return;
    }
}

// CONDITIONS NOT MET: Fall back to standard Earley completion
standard_completion(A, i, k);
```

### Leo Item Creation

```c
/**
 * Create or update Leo item for nonterminal in current set
 *
 * @param set Current Earley set
 * @param symbol Completed nonterminal symbol
 * @param parent_sit Parent situation [B → γ•A β, j]
 * @param origin Origin set index j
 */
void create_leo_item(struct set *set, struct symb *symbol, 
                     struct sit *parent_sit, int origin) {
    struct leo_item *leo;
    
    // Check if Leo item already exists for (set, symbol)
    leo = lookup_leo_item(set->set_number, symbol);
    
    if (leo != NULL) {
        // Update existing Leo item
        leo->sit = sit_create(parent_sit->rule, parent_sit->pos + 1, 
                             parent_sit->context);
        leo->origin = origin;
    } else {
        // Create new Leo item
        leo = alloc_leo_item();
        leo->sit = sit_create(parent_sit->rule, parent_sit->pos + 1,
                             parent_sit->context);
        leo->origin = origin;
        leo->symbol = symbol;
        
        // Insert into Leo item table
        insert_leo_item(set->set_number, symbol, leo);
    }
}
```

### Leo Item Usage (Completing with Leo)

```c
/**
 * Complete using Leo item instead of standard completion
 *
 * When we complete nonterminal A and a Leo item exists,
 * we can directly jump to the topmost item instead of
 * propagating through intermediate items.
 */
void complete_with_leo(struct symb *symbol, int complete_origin, int current_set) {
    struct leo_item *leo;
    
    // Look for Leo item in origin set
    leo = lookup_leo_item(complete_origin, symbol);
    
    if (leo != NULL) {
        // Use Leo item: Jump directly to topmost item
        // Add the advanced Leo situation to current set
        set_add_new_nonstart_sit(leo->sit, leo->origin);
        
        // If Leo item's rule is also complete, recursively apply Leo
        if (is_complete(leo->sit)) {
            struct symb *leo_lhs = leo->sit->rule->lhs;
            complete_with_leo(leo_lhs, leo->origin, current_set);
        }
    } else {
        // No Leo item: Fall back to standard completion
        standard_completion(symbol, complete_origin, current_set);
    }
}
```

---

## Implementation Plan

### P4-001: Data Structures (Week 1, Days 1-2)

**Files:**
- `src/yaep.c` (add structures after situation declarations)

**Tasks:**
1. Add `struct leo_item` definition
2. Add Leo item table variables:
   - `static hash_table_t *leo_item_table`
   - `static os_t *leo_items_os` (object stack for leo items)
3. Add Leo item counters (for statistics):
   - `static int n_leo_items`
   - `static int n_leo_completions`

**Testing:**
- Compile verification
- No functionality change

### P4-002: Leo Item Table Operations (Week 1, Days 3-4)

**Functions to implement:**

```c
// Initialize Leo item table
static void leo_init(void);

// Destroy Leo item table
static void leo_finish(void);

// Clear Leo table for new parse
static void leo_clear(void);

// Hash function for (set_num, symbol)
static unsigned int leo_hash(int set_num, struct symb *symbol);

// Lookup Leo item
static struct leo_item *lookup_leo_item(int set_num, struct symb *symbol);

// Insert Leo item
static void insert_leo_item(int set_num, struct symb *symbol, struct leo_item *leo);

// Statistics
static void print_leo_stats(void);
```

**Testing:**
- Unit test Leo table operations
- Create `test/test_leo.c`:
  - Test insert/lookup/clear
  - Test hash distribution
  - Test edge cases (empty table, collisions)

### P4-003: Detection Logic (Week 1, Day 5 - Week 2, Day 2)

**Implement detection of Leo-eligible completions:**

1. **Find items waiting for nonterminal:**
```c
/**
 * Find all items in set with dot before nonterminal
 *
 * @param set Earley set to search
 * @param symbol Nonterminal symbol
 * @return Count of items found (0, 1, or >1)
 */
static int find_items_waiting_for(struct set *set, struct symb *symbol,
                                  struct sit **out_item);
```

2. **Check determinism:**
```c
/**
 * Check if configuration is deterministic
 *
 * Deterministic if only ONE item can reduce to this nonterminal
 * in the current set.
 *
 * @return TRUE if deterministic, FALSE otherwise
 */
static int is_deterministic_reduction(struct set *set, struct symb *symbol);
```

3. **Integrate into completer:**
   - Modify completion logic in `build_new_set()`
   - Before standard completion, check Leo conditions
   - If met, create/update Leo item
   - If not met, proceed with standard completion

**Testing:**
- Test grammars with right-recursion
- Test grammars without right-recursion
- Test mixed grammars
- Verify no behavior change (Leo items created but not yet used)

### P4-004: Leo Completion (Week 2, Days 3-5)

**Implement Leo-based completion:**

1. **Modify completer to use Leo items:**
```c
// In build_new_set(), when completing nonterminal A from set i:
leo = lookup_leo_item(i, A);
if (leo != NULL) {
    // Use Leo item
    complete_with_leo_item(leo, current_set);
} else {
    // Standard completion
    standard_earley_completion(A, i, current_set);
}
```

2. **Handle recursive Leo completions:**
   - When Leo item's situation is also complete
   - Recursively apply Leo completion
   - Prevent infinite loops (cycle detection)

**Testing:**
- Right-recursive grammar tests
- Verify parse results identical to standard Earley
- Verify fewer items created (check statistics)

### P4-005: Testing Suite (Week 3, Days 1-2)

**Create comprehensive test suite:**

1. **test/test_leo_basic.c:**
   - Simple right-recursive grammar: `S → S 'a' | 'b'`
   - Input: "b a a a a"
   - Verify: Correct parse, fewer items with Leo

2. **test/test_leo_complex.c:**
   - Expression grammar with right-recursion
   - Nested structures
   - Verify: Correctness and performance

3. **test/test_leo_mixed.c:**
   - Grammar with both left and right recursion
   - Verify: Leo applied only where appropriate

4. **test/test_leo_disabled.c:**
   - Verify disabling Leo works (fallback to standard)
   - Compare results (should be identical)

**All tests must:**
- Pass with Leo enabled
- Pass with Leo disabled  
- Produce identical parse trees
- Show performance improvement (measure item count)

### P4-006: Benchmarking (Week 3, Days 3-4)

**Measure performance impact:**

1. **Create benchmark grammars:**
```
bench/grammars/right_recursive.yaep
bench/grammars/expression_right.yaep
bench/grammars/ansi_c.yaep  (has right-recursive elements)
```

2. **Benchmark inputs:**
```
bench/inputs/right_recursive_n100.txt
bench/inputs/right_recursive_n1000.txt
bench/inputs/right_recursive_n10000.txt
```

3. **Run benchmarks:**
```bash
# Before Leo (baseline from P3)
bench/yaep_bench --baseline --grammar=right_recursive --input=n1000

# After Leo
bench/yaep_bench --leo --grammar=right_recursive --input=n1000
```

4. **Capture results:**
```json
{
  "grammar": "right_recursive",
  "input_size": 1000,
  "baseline_ms": 450,
  "leo_ms": 12,
  "speedup": "37.5x",
  "items_baseline": 500500,
  "items_leo": 1000,
  "reduction": "99.8%"
}
```

**Expected Results:**
- Right-recursive: **10-100x speedup**
- LR(k) grammars: **50-500x speedup** (near-linear)
- General grammars: **10-30% speedup**

### P4-GATE: Gate Report (Week 3, Day 5)

**Create GATE_P4_REPORT.md:**

1. Implementation summary
2. Performance measurements
3. Test results (all passing)
4. Code metrics
5. Known limitations
6. Readiness for P5

---

## Integration Points

### Existing Code Modifications

**Files to modify:**
1. **src/yaep.c:**
   - Add Leo structures (after line ~2160, with situation declarations)
   - Modify `sit_init()` to call `leo_init()`
   - Modify `set_init()` to call `leo_clear()`
   - Modify completion logic in `build_new_set()` (~line 4900-5100)
   - Add Leo statistics to debug output (~line 8230)

2. **src/yaep.h:**
   - Add Leo enable/disable flag to `struct grammar`:
     ```c
     int leo_optimization;  /**< Enable Leo optimization (default TRUE) */
     ```

3. **CMakeLists.txt:**
   - No changes (Leo code integrated into yaep.c)

### API Changes

**Public API additions:**
```c
// Enable/disable Leo optimization (default: enabled)
void yaep_set_leo_optimization(struct grammar *g, int enable);
int yaep_get_leo_optimization(struct grammar *g);
```

**Backward Compatibility:**
- Leo enabled by default
- All existing code works unchanged
- Performance improvement transparent to users

---

## Testing Strategy

### Correctness Testing

1. **Existing test suite:** All 129 tests must pass with Leo enabled
2. **Parse tree verification:** Leo and standard must produce identical trees
3. **Grammar coverage:**
   - Left-recursive grammars (Leo not applicable)
   - Right-recursive grammars (Leo highly effective)
   - Mixed grammars
   - Ambiguous grammars
   - LR(k) grammars

### Performance Testing

1. **Micro-benchmarks:** Isolated right-recursive grammars
2. **Macro-benchmarks:** Real-world grammars (ANSI C)
3. **Scaling tests:** Input sizes 100, 1K, 10K, 100K tokens
4. **Comparison:** Leo vs baseline (PERF_BASELINE_P3.json)

### Regression Testing

1. **Zero warnings:** `build_warnings.log` must be empty
2. **Valgrind clean:** No memory leaks
3. **No slowdown:** No grammar should be slower with Leo

---

## Risk Mitigation

### Risk 1: Incorrect Leo Detection

**Risk:** False positives - creating Leo items when conditions not met

**Mitigation:**
- Conservative detection (strict condition checking)
- Extensive testing with edge cases
- Compare results with Leo disabled

**Fallback:** Disable Leo if issues detected

### Risk 2: Performance Regression

**Risk:** Leo overhead on grammars where it doesn't help

**Mitigation:**
- Measure overhead in detection phase
- Add fast-path check (skip Leo detection if grammar not right-recursive)
- Profile and optimize hot paths

**Fallback:** Allow per-grammar Leo disable

### Risk 3: Complexity

**Risk:** Implementation bugs due to algorithm subtlety

**Mitigation:**
- Follow Leo (1991) paper precisely
- Implement incrementally (P4-001 through P4-006)
- Test each phase independently
- Document every decision

**Verification:**
- All existing tests pass
- New Leo-specific tests pass
- Parse trees identical with/without Leo

---

## Success Criteria

### Functional

- [ ] All 129 existing tests pass with Leo enabled
- [ ] All new Leo tests pass (>10 tests)
- [ ] Parse trees identical with/without Leo
- [ ] Zero compiler warnings
- [ ] Valgrind clean

### Performance

- [ ] Right-recursive grammars: >10x speedup
- [ ] LR(k) grammars: >20x speedup
- [ ] General grammars: No slowdown (±5%)
- [ ] Item count reduced for right-recursive grammars

### Quality

- [ ] Code documented (400+ lines documentation)
- [ ] Gate report complete
- [ ] Benchmark results captured
- [ ] Ready for Phase P5

---

## Timeline

| Week | Days | Tasks | Deliverable |
|------|------|-------|-------------|
| 1 | 1-2 | P4-001: Data structures | Structures compile |
| 1 | 3-4 | P4-002: Leo table ops | Table tests pass |
| 1 | 5 | P4-003: Detection (start) | Detection compiles |
| 2 | 1-2 | P4-003: Detection (finish) | Detection tests pass |
| 2 | 3-5 | P4-004: Leo completion | Completion works |
| 3 | 1-2 | P4-005: Testing | All tests pass |
| 3 | 3-4 | P4-006: Benchmarking | Results captured |
| 3 | 5 | P4-GATE: Gate report | Phase complete |

**Total: 3 weeks (15 working days)**

---

## References

1. **Leo (1991):** "A general context-free parsing algorithm running in linear time on every LR(k) grammar without using lookahead"
   - Original paper describing the algorithm
   - Theoretical complexity analysis
   - Proof of correctness

2. **Aycock & Horspool (2002):** "Practical Earley Parsing"
   - Section 4.1: Leo optimization implementation details
   - Performance measurements
   - Practical considerations

3. **Scott & Johnstone (2010):** "GLL Parsing"
   - Alternative approach to same problem
   - Comparison with Leo's method

---

**Document Status:** ✅ Ready for implementation  
**Next Step:** Begin P4-001 (Data structures)  
**Prerequisites:** P3-GATE complete ✅
