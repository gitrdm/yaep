# YAEP Earley Optimization Implementation Checklist

**Project:** YAEP (Yet Another Earley Parser) Optimization  
**Branch:** `yaep-expansion`  
**Document Version:** 1.0  
**Last Updated:** October 9, 2025  
**Build/Test Script:** `./build_and_test.sh` (must exit 0 before any commit)

---

## 📋 CRITICAL: Context Restoration Guide (For LLM Context Window Shifts)

When resuming work after a context window shift, **ALWAYS** review these sections first:

### Quick Restoration Checklist
1. ✅ Read **Section 0: Mandatory Coding Standards** (non-negotiable rules)
2. ✅ Read **Section 1: Code Documentation Standards** (how to comment)
3. ✅ Check **Section 2: Progress Tracking** (where we are now)
4. ✅ Review **Section 3: Build & Test Protocol** (verification steps)
5. ✅ Review **Section 4: Git Commit Protocol** (how to commit)
6. ✅ Scan **Current Phase Tasks** in relevant section (what to do next)

### Essential Files to Check
```bash
# Current status
cat LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md  # This file
grep "\[x\]" LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md | wc -l  # Completed count

# Latest build state
./build_and_test.sh
grep "warning:" build_warnings.log | wc -l  # Should be 0

# Latest commit
git log -1 --oneline

# Current branch
git branch --show-current  # Should be yaep-expansion
```

---

## 0. MANDATORY CODING STANDARDS (Hard Requirements — NO EXCEPTIONS)

**⚠️ VIOLATION OF ANY STANDARD = IMMEDIATE REJECTION OF CHANGE ⚠️**

| # | Standard | Enforcement | Check Command | Violation Action |
|---|----------|-------------|---------------|------------------|
| 1 | **No stubs/placeholders/TODOs** | Zero tolerance | `grep -Rn "TODO\|FIXME\|STUB\|XXX\|HACK" src/ test/` | Reject commit |
| 2 | **Production-ready code only** | All tests pass | `./build_and_test.sh` exit 0 | Reject commit |
| 3 | **Zero compiler warnings** | Strict enforcement | `grep "warning:" build_warnings.log` returns empty | Fix before commit |
| 4 | **Test every new code path** | Required test addition | Review diff for test file changes | Reject if no tests |
| 5 | **Literate, explanatory comments** | Mandatory documentation | Review all changed functions | Amend before commit |
| 6 | **Atomic commits** | One logical change per commit | Review git log | Squash if violated |
| 7 | **No performance regressions** | Baseline comparison | Compare with `PERF_BASELINE.json` | Revert or fix |
| 8 | **Valgrind clean** | Weekly gate check | `valgrind --leak-check=full` on tests | Fix leaks |
| 9 | **Consistent formatting** | No unrelated changes | Diff review (minimal changes only) | Amend commit |
| 10 | **Document design decisions** | Inline + commit message | Code review | Amend before commit |

### Pre-Commit Verification Script (MUST RUN)

```bash
#!/bin/bash
# Save as: scripts/pre_commit_check.sh

echo "=== Pre-Commit Verification ==="

# 1. Build and test
echo "[1/5] Running build and tests..."
./build_and_test.sh
if [ $? -ne 0 ]; then
    echo "❌ FAIL: Build or tests failed"
    exit 1
fi

# 2. Check for placeholders
echo "[2/5] Checking for TODOs/FIXMEs/STUBs..."
if grep -Rn "TODO\|FIXME\|STUB\|XXX\|HACK" src/ test/ 2>/dev/null; then
    echo "❌ FAIL: Placeholders found in code"
    exit 1
fi

# 3. Check for warnings
echo "[3/5] Checking for compiler warnings..."
if grep "warning:" build_warnings.log 2>/dev/null; then
    echo "❌ FAIL: Compiler warnings present"
    exit 1
fi

# 4. Check formatting
echo "[4/5] Checking git diff size..."
DIFF_LINES=$(git diff --cached --stat | tail -1 | awk '{print $4}')
if [ "$DIFF_LINES" -gt 1000 ]; then
    echo "⚠️  WARNING: Large diff ($DIFF_LINES lines). Verify changes are minimal."
fi

# 5. Verify tests added
echo "[5/5] Checking for test additions..."
if git diff --cached --name-only | grep -q "^test/"; then
    echo "✅ Test files modified"
else
    echo "⚠️  WARNING: No test files in commit. Verify if tests needed."
fi

echo "✅ Pre-commit checks passed"
exit 0
```

**Usage Before Every Commit:**
```bash
chmod +x scripts/pre_commit_check.sh
./scripts/pre_commit_check.sh && git commit -m "Your message"
```

---

## 1. CODE DOCUMENTATION STANDARDS

### 1.1 Function Documentation Template

**EVERY new or modified function MUST have this documentation:**

```c
/**
 * Brief one-line description of what function does
 *
 * Detailed explanation of purpose, algorithm, and usage.
 * Include complexity analysis if non-trivial (e.g., O(n²)).
 *
 * RATIONALE: Why this approach was chosen (especially for optimizations)
 *
 * EXAMPLE:
 *   typical_usage_example();
 *
 * @param param_name Description of parameter, valid ranges, NULL handling
 * @param another    Continue description on new line if needed
 *
 * @return Description of return value, including special values (NULL, -1, etc.)
 *         and error conditions
 *
 * @note Any important caveats, thread-safety, side effects
 * @warning Performance implications, memory ownership, or dangerous behavior
 *
 * @see related_function() - Reference to related functions
 *
 * OPTIMIZATION: [If this is an optimization] What it optimizes and expected speedup
 *
 * TESTING: Brief note on how this is tested (e.g., "See test_foo() in test/test_bar.c")
 */
static int example_function(grammar_t *g, const char *param) {
    // Implementation
}
```

### 1.2 Code Block Documentation

**Complex logic blocks MUST have explanatory comments:**

```c
/* Step 1: Compute nullable set using fixed-point iteration
 * 
 * Algorithm: Iterate over all rules until no changes occur.
 * For each rule, if all RHS symbols are nullable and LHS is not marked,
 * mark LHS as nullable and record change.
 *
 * Complexity: O(R * S) where R = rules, S = symbols per rule
 * Worst case: O(N) iterations where N = nonterminals
 */
do {
    changed = 0;
    for (size_t i = 0; i < num_rules; i++) {
        // ... implementation ...
    }
} while (changed);
```

### 1.3 Data Structure Documentation

