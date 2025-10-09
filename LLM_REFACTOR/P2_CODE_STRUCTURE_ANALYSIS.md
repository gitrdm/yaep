# YAEP Code Structure Analysis - Phase P2 Reality Check

**Date:** October 9, 2025  
**Branch:** earley-optimization  
**Phase:** P2 - Modularization  
**Document Purpose:** Reconcile planned extraction with actual code structure

---

## Executive Summary

The original P2 checklist assumed YAEP had separate `predictor()`, `scanner()`, and `completer()` functions that could be extracted independently. **This assumption is incorrect.**

**Reality:** The Earley algorithm in YAEP is implemented as a monolithic, highly optimized system where prediction, scanning, and completion are interleaved within several large functions with significant shared state.

**Impact:** The original P2 plan (tasks P2-001-C through P2-001-E) cannot be executed as written. We need a revised extraction strategy.

---

## Actual Code Structure

### Main Parsing Flow

```
yaep_parse()
  └─> yaep_parse_internal()
       ├─> tok_init()          // Initialize token processing
       ├─> read_toks()         // Read all input tokens
       ├─> yaep_parse_init()   // Initialize parsing state
       ├─> pl_create()         // Create parse list structure
       ├─> build_pl()          // *** MAIN EARLEY ALGORITHM ***
       │    └─> (for each token)
       │         ├─> core_symb_vect_find()  // Look up transition
       │         ├─> build_new_set()        // Build next set
       │         │    ├─> set_new_start()
       │         │    ├─> (scan via transition vector)
       │         │    ├─> (complete via reduce)
       │         │    ├─> expand_new_start_set()  // PREDICTION HERE
       │         │    │    ├─> add_derived_nonstart_sits()
       │         │    │    └─> (prediction loop)
       │         │    └─> set_insert()
       │         └─> pl[++pl_curr] = new_set
       ├─> make_parse()        // Build parse tree
       └─> yaep_parse_fin()    // Cleanup
```

### Key Functions and Their Roles

| Function | Lines | Role | Complexity |
|----------|-------|------|------------|
| `build_pl()` | 5512-5670 | Main parsing loop (scanning) | High |
| `build_new_set()` | 4760-4900 | Set construction (scan+complete) | Very High |
| `expand_new_start_set()` | 4604-4750 | Set expansion (PREDICTION) | Very High |
| `add_derived_nonstart_sits()` | 4494-4506 | Add derived situations | Low |

### Where Earley Steps Happen

**PREDICTION (Closure):**
- Location: `expand_new_start_set()` lines 4635-4641
- Logic: When nonterminal N is after dot, add all rules with LHS=N
- Code:
  ```c
  if (!symb->term_p)
    for (rule = symb->u.nonterm.rules; rule != NULL; rule = rule->lhs_next)
      set_new_add_initial_sit (sit_create (rule, 0, 0));
  ```

**SCANNING:**
- Location: `build_new_set()` lines 4780-4810
- Logic: Advance items via transition vector for current terminal
- Interleaved with completion

**COMPLETION:**
- Location: `build_new_set()` lines 4812-4892
- Logic: When item completes (empty tail), propagate to waiting items
- Interleaved with scanning

### Shared State (Global Variables)

The code uses extensive global state, making extraction difficult:

- `grammar` - Current grammar
- `pl[]` - Parse list (array of sets)
- `pl_curr` - Current parse list index
- `tok_curr` - Current token index
- `toks[]` - Token array
- `new_set` - Set being constructed
- `new_core` - Core being constructed
- `new_sits[]` - Situations in new set
- `new_dists[]` - Distances for new situations
- ... and dozens more

---

## Why Original P2 Plan Won't Work

### Original Plan Assumptions ❌
1. Separate predictor/scanner/completer functions exist
2. Functions can be extracted independently
3. Functions have clean interfaces
4. Minimal shared state

### Actual Reality ✅
1. Prediction/scanning/completion are interleaved in 3 large functions
2. Functions share massive amounts of global state
3. No clean separation of concerns
4. Hundreds of global variables used throughout

