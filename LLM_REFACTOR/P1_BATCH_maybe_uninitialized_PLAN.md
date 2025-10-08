# P1 Batch Plan: maybe-uninitialized

Goal: Eliminate all warnings flagged as potential uninitialized variable usage.
Risk Level: High (could hide UB / non-determinism)
Scope Sources: `src/yaep.c` (functions: prune_to_minimal, find_minimal_translation, yaep_read_grammar_internal, make_parse) and any others identified via filtered index.

## Inventory
Extract via:
```
awk -F, '/maybe-uninitialized/ {print}' LLM_REFACTOR/WARNINGS_INDEX.csv
```
(Current baseline examples include: result, min_cost, start, term_node_array potential uses.)

## Strategy
1. Audit each variable for control-flow guaranteeing initialization.
2. Prefer definitive initialization at declaration if cheap.
3. For variables whose initialization is logically conditional, restructure to ensure assignment before use (split blocks / early continue / guard).
4. Add asserts for invariants that should hold post-initialization.
5. Rebuild; confirm zero maybe-uninitialized warnings.

## Acceptance Criteria
- Zero maybe-uninitialized warnings.
- No semantic behavior changes (confirmed by full test suite pass).
- No introduction of new warnings in other categories.
- Document changes in `WARNINGS_BASELINE.md` delta section.

## Rollback Plan
If a refactor unexpectedly changes parse output or tests fail, revert the specific commit (isolated to this batch) and re-approach with finer granularity.

## Implementation Notes
- For `result/min_cost` ambiguity inside cost selection logic, ensure min_cost initialized to sentinel (e.g., INT_MAX) and result to NULL explicitly.
- For arrays like `term_node_array`, allocate and zero memory or explicitly set NULL pointers before loop logic.
- For `start` symbol detection, ensure assignment path executes or add explicit failure with error return if not set.

## Next Step After Batch
Proceed to `cast-qual` or `shadow` depending on complexity encountered here.

---
(Generated – ready for execution.)