**All new structs MUST document fields:**

```c
/**
 * Leo item for right-recursion optimization
 *
 * Represents a topmost item in a right-recursive derivation chain.
 * Used to eliminate redundant completions in Leo's algorithm.
 *
 * MEMORY: Allocated in parse-local arena, freed with parse cleanup
 * REFS: Leo (1991) "A general context-free parsing algorithm..."
 */
typedef struct leo_item {
    struct core *core;           /**< The Earley item core (LR(0) item) */
    struct set *origin_set;      /**< Set where this item originated */
    struct leo_item *next;       /**< Next in hash chain (NULL if last) */
    int transition_symbol;       /**< Symbol that transitions to this item */
} leo_item_t;
```

### 1.4 Optimization Documentation

**EVERY optimization MUST document:**

1. **What** it optimizes (specific algorithm/data structure)
2. **Why** it's needed (performance problem being solved)
3. **How** it works (algorithm with complexity analysis)
4. **Impact** (expected speedup, memory savings, test results)
5. **References** (papers, blog posts, or prior art)

**Example:**
```c
/*******************************************************************************
 * OPTIMIZATION: Leo's Right-Recursion Optimization
 *
 * WHAT: Eliminates redundant completions in right-recursive grammars
 *
 * WHY: Standard Earley is O(n³) for arbitrary grammars. Right-recursive rules
 *      (common in programming languages) cause performance degradation due to
 *      repeated completion operations.
 *
 * HOW: Track "topmost" items in right-recursive chains using Leo items.
 *      When completing, check if a Leo item exists - if so, skip the chain
 *      and jump directly to the topmost item.
 *
 * IMPACT: O(n³) → O(n²) for right-recursive grammars
 *         Measured: 2.3x speedup on C grammar, 3.1x on highly right-recursive
 *
 * REFERENCE: Leo (1991) "A general context-free parsing algorithm running in
 *            linear time on every LR(k) grammar without using lookahead"
 *
 * TESTING: test/test_leo_optimization.c - synthetic right-recursive grammars
 ******************************************************************************/
```

---

## 2. PROGRESS TRACKING

### 2.1 Current Status (Update After Each Phase)

**Last Updated:** October 9, 2025

| Phase | Status | Completion % | Started | Completed | Gate Report |
|-------|--------|--------------|---------|-----------|-------------|
| P0: Baseline & Hygiene | ✅ Complete | 100% | Oct 8 | Oct 8 | `GATE_P0_REPORT.md` |
| P1: Warning Elimination | ✅ Complete | 100% | Oct 8 | Oct 9 | `WARNINGS_ELIMINATION_REPORT.md` |
| P2: Modularization | ⬜ Not Started | 0% | - | - | - |
| P3: Optimization Infrastructure | ⬜ Not Started | 0% | - | - | - |
| P4: Leo Optimization | ⬜ Not Started | 0% | - | - | - |
| P5: SPPF | ⬜ Not Started | 0% | - | - | - |
| P6: Final Optimizations | ⬜ Not Started | 0% | - | - | - |
| P7: Documentation & Hardening | ⬜ Not Started | 0% | - | - | - |

### 2.2 Quick Metrics

```bash
# Run these commands to check current status:

# Warning count (should be 0)
grep "warning:" build_warnings.log | wc -l

# Test pass rate (should be 100%)
./build_and_test.sh 2>&1 | grep "tests passed"

# Completed tasks
grep "\[x\]" LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md | wc -l

# Pending tasks  
grep "\[ \]" LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md | wc -l

# Current TODO count (should be 0)
grep -Rn "TODO\|FIXME" src/ test/ | wc -l
```

---

## 3. BUILD & TEST PROTOCOL

### 3.1 Mandatory Build/Test Frequency

**⚠️ MUST run `./build_and_test.sh` after EVERY logical change ⚠️**

A "logical change" is:
- Modifying any function implementation
- Adding/removing/renaming any function or variable
- Changing any data structure
- Completing any checklist item

### 3.2 Build/Test Verification Steps

```bash
# Step 1: Clean build (if major changes)
rm -rf build/
mkdir build
cd build
cmake ..
cd ..

# Step 2: Build and test
./build_and_test.sh

# Step 3: Verify zero warnings
if grep "warning:" build_warnings.log; then
    echo "❌ WARNINGS PRESENT - FIX BEFORE PROCEEDING"
    exit 1
fi

# Step 4: Verify test results
if ! grep "100% tests passed" build_warnings.log; then
    echo "❌ TESTS FAILED - FIX BEFORE PROCEEDING"
    exit 1
fi

# Step 5: Record results
echo "✅ Build/test successful at $(date)" >> build_test_log.txt
```

### 3.3 Regression Detection

**After each optimization, compare performance:**

```bash
# Before optimization
./bench/run_benchmark.sh > before.json

# After optimization  
./bench/run_benchmark.sh > after.json

# Compare (should show improvement or no regression)
python3 scripts/compare_perf.py before.json after.json
```

---

## 4. GIT COMMIT PROTOCOL

### 4.1 Commit Message Format (MANDATORY)

```
[PHASE] Brief description (50 chars max)

Detailed explanation of WHAT changed and WHY.
Include algorithmic changes, optimizations, and rationale.

TESTING: How this was tested
IMPACT: Performance/memory impact (if applicable)
REFS: Issue/ticket numbers or documentation references

Files modified:
- src/file1.c: Added function foo() for optimization X
- test/test_file.c: Added test_foo() to verify optimization
```

**Example:**
```
[P4-LEO] Add Leo item tracking for right-recursion

Implemented Leo's optimization for right-recursive grammars.
Added leo_item_t structure and tracking in Earley sets.
Modified completion step to use Leo items when available.

Algorithm: Tracks topmost items in right-recursive chains,
eliminating redundant completions by jumping directly to
the top of the chain.

TESTING: test/test_leo.c - synthetic right-recursive grammars
IMPACT: 2.3x speedup on C grammar (measured), O(n³)→O(n²) complexity
REFS: Leo (1991) "A general context-free parsing algorithm..."

Files modified:
- src/yaep.c: Added leo_item_t, modified completion_step()
- src/yaep.h: Added leo_item_t structure definition
- test/test_leo.c: Added Leo optimization test suite
```

### 4.2 Commit Frequency (MANDATORY)

**Commit after EVERY completed checklist item (even if small)**

**Why:** 
- Enables easy rollback if issues found
- Creates clear audit trail
- Prevents large, hard-to-review commits
- Allows bisecting to find regressions