### Consequences
- Cannot extract `predictor()` - doesn't exist as separate function
- Cannot extract `scanner()` - interleaved with completion
- Cannot extract `completer()` - interleaved with scanning
- Original checklist tasks P2-001-C through P2-001-E are **not executable**

---

## Revised Extraction Strategy

### Phase P2 Goal Remains Valid
**Goal:** Modularize Earley engine for future optimizations

**But the approach must change:**

### Option 1: Defer Major Extraction (RECOMMENDED)
**Rationale:** The existing code works perfectly. Premature extraction risks introducing bugs.

**Approach:**
1. **P2-001-C (Revised):** Document code structure (this file)
2. **P2-001-D (Revised):** Create architecture diagram
3. **P2-001-E (Revised):** Add detailed inline comments to existing functions
4. **P2-001-F (Revised):** Verify build_pl() can be called via wrapper (no extraction yet)
5. **Phase P3+:** Add optimizations to existing structure FIRST
6. **Later (P8?):** Extract AFTER proving optimizations work

**Advantages:**
- Zero risk of breaking existing functionality
- Can add optimizations (Leo, SPPF) to existing code
- Modularization can happen AFTER optimizations proven
- Follows "make it work, make it right, make it fast" principle

**Disadvantages:**
- Defers the modularization goal
- Optimization code will be in yaep.c initially

### Option 2: Gradual State Migration
**Approach:**
1. Move global state into `struct earley_engine` incrementally
2. Pass engine pointer to existing functions
3. Refactor functions to use engine->field instead of globals
4. Eventually extract functions once state is encapsulated

**Advantages:**
- Progressive improvement
- Can verify at each step
- Eventually achieves modularization

**Disadvantages:**
- Many small commits
- Risk of subtle bugs in state migration
- Significant effort before any optimization begins

### Option 3: Wrapper-Only Approach
**Approach:**
1. Keep all implementation in yaep.c
2. earley_engine.c contains only thin wrappers
3. Wrappers call yaep.c functions
4. Modularization is API-level only, not implementation-level

**Advantages:**
- Minimal code change
- Low risk
- Establishes API for future work

**Disadvantages:**
- Doesn't achieve true modularization
- Limited value for current phase

---

## Recommendation: Option 1 (Documentation & Defer Extraction)

**Proposed Revised P2 Tasks:**

```
[x] P2-001-A: Create earley_engine.h header (DONE)
[x] P2-001-B: Create earley_engine.c skeleton (DONE)
[ ] P2-001-C: Document actual code structure (this file)
[ ] P2-001-D: Add comprehensive inline comments to build_pl()
[ ] P2-001-E: Add comprehensive inline comments to build_new_set()
[ ] P2-001-F: Add comprehensive inline comments to expand_new_start_set()
[ ] P2-002-A: Create grammar_analysis.h header
[ ] P2-002-B: Create grammar_analysis.c skeleton  
[ ] P2-002-C: Document grammar validation (no extraction yet)
[ ] P2-GATE: Phase 2 completion - documentation complete, ready for P3
```

**Then proceed to Phase P3-P7 with optimizations IN PLACE in yaep.c**

**Later (optional Phase P8): Extract After Success**
- Once optimizations are proven and working
- Extract modules for maintainability
- Less pressure, lower risk

---

## Conclusion

The original P2 checklist was based on an incorrect assumption about YAEP's code structure. Rather than force-fitting an extraction that doesn't match reality, we should:

1. **Accept reality:** Code is monolithic and highly optimized
2. **Document thoroughly:** Make the existing structure understandable
3. **Add optimizations first:** Prove they work in current structure
4. **Extract later:** When we have working optimizations to preserve

This approach is **safer, faster, and more pragmatic** than attempting risky refactoring before adding value through optimizations.

---

**Next Steps:**
1. Review this document with stakeholder
2. Update EARLEY_OPTIMIZATION_CHECKLIST.md with revised P2 tasks
3. Proceed with documentation tasks (P2-001-C through P2-001-F)
4. Move to Phase P3 (optimization infrastructure) sooner

**Status:** Ready for review and decision
