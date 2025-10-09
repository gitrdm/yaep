# P1 Warning Elimination Plan

Schema Version: 1
Baseline Tag: `refactor-baseline`
Total Raw Warnings: 1787
Index File: `WARNINGS_INDEX.csv`
Top 30 Signatures: `warnings_signature_top30.txt`

## 1. Categorization Buckets (Initial)
| Bucket | Description | Est Count | Strategy | Done? |
|--------|-------------|-----------|----------|-------|
| sign-conversion | Signed/unsigned & width conversions | ~196 + related | Introduce tmp vars with explicit casts; prefer size_t for counts | |
| conversion | Narrowing/other conversions | ~200 (overlap) | Audit each; adjust types or add safe casts | |
| cast-qual | Discarding const qualifiers | 127 + related | Refactor APIs to accept const where safe; else isolate cast in helper | |
| old-style-cast | C-style casts in C++ sources | ~600 aggregated | Replace with static_cast / reinterpret_cast / const_cast as appropriate | |
| shadow | Variable shadowing (grammar, parse_free, etc.) | ~54 | Rename locals; unify naming convention suffix _local | |
| maybe-uninitialized | Potential uninitialized reads | handful | Initialize or restructure logic; add defensive asserts if needed | |
| parser-conflict | Shift/reduce from test grammars | 1 | Document acceptable; suppress via test-only flags if desired | |
| missing-prototypes | None observed yet? | 0 | Monitor after refactors | |
| other | Rare/singletons | <50 | Address ad hoc | |

## 2. Execution Order (Fail-Fast Risk First)
1. maybe-uninitialized (potential UB)
2. cast-qual (const correctness ripples early)
3. shadow (clarity; prevents future misreads)
4. sign-conversion / conversion (grouped by module to reduce churn)
5. old-style-cast (mechanical; batch C++ then C casts that matter)
6. residual / other
7. Optional: selective suppression for generated parser conflicts

## 3. Batch Protocol
For each batch:
- Derive filtered subset: grep bucket from `WARNINGS_INDEX.csv`.
- Create `P1_BATCH_<NAME>_PLAN.md` with scope & acceptance criteria.
- Implement minimal fixes; run full build & tests; snapshot delta warnings.
- Append entry to `WARNINGS_BASELINE.md` changelog section.
- No mixed-category commits (atomic closure per bucket slice).

## 4. Tooling Aids (Future Enhancements)
- Script to diff warnings between HEAD and baseline tag, failing CI on net new categories.
- Optional Python parser to produce JSON summary for dashboards.

## 5. Non-Goals (P1)
- Refactoring algorithmic structures.
- Performance modifications (except trivial type changes with no semantic impact).
- Introducing new dependencies.

## 6. Exit Criteria for P1
- All high-risk categories (maybe-uninitialized, cast-qual, shadow) = 0.
- >=90% reduction in sign/width conversion warnings OR justified doc for remaining.
- Old-style casts eliminated from C++ and reduced in C where safe.
- `WARNINGS_BASELINE.md` updated with before/after counts.
- CI guard added preventing regression.

## 7. Immediate Next Step
Generate first batch plan: `P1_BATCH_maybe_uninitialized_PLAN.md`.

---
(Generated automatically – edit as needed before execution.)