**Minimum commit size:** One logical change (e.g., one function modified)  
**Maximum commit size:** One checklist item (even if multiple files)

### 4.3 Git Workflow

```bash
# 1. Make changes for ONE checklist item
vim src/yaep.c

# 2. Build and test
./build_and_test.sh
if [ $? -ne 0 ]; then
    echo "Build/test failed - fix before commit"
    exit 1
fi

# 3. Verify no warnings
if grep "warning:" build_warnings.log; then
    echo "Warnings present - fix before commit"
    exit 1
fi

# 4. Stage changes
git add src/yaep.c test/test_new.c

# 5. Verify diff is minimal and focused
git diff --cached

# 6. Commit with detailed message
git commit -m "[P4-LEO] Add leo_item_t structure

Added Leo item structure for right-recursion optimization.
...
"

# 7. Update checklist (mark item as [x])
vim LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md
git add LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md
git commit -m "[TRACKING] Mark P4-001 complete"

# 8. Verify clean working tree
git status  # Should show "nothing to commit"
```

---

## 5. EARLEY OPTIMIZATION PHASES (Detailed Step-by-Step)

### PHASE 0: Baseline & Hygiene ✅ COMPLETE

**Status:** ✅ **COMPLETE** (October 8, 2025)  
**Gate Report:** `LLM_REFACTOR/GATE_P0_REPORT.md`

All P0 tasks completed. Baseline established.

---

### PHASE 1: Warning Elimination ✅ COMPLETE

**Status:** ✅ **COMPLETE** (October 9, 2025)  
**Gate Report:** `LLM_REFACTOR/WARNINGS_ELIMINATION_REPORT.md`

**Achievement:** Zero compiler warnings (reduced from 22 to 0)

All P1 tasks completed. Ready for Phase 2.

---

### PHASE 2: Modularization (Behavior-Preserving Refactoring)

**Status:** ⬜ **NOT STARTED**  
**Goal:** Extract Earley engine and grammar analysis into separate modules  
**Duration:** 2-3 days  
**Risk:** Low (behavior-preserving refactoring)

**Prerequisites:**
- ✅ P1 complete (zero warnings)
- ✅ All tests passing
- ✅ Baseline performance captured

#### P2-001: Extract Earley Engine Module

**Objective:** Create `src/earley_engine.c` and `src/earley_engine.h`

**Tasks:**
```
[x] P2-001-A: Create earley_engine.h header
    - Define opaque engine handle: typedef struct earley_engine earley_engine_t;
    - Declare engine lifecycle functions: create, destroy, reset
    - Declare core parsing functions: predict, scan, complete
    - Add comprehensive documentation header
    - COMMIT: "[P2-ENG] Add earley_engine.h header" (a2a129f)

[x] P2-001-B: Create earley_engine.c skeleton
    - Implement empty structure
    - Implement stub lifecycle functions (return NULL/error for now)
    - Add file header with module purpose documentation
    - BUILD & TEST (should still pass with stubs)
    - COMMIT: "[P2-ENG] Add earley_engine.c skeleton" (d9fda4e)

[ ] P2-001-C: Extract prediction logic
    - Move predictor() function from yaep.c to earley_engine.c
    - Keep same signature initially (later will refactor)
    - Update yaep.c to call earley_engine version
    - Add detailed function documentation (see Section 1.1)
    - BUILD & TEST
    - COMMIT: "[P2-ENG] Extract predictor() to earley_engine.c"

[ ] P2-001-D: Extract scan logic
    - Move scanner() function from yaep.c to earley_engine.c
    - Update yaep.c to call earley_engine version
    - Document function purpose and algorithm
    - BUILD & TEST
    - COMMIT: "[P2-ENG] Extract scanner() to earley_engine.c"

[ ] P2-001-E: Extract completion logic
    - Move completer() function from yaep.c to earley_engine.c
    - Update yaep.c to call earley_engine version
    - Document completion algorithm (critical for Leo opt later)
    - BUILD & TEST
    - COMMIT: "[P2-ENG] Extract completer() to earley_engine.c"

[ ] P2-001-F: Verify no behavior change
    - Run full test suite
    - Compare with baseline performance (should be identical ±2%)
    - Review git diff (ensure only function moves, no logic changes)
    - CREATE: performance_p2_001.json
    - COMMIT: "[P2-ENG] Verify earley_engine extraction complete"
```

**Acceptance Criteria:**
- ✅ All tests pass (128/128)
- ✅ Zero warnings
- ✅ Performance unchanged (within ±2% of baseline)
- ✅ Git diff shows only function moves (no algorithm changes)
- ✅ All extracted functions have full documentation

#### P2-002: Extract Grammar Analysis Module

**Objective:** Create `src/grammar_analysis.c` and `src/grammar_analysis.h`

**Tasks:**
```
[ ] P2-002-A: Create grammar_analysis.h header
    - Define analysis result structures
    - Declare analysis functions (nullable, first, follow placeholders)
    - Add module documentation
    - COMMIT: "[P2-GRAM] Add grammar_analysis.h header"

[ ] P2-002-B: Create grammar_analysis.c skeleton
    - Implement stub analysis functions
    - Add file header documentation
    - BUILD & TEST
    - COMMIT: "[P2-GRAM] Add grammar_analysis.c skeleton"

[ ] P2-002-C: Extract grammar validation logic
    - Move validation functions from yaep.c to grammar_analysis.c
    - Keep validation behavior identical
    - Document validation rules
    - BUILD & TEST
    - COMMIT: "[P2-GRAM] Extract grammar validation to grammar_analysis.c"

[ ] P2-002-D: Prepare for nullable analysis (infrastructure only)
    - Add nullable_set field to grammar structure (unused for now)
    - Add compute_nullable_set() stub (returns immediately)
    - Document what nullable analysis will do (in comments)
    - BUILD & TEST
    - COMMIT: "[P2-GRAM] Add nullable analysis infrastructure (stub)"
```

**Acceptance Criteria:**
- ✅ All tests pass
- ✅ Zero warnings
- ✅ No behavior change
- ✅ Infrastructure ready for P3 optimizations

#### P2-003: Update Build System

**Tasks:**
```
[ ] P2-003-A: Update CMakeLists.txt
    - Add earley_engine.c to build
    - Add grammar_analysis.c to build
    - Update include paths if needed
    - BUILD & TEST
    - COMMIT: "[P2-BUILD] Add new modules to CMake"

[ ] P2-003-B: Update documentation
    - Update doc/Internals.txt with new module structure
    - Add ASCII diagram showing module relationships
    - Document module responsibilities
    - COMMIT: "[P2-DOC] Update Internals.txt for modularization"
```

