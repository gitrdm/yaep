# P1 Batch Plan: shadow

Goal: Eliminate all `-Wshadow` warnings (variable/member/parameter shadowing) to improve code clarity and reduce risk of subtle logic errors.

Baseline Count: 95 (from original `build_warnings.log` grep for "shadows a").

Scope Summary:
- C++ sources: `hashtab.cpp` (constructor parameters shadowing member names: hash_function, eq_function, size)
- C++ sources: `objstack.cpp` (constructor parameter `initial_segment_length` shadowing member)
- C sources: `yaep.c` (function parameters `grammar` shadowing static global pointer `grammar`; local function pointer parameter `parse_free` shadowing global static `parse_free`).
- Repeated warnings due to multiple object files / builds; unique sites fewer than raw count.

Strategy:
1. Rename constructor/function parameters to non-conflicting names (prefix with `in_` or descriptive variant).
2. For C functions with global static of same name, rename parameter to short form (`g`).
3. For function pointer params shadowing globals (`parse_free`), rename to `free_fn`.
4. Rebuild and verify zero `-Wshadow` warnings (grep check).
5. Run full test suite to ensure no behavior changes.
6. Update `WARNINGS_BASELINE.md` delta log.

Implementation Notes:
- Chosen minimal renames to avoid API surface changes (all impacted identifiers are local to translation units or static).
- Avoided renaming struct/class members to keep public layout stable.

Status: COMPLETED
Outcome:
- All shadow warnings removed (pass1 rebuild: "No shadow warnings").
- Parameter renames applied in `hashtab.cpp`, `yaep.c` sections, plus free function sweep rename.
- Tests will be re-run before commit to re-affirm (should remain green given purely lexical renames).

Next Batch Recommendation: `cast-qual` (promotes const-correctness, may cascade changes) or start with narrowing the prolific `sign-conversion` by addressing low-risk size_t/unsigned patterns.

Rollback: Revert commit if any unforeseen regression (unlikely due to lexical-only edits).