#### P2-GATE: Phase 2 Completion Gate

**Requirements:**
```
[ ] All P2 tasks marked [x]
[ ] ./build_and_test.sh exits 0
[ ] grep "warning:" build_warnings.log returns empty
[ ] Performance comparison: |baseline - current| < 2%
[ ] Git history shows atomic commits (one change per commit)
[ ] All new functions have documentation per Section 1.1
[ ] Create GATE_P2_REPORT.md documenting completion
```

**Gate Report Template:**
```markdown
# Phase P2 Completion Report

## Summary
- Start Date: [DATE]
- End Date: [DATE]
- Duration: [DAYS]
- Commits: [COUNT]

## Modules Created
- src/earley_engine.{c,h} - [LINE_COUNT] lines
- src/grammar_analysis.{c,h} - [LINE_COUNT] lines

## Verification
- Tests: 128/128 passing ✅
- Warnings: 0 ✅
- Performance: baseline ±[X]% ✅
- Documentation: Complete ✅

## Ready for Phase 3
All prerequisites met for optimization infrastructure phase.
```

---

### PHASE 3: Optimization Infrastructure

**Status:** ⬜ **NOT STARTED**  
**Goal:** Build infrastructure for optimizations (no behavior change yet)  
**Duration:** 3-4 days  
**Risk:** Low (infrastructure only, not activated)

**Prerequisites:**
- ✅ P2 complete (modularization done)
- ✅ Modules extracted and tested
- ✅ Performance baseline established

#### P3-001: Nullable Preprocessing Infrastructure

**Objective:** Add infrastructure for nullable analysis (compute but don't use yet)

**Tasks:**
```
[ ] P3-001-A: Add nullable bitset to grammar structure
    - Modify struct grammar to include unsigned char *nullable_set
    - Allocate/deallocate in grammar lifecycle functions
    - Document bitset layout: nullable_set[symbol_id/8] & (1<<(symbol_id%8))
    - BUILD & TEST
    - COMMIT: "[P3-NULL] Add nullable bitset to grammar structure"

[ ] P3-001-B: Implement nullable computation algorithm
    - Implement compute_nullable_set() in grammar_analysis.c
    - Use fixed-point iteration algorithm
    - Add detailed comments explaining algorithm
    - Document complexity: O(R × S) where R=rules, S=symbols
    - DO NOT use results yet (compute but ignore for now)
    - BUILD & TEST
    - COMMIT: "[P3-NULL] Implement nullable computation (unused)"

[ ] P3-001-C: Add nullable computation test
    - Create test/test_nullable.c
    - Test with grammars with known nullable nonterminals
    - Test edge cases (all nullable, none nullable, cycles)
    - Verify bitset correctly populated
    - BUILD & TEST
    - COMMIT: "[P3-NULL] Add nullable computation tests"

[ ] P3-001-D: Call nullable computation in grammar parsing
    - Add compute_nullable_set() call in yaep_parse_grammar()
    - Verify set is computed (check bitset populated)
    - Verify parsing still works identically
    - BUILD & TEST
    - COMMIT: "[P3-NULL] Compute nullable set during grammar parsing"
```

**Acceptance Criteria:**
- ✅ Nullable set correctly computed (verified by tests)
- ✅ Not used in parsing yet (behavior unchanged)
- ✅ All tests pass
- ✅ Zero warnings

#### P3-002: Fast State Deduplication Infrastructure

**Objective:** Add hash-based deduplication (compute but don't use)

**Tasks:**
```
[ ] P3-002-A: Design item fingerprint hash function
    - Add compute_item_fingerprint() function
    - Hash: (rule_id << 16) | dot_position | origin_set_id
    - Document collision handling strategy
    - Add unit test for hash function
    - BUILD & TEST
    - COMMIT: "[P3-DEDUP] Add item fingerprint hash function"

[ ] P3-002-B: Add item hash table to Earley sets
    - Add hash table field to struct set
    - Allocate/deallocate in set lifecycle
    - Document hash table layout and purpose
    - DO NOT use for deduplication yet
    - BUILD & TEST
    - COMMIT: "[P3-DEDUP] Add item hash table infrastructure"

[ ] P3-002-C: Populate hash table (but don't use)
    - Insert items into hash table when added to set
    - Verify hash table correctly populated
    - Still use linear search for actual deduplication
    - BUILD & TEST
    - COMMIT: "[P3-DEDUP] Populate item hash table (unused)"

[ ] P3-002-D: Add hash table tests
    - Test hash collisions handled correctly
    - Test lookup performance vs linear search (should be faster)
    - Verify no false positives/negatives
    - BUILD & TEST
    - COMMIT: "[P3-DEDUP] Add deduplication hash table tests"
```

**Acceptance Criteria:**
- ✅ Hash table infrastructure in place
- ✅ Hash function tested and working
- ✅ Not used for deduplication yet (behavior unchanged)
- ✅ Tests demonstrate hash lookup is faster

#### P3-003: Memory Pool Infrastructure

**Objective:** Add memory pool for parse-local allocations

**Tasks:**
```
[ ] P3-003-A: Design memory pool structure
    - Create src/memory_pool.{c,h}
    - Define pool_t structure (arena allocator)
    - Document pool allocation strategy
    - COMMIT: "[P3-POOL] Add memory pool structure definition"

[ ] P3-003-B: Implement pool lifecycle functions
    - pool_create(), pool_destroy(), pool_reset()
    - Implement arena allocation (bump allocator)
    - Add safety checks (NULL checks, overflow detection)
    - Document memory alignment guarantees
    - BUILD & TEST
    - COMMIT: "[P3-POOL] Implement memory pool lifecycle"

[ ] P3-003-C: Add pool allocation functions
    - pool_alloc(pool, size) - allocate from pool
    - pool_alloc_array(pool, count, size) - typed allocation
    - Add alignment padding logic
    - Document allocation guarantees
    - BUILD & TEST
    - COMMIT: "[P3-POOL] Implement pool allocation functions"

[ ] P3-003-D: Add memory pool tests
    - Test allocation correctness
    - Test reset/reuse behavior
    - Test alignment guarantees
    - Test large allocations
    - BUILD & TEST
    - COMMIT: "[P3-POOL] Add memory pool tests"

[ ] P3-003-E: Integrate pool into parser (but don't use)
    - Add pool_t field to parse context
    - Create pool at parse start, destroy at parse end
    - DO NOT use for allocations yet (infrastructure only)
    - BUILD & TEST
    - COMMIT: "[P3-POOL] Integrate memory pool into parser"
```

**Acceptance Criteria:**
- ✅ Memory pool implemented and tested
- ✅ Pool created/destroyed with parse lifecycle
- ✅ Not used for allocations yet (behavior unchanged)
- ✅ Tests verify pool correctness

#### P3-004: Benchmark Harness

**Objective:** Create performance measurement infrastructure

**Tasks:**
```
[ ] P3-004-A: Create benchmark directory structure
    - mkdir -p bench/grammars bench/inputs bench/results
    - Add README explaining benchmark structure
    - COMMIT: "[P3-BENCH] Create benchmark directory structure"

[ ] P3-004-B: Implement benchmark driver
    - Create bench/run_benchmark.c
    - Parse multiple grammars with various inputs
    - Measure time, memory, item count
    - Output JSON format results
    - BUILD & TEST
    - COMMIT: "[P3-BENCH] Implement benchmark driver"

[ ] P3-004-C: Add representative test grammars
    - bench/grammars/c_grammar.txt - C language grammar
    - bench/grammars/right_recursive.txt - Highly right-recursive
    - bench/grammars/ambiguous.txt - Ambiguous grammar
    - Document each grammar's characteristics
    - COMMIT: "[P3-BENCH] Add representative test grammars"

[ ] P3-004-D: Create benchmark comparison script
    - scripts/compare_perf.py
    - Compare two JSON benchmark results
    - Show speedup/slowdown percentages
    - Highlight regressions (>5% slowdown)
    - BUILD & TEST
    - COMMIT: "[P3-BENCH] Add benchmark comparison script"

[ ] P3-004-E: Capture infrastructure baseline
    - Run benchmarks: ./bench/run_benchmark.sh > PERF_INFRA_BASELINE.json
    - Should be identical to PERF_BASELINE.json (±2%)
    - Document that infrastructure added with no performance impact
    - COMMIT: "[P3-BENCH] Capture infrastructure baseline performance"
```

**Acceptance Criteria:**
- ✅ Benchmark harness working
- ✅ Baseline performance captured
- ✅ Performance unchanged from Phase 0 baseline
- ✅ Comparison tool working

#### P3-GATE: Phase 3 Completion Gate

**Requirements:**
```
[ ] All P3 tasks marked [x]
[ ] ./build_and_test.sh exits 0
[ ] grep "warning:" build_warnings.log returns empty
[ ] Performance: |P0_baseline - P3_baseline| < 2%
[ ] Infrastructure tested but inactive (behavior unchanged)
[ ] All infrastructure documented
[ ] Create GATE_P3_REPORT.md
```

---

### PHASE 4: Leo's Right-Recursion Optimization

**Status:** ⬜ **NOT STARTED**  
**Goal:** Implement Leo's optimization for O(n²) right-recursive parsing  
**Duration:** 4-5 days  
**Risk:** Medium (complex algorithm, but well-documented)

**Prerequisites:**
- ✅ P3 complete (infrastructure ready)
- ✅ Earley engine modularized
- ✅ Benchmark harness working

**Reference:** Leo (1991) "A general context-free parsing algorithm running in linear time on every LR(k) grammar without using lookahead"

#### P4-001: Leo Data Structures

**Objective:** Add Leo item tracking structures

**Tasks:**
```
[ ] P4-001-A: Define leo_item_t structure
    - Create in src/earley_engine.h
    - Fields: core, origin_set, transition_symbol, next
    - Add comprehensive documentation (see Section 1.3)
    - Document memory ownership (allocated in parse arena)
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Add leo_item_t structure definition"

[ ] P4-001-B: Add Leo item list to Earley sets
    - Add leo_item_t *leo_items field to struct set
    - Initialize to NULL in set creation
    - Document what Leo items represent
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Add leo_items field to Earley sets"

[ ] P4-001-C: Add Leo item helper functions
    - create_leo_item() - allocate and initialize
    - find_leo_item() - lookup by transition symbol
    - add_leo_item() - insert into set
    - Document each function (see Section 1.1)
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Add Leo item helper functions"
```

**Acceptance Criteria:**
- ✅ Leo structures defined and documented
- ✅ Helper functions implemented
- ✅ Not used in parsing yet (behavior unchanged)

#### P4-002: Right-Recursive Detection

**Objective:** Detect right-recursive rule patterns

**Tasks:**
```
[ ] P4-002-A: Implement is_right_recursive() predicate
    - Check if rule has form: A → α B where B derives ε
    - Use nullable analysis from P3
    - Document detection algorithm
    - Add unit tests
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Add right-recursive rule detection"

[ ] P4-002-B: Mark right-recursive rules during grammar parse
    - Add is_right_recursive flag to rule structure
    - Compute flag in yaep_parse_grammar()
    - Document which rules are right-recursive
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Mark right-recursive rules in grammar"

[ ] P4-002-C: Add right-recursive detection tests
    - Test with known right-recursive grammars
    - Test with left-recursive grammars (should not match)
    - Test edge cases
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Add right-recursive detection tests"
```

**Acceptance Criteria:**
- ✅ Detection algorithm correct (verified by tests)
- ✅ Rules correctly marked
- ✅ Not used in parsing yet

#### P4-003: Leo Item Creation (Completion Step)

**Objective:** Create Leo items during completion

**Tasks:**
```
[ ] P4-003-A: Modify completion step to create Leo items
    - In completer() function, after completing item:
      * Check if completed item is from right-recursive rule
      * Check if all items in set with same core have same origin
      * If conditions met, create Leo item
    - Document Leo item creation conditions
    - Add detailed comments explaining algorithm
    - DO NOT use Leo items for completion yet
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Create Leo items during completion"

[ ] P4-003-B: Verify Leo items created correctly
    - Add logging/debugging output (conditional on flag)
    - Parse test grammar, verify Leo items created
    - Count Leo items vs regular items
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Verify Leo item creation"
```

**Acceptance Criteria:**
- ✅ Leo items created correctly
- ✅ Verified by logging/tests
- ✅ Not used for completion yet (behavior unchanged)

#### P4-004: Leo-Based Completion (Activation)

**Objective:** Use Leo items to skip redundant completions

**Tasks:**
```
[ ] P4-004-A: Modify completer to check for Leo items first
    - Before normal completion, check if Leo item exists
    - If Leo item found: use it instead of completing chain
    - If not found: fall back to normal completion
    - Document decision logic with comments
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Use Leo items in completion step"

[ ] P4-004-B: Verify correctness with tests
    - Run all existing tests (should still pass)
    - Verify parse results identical to before
    - Check that Leo items are actually being used (logging)
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Verify Leo optimization correctness"

[ ] P4-004-C: Add Leo-specific tests
    - Create test/test_leo.c
    - Test highly right-recursive grammars
    - Test that Leo items reduce completion count
    - Verify parse results correct
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Add Leo optimization test suite"
```

**Acceptance Criteria:**
- ✅ Leo optimization active
- ✅ All tests pass (including new Leo tests)
- ✅ Parse results correct
- ✅ Zero warnings

#### P4-005: Performance Measurement

**Objective:** Measure Leo optimization impact

**Tasks:**
```
[ ] P4-005-A: Run benchmarks with Leo enabled
    - ./bench/run_benchmark.sh > PERF_LEO.json
    - Compare with PERF_INFRA_BASELINE.json
    - Document speedup on right-recursive grammars
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Capture Leo optimization performance"

[ ] P4-005-B: Add toggle for Leo optimization
    - Add YAEP_ENABLE_LEO environment variable
    - If set to 0, disable Leo optimization
    - If set to 1 (default), enable Leo optimization
    - Document toggle in README
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Add toggle for Leo optimization"

[ ] P4-005-C: Verify no regression on non-right-recursive
    - Test that left-recursive grammars not slowed down
    - Test that simple grammars not affected
    - Verify overhead is minimal (<5%)
    - BUILD & TEST
    - COMMIT: "[P4-LEO] Verify no regression on other grammar types"
```

**Acceptance Criteria:**
- ✅ Right-recursive grammars: >=2x speedup
- ✅ Other grammars: <5% overhead
- ✅ Toggle working
- ✅ Performance documented

#### P4-GATE: Phase 4 Completion Gate

**Requirements:**
```
[ ] All P4 tasks marked [x]
[ ] ./build_and_test.sh exits 0
[ ] grep "warning:" build_warnings.log returns empty
[ ] Right-recursive performance: >=2x improvement
[ ] Other grammars: <5% overhead
[ ] All tests pass (128 + new Leo tests)
[ ] Leo optimization fully documented
[ ] Create GATE_P4_REPORT.md
```

---

### PHASE 5: Packed Parse Forests (SPPF)

**Status:** ⬜ **NOT STARTED**  
**Goal:** Implement SPPF for efficient ambiguous parse representation  
**Duration:** 5-6 days  
**Risk:** High (complex data structure, API changes)

**Prerequisites:**
- ✅ P4 complete (Leo optimization working)
- ✅ Performance infrastructure in place

**Reference:** Scott & Johnstone (2013) "GLL Parsing"

#### P5-001: SPPF Data Structures

**Tasks:**
```
[ ] P5-001-A: Define SPPF node types
    - Create src/sppf.{c,h}
    - Define sppf_node_type_t enum (SYMBOL, PACKED, INTERMEDIATE)
    - Define sppf_node_t structure
    - Document SPPF format and node types
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Add SPPF node structures"

[ ] P5-001-B: Add SPPF node lifecycle functions
    - create_sppf_node()
    - add_packed_alternative()
    - destroy_sppf_forest()
    - Document memory management
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Add SPPF lifecycle functions"

[ ] P5-001-C: Add SPPF node pool
    - Use memory pool from P3 for SPPF nodes
    - Document allocation strategy
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Use memory pool for SPPF nodes"
```

**Acceptance Criteria:**
- ✅ SPPF structures defined
- ✅ Lifecycle functions working
- ✅ Not integrated into parsing yet

#### P5-002: SPPF Construction During Parse

**Tasks:**
```
[ ] P5-002-A: Add SPPF building hooks to parser
    - Modify tree building code to create SPPF nodes
    - Detect when multiple alternatives exist
    - Create packed nodes for alternatives
    - Document construction algorithm
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Add SPPF construction hooks"

[ ] P5-002-B: Handle node sharing
    - Implement deduplication for SPPF nodes
    - Use hash table to find existing nodes
    - Document sharing strategy
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Implement SPPF node sharing"

[ ] P5-002-C: Verify SPPF correctness
    - Test with ambiguous grammars
    - Verify all parse trees represented
    - Check that no duplicates exist
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Verify SPPF construction correctness"
```

**Acceptance Criteria:**
- ✅ SPPF correctly built for ambiguous parses
- ✅ All alternatives represented
- ✅ Nodes properly shared

#### P5-003: SPPF Traversal API

**Tasks:**
```
[ ] P5-003-A: Design traversal iterator
    - Add sppf_iterator_t structure
    - Implement iterator_next() function
    - Document traversal order
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Add SPPF traversal iterator"

[ ] P5-003-B: Add example traversal code
    - Create examples/sppf_traverse.c
    - Show how to extract all parse trees
    - Document common traversal patterns
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Add SPPF traversal examples"

[ ] P5-003-C: Update public API
    - Add yaep_get_sppf() function
    - Document SPPF API in yaep.h
    - Add API usage examples
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Update public API for SPPF"
```

**Acceptance Criteria:**
- ✅ Traversal API working
- ✅ Examples demonstrate usage
- ✅ API documented

#### P5-004: SPPF Performance Measurement

**Tasks:**
```
[ ] P5-004-A: Measure memory usage
    - Compare SPPF vs. multiple tree representation
    - Test on highly ambiguous grammars
    - Document memory savings
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Measure SPPF memory usage"

[ ] P5-004-B: Add SPPF toggle
    - Environment variable: YAEP_ENABLE_SPPF
    - Default: enabled
    - Document performance tradeoffs
    - BUILD & TEST
    - COMMIT: "[P5-SPPF] Add toggle for SPPF"

[ ] P5-004-C: Benchmark SPPF performance
    - Run benchmarks with SPPF enabled/disabled
    - Compare memory usage
    - Compare parse time
    - Document results
    - COMMIT: "[P5-SPPF] Benchmark SPPF performance"
```

**Acceptance Criteria:**
- ✅ Memory usage reduced on ambiguous grammars
- ✅ Performance acceptable (<10% overhead on unambiguous)
- ✅ Toggle working

#### P5-GATE: Phase 5 Completion Gate

**Requirements:**
```
[ ] All P5 tasks marked [x]
[ ] ./build_and_test.sh exits 0
[ ] grep "warning:" build_warnings.log returns empty
[ ] Ambiguous grammar memory: significant reduction demonstrated
[ ] Unambiguous grammar overhead: <10%
[ ] SPPF API documented and tested
[ ] Create GATE_P5_REPORT.md
```

---

### PHASE 6: Final Optimizations (Pruning + Aycock–Horspool)

**Status:** ⬜ **NOT STARTED**  
**Goal:** Add final optimizations for completeness  
**Duration:** 3-4 days  
**Risk:** Low (well-understood techniques)

**Prerequisites:**
- ✅ P5 complete (SPPF working)

#### P6-001: Grammar Reachability Analysis

**Tasks:**
```
[ ] P6-001-A: Compute reachable nonterminals
    - Add compute_reachable_set() to grammar_analysis.c
    - Use fixed-point iteration from start symbol
    - Add reachable_set bitset to grammar
    - Document algorithm
    - BUILD & TEST
    - COMMIT: "[P6-PRUNE] Compute reachable nonterminals"

[ ] P6-001-B: Compute productive nonterminals
    - Add compute_productive_set() to grammar_analysis.c
    - Nonterminal is productive if it can derive terminals
    - Add productive_set bitset to grammar
    - Document algorithm
    - BUILD & TEST
    - COMMIT: "[P6-PRUNE] Compute productive nonterminals"

[ ] P6-001-C: Use reachability for prediction pruning
    - In predictor(), skip unreachable nonterminals
    - Document pruning logic
    - Verify parse correctness maintained
    - BUILD & TEST
    - COMMIT: "[P6-PRUNE] Prune unreachable nonterminals in prediction"
```

**Acceptance Criteria:**
- ✅ Reachability analysis correct
- ✅ Pruning reduces prediction overhead
- ✅ Parse correctness maintained

#### P6-002: Early Stopping Detection

**Tasks:**
```
[ ] P6-002-A: Detect impossible continuations
    - If Earley set is empty and input remains: fail immediately
    - Document early stopping conditions
    - Add tests for early failure cases
    - BUILD & TEST
    - COMMIT: "[P6-PRUNE] Add early stopping for impossible parses"

[ ] P6-002-B: Measure early stopping impact
    - Test with invalid inputs
    - Verify early exit vs full parse
    - Document speedup on error cases
    - COMMIT: "[P6-PRUNE] Measure early stopping performance"
```

**Acceptance Criteria:**
- ✅ Early stopping working
- ✅ Speedup on error cases demonstrated

#### P6-003: Aycock–Horspool Scanner Optimization

**Tasks:**
```
[ ] P6-003-A: Group scanner items by terminal
    - In scanner(), group items awaiting same terminal
    - Process all items for same terminal together
    - Document grouping strategy
    - BUILD & TEST
    - COMMIT: "[P6-SCAN] Group scanner items by terminal"

[ ] P6-003-B: Batch scanner advancement
    - Advance all items for terminal in one pass
    - Document batch processing algorithm
    - Verify correctness
    - BUILD & TEST
    - COMMIT: "[P6-SCAN] Batch advance scanner items"

[ ] P6-003-C: Measure scanner optimization
    - Benchmark scanner performance
    - Compare with ungrouped version
    - Document speedup
    - COMMIT: "[P6-SCAN] Measure scanner optimization performance"
```

**Acceptance Criteria:**
- ✅ Grouping implemented correctly
- ✅ Speedup on token-heavy grammars

#### P6-004: Combined Optimization Benchmarks

**Tasks:**
```
[ ] P6-004-A: Run full benchmark suite
    - Test all optimizations enabled
    - Compare with baseline (P0)
    - Document aggregate speedup
    - COMMIT: "[P6-FINAL] Run combined optimization benchmarks"

[ ] P6-004-B: Create performance report
    - Document speedup by grammar type
    - Document memory usage improvements
    - Create charts/graphs if helpful
    - COMMIT: "[P6-FINAL] Create final performance report"
```

**Acceptance Criteria:**
- ✅ Combined optimizations show aggregate benefit
- ✅ No negative interactions between optimizations
- ✅ Performance report complete

#### P6-GATE: Phase 6 Completion Gate

**Requirements:**
```
[ ] All P6 tasks marked [x]
[ ] ./build_and_test.sh exits 0
[ ] grep "warning:" build_warnings.log returns empty
[ ] Aggregate performance improvement demonstrated
[ ] All optimizations documented
[ ] Create GATE_P6_REPORT.md
```

---

### PHASE 7: Documentation & Hardening

**Status:** ⬜ **NOT STARTED**  
**Goal:** Final QA, documentation, and release preparation  
**Duration:** 2-3 days  
**Risk:** Very Low

**Prerequisites:**
- ✅ P6 complete (all optimizations done)

#### P7-001: Final Testing

**Tasks:**
```
[ ] P7-001-A: Full valgrind pass
    - Run valgrind on all tests
    - Run valgrind on benchmarks
    - Verify 0 leaks, 0 errors
    - Document in valgrind_final.txt
    - COMMIT: "[P7-QA] Run final valgrind pass"

[ ] P7-001-B: Coverage analysis
    - Run coverage tool if available
    - Identify untested code paths
    - Add tests for uncovered paths
    - Document coverage percentage
    - COMMIT: "[P7-QA] Improve test coverage"

[ ] P7-001-C: Stress testing
    - Parse very large inputs
    - Parse deeply nested structures
    - Verify no crashes, no leaks
    - Document stress test results
    - COMMIT: "[P7-QA] Add stress tests"
```

**Acceptance Criteria:**
- ✅ Valgrind clean
- ✅ Coverage acceptable (>80% ideally)
- ✅ Stress tests pass

#### P7-002: Documentation Completion

**Tasks:**
```
[ ] P7-002-A: Update README.md
    - Document optimization features
    - Add performance characteristics
    - Add usage examples
    - Document build options
    - COMMIT: "[P7-DOC] Update README with optimizations"

[ ] P7-002-B: Update doc/Internals.txt
    - Document final architecture
    - Add optimization algorithm descriptions
    - Update performance characteristics
    - Add references to papers
    - COMMIT: "[P7-DOC] Update Internals.txt"

[ ] P7-002-C: Create optimization guide
    - Document when to use each optimization
    - Document performance tradeoffs
    - Add tuning recommendations
    - COMMIT: "[P7-DOC] Create optimization guide"
```

**Acceptance Criteria:**
- ✅ All documentation updated
- ✅ Examples working
- ✅ Guide helpful for users

#### P7-003: Release Preparation

**Tasks:**
```
[ ] P7-003-A: Update CHANGELOG
    - Document all changes since baseline
    - Organize by phase
    - Include performance improvements
    - COMMIT: "[P7-REL] Update CHANGELOG"

[ ] P7-003-B: Version bump
    - Update version numbers
    - Tag release candidate
    - COMMIT: "[P7-REL] Bump version for release"

[ ] P7-003-C: Final gate report
    - Create GATE_P7_REPORT.md
    - Summarize entire project
    - Document final metrics
    - COMMIT: "[P7-REL] Create final gate report"
```

**Acceptance Criteria:**
- ✅ CHANGELOG complete
- ✅ Version updated
- ✅ Final report created

#### P7-GATE: Phase 7 Completion Gate (PROJECT COMPLETE)

**Requirements:**
```
[ ] All P7 tasks marked [x]
[ ] ./build_and_test.sh exits 0
[ ] grep "warning:" build_warnings.log returns empty
[ ] Valgrind clean (0 leaks, 0 errors)
[ ] All documentation complete
[ ] Performance improvements documented
[ ] Tag: git tag -a v2.0-optimized
[ ] Create GATE_P7_REPORT.md (final project report)
```

---

## 6. OPTIMIZATION REFERENCE

### 6.1 Quick Optimization Summary

| Optimization | Benefit | Complexity | Risk | Phase |
|--------------|---------|------------|------|-------|
| Leo's | O(n³)→O(n²) right-recursive | Medium | Low | P4 |
| SPPF | Exponential→Polynomial ambiguous memory | High | Medium | P5 |
| Nullable | Eliminates redundant predictions | Low | Very Low | P3 |
| Deduplication | 10-20% memory/time reduction | Low | Very Low | P3 |
| Pruning | Avoids wasted computation | Low | Very Low | P6 |
| Aycock–Horspool | Faster scan step | Low | Very Low | P6 |

### 6.2 Performance Expectations

**Expected Aggregate Improvements (P7 vs P0):**
- Right-recursive grammars: **2-3x faster**
- Ambiguous grammars: **50-80% memory reduction**
- General grammars: **20-40% faster**
- Error cases: **5-10x faster** (early stopping)

---

## 7. TROUBLESHOOTING

### 7.1 Build Failures

**Problem:** `./build_and_test.sh` fails

**Solutions:**
1. Check for syntax errors: `gcc -fsyntax-only src/*.c`
2. Clean build: `rm -rf build/ && mkdir build && cd build && cmake .. && cd ..`
3. Check CMakeLists.txt for missing files
4. Verify all includes present

### 7.2 Test Failures

**Problem:** Tests fail after changes

**Solutions:**
1. Run individual test: `./build/test/testXX`
2. Check valgrind: `valgrind ./build/test/testXX`
3. Add debug logging to identify failure point
4. Compare with previous git commit: `git diff HEAD^`
5. Bisect if needed: `git bisect start`

### 7.3 Performance Regressions

**Problem:** Optimization slower than baseline

**Solutions:**
1. Profile with gprof: `gcc -pg ...`
2. Check algorithm complexity (O(n) vs O(n²) mistake?)
3. Verify optimization actually active (check toggles)
4. Compare item counts (are we creating more items?)
5. Review commits since last good performance

### 7.4 Memory Leaks

**Problem:** Valgrind reports leaks

**Solutions:**
1. Identify leak source: Look at valgrind backtrace
2. Check allocation/deallocation pairs
3. Verify cleanup functions called
4. Check error paths (are resources freed on error?)
5. Use memory pool to simplify cleanup

---

## 8. APPENDICES

### Appendix A: References

**Academic Papers:**
1. Leo (1991) "A general context-free parsing algorithm running in linear time on every LR(k) grammar without using lookahead"
2. Scott & Johnstone (2013) "GLL Parsing"
3. Aycock & Horspool (2002) "Practical Earley Parsing"

**Implementation Guides:**
- YAEP original documentation: `doc/Internals.txt`
- C17 Standard: ISO/IEC 9899:2018

### Appendix B: File Inventory

**Core Implementation:**
- `src/yaep.c` - Main parser (7460 lines)
- `src/earley_engine.c` - Earley engine (to be created in P2)
- `src/grammar_analysis.c` - Grammar analysis (to be created in P2)
- `src/sppf.c` - SPPF implementation (to be created in P5)
- `src/memory_pool.c` - Memory pools (to be created in P3)

**Tests:**
- `test/test*.c` - 128 existing tests
- `test/test_leo.c` - Leo tests (to be created in P4)
- `test/test_sppf.c` - SPPF tests (to be created in P5)

**Benchmarks:**
- `bench/run_benchmark.sh` - Benchmark driver (to be created in P3)
- `bench/grammars/` - Test grammars (to be created in P3)

**Documentation:**
- `LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md` - This file
- `LLM_REFACTOR/GATE_P*.md` - Phase completion reports
- `doc/Internals.txt` - Architecture documentation

---

## 9. QUICK START (For Resuming Work)

### When Resuming After Context Window Shift:

1. **Check current status:**
   ```bash
   cd /home/rdmerrio/gits/yaep
   git branch --show-current  # Should be yaep-expansion
   git log -3 --oneline       # See recent commits
   ```

2. **Find current phase:**
   ```bash
   grep "Status.*Complete\|Status.*In Progress" LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md
   ```

3. **Verify build state:**
   ```bash
   ./build_and_test.sh
   grep "warning:" build_warnings.log | wc -l  # Should be 0
   ```

4. **Find next task:**
   ```bash
   # Find first unchecked task in current phase
   grep -A 5 "^\[ \]" LLM_REFACTOR/EARLEY_OPTIMIZATION_CHECKLIST.md | head -20
   ```

5. **Review coding standards:**
   - Re-read Section 0 (Mandatory Coding Standards)
   - Re-read Section 1 (Code Documentation Standards)
   - Re-read Section 3 (Build & Test Protocol)
   - Re-read Section 4 (Git Commit Protocol)

6. **Proceed with next task:**
   - Implement task
   - Build & test
   - Commit
   - Update checklist
   - Repeat

---

**END OF CHECKLIST**

Last Updated: October 9, 2025  
Document Version: 1.0  
Status: Ready for use
