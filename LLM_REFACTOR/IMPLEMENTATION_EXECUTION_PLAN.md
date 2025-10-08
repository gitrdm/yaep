# YAEP Refactor & Optimization Execution Plan (LLM-Oriented Checklist)

**Directory:** `LLM_REFACTOR/`  
**Branch:** `yaep-expansion`  
**Script to run builds/tests:** `./build_and_test.sh` (must succeed before any commit)  
**Style:** Strict checklist (no narrative). Each step = atomic, testable, commit-ready.  
**Legend:** `[ ] = Pending` / `[~] = In Progress` / `[x] = Done` / `⚠` = Attention Required

---

## 0. Coding Standards (Hard Requirements — MUST be enforced in every step)

| # | Standard | Enforcement Mechanism | Violation Action |
|---|----------|-----------------------|------------------|
| 1 | No stubs / placeholders / TODOs | Grep for `TODO|FIXME|STUB` (must be zero) | Reject change |
| 2 | Production-ready code only | All tests + valgrind clean | Reject change |
| 3 | New tests for each new code path | Added test file / test case ID in commit | Reject change |
| 4 | Full rebuild + test via `build_and_test.sh` per logical step | Script exit code 0 | Do not commit |
| 5 | Literate, explanatory comments for new/changed logic & tests | Diff review checklist | Amend before commit |
| 6 | Frequent atomic commits (one logical change) | Git history inspection | Squash/fixup if violated |
| 7 | Zero new warnings (treat warnings as errors conceptually) | Diff warning delta = 0 | Fix before commit |
| 8 | No regressions (tests, performance baseline if applicable) | Baseline compare logs | Revert or fix |
| 9 | Consistent formatting (no reflow of unrelated code) | Minimal diff review | Reject drift |
| 10| Document every non-trivial design decision inline | Code comments + commit msg | Amend |

**Automated Pre-Commit Checklist (ALL must pass):**
```
./build_and_test.sh                          # Must exit 0
grep -R "TODO\|FIXME\|STUB" -n src/ || true  # Must produce NO matches
grep -i "warning" build_warnings.log && echo "FAIL (warnings present)" && exit 1 || true
valgrind (weekly / milestone gates)          # 0 errors, 0 leaks
```

---

## 1. High-Level Phase Map (Sequential Gate Progression)

| Phase | Goal | Must Pass Gate To Advance |
|-------|------|---------------------------|
| P0 | Baseline & Hygiene | Clean baseline snapshot logged |
| P1 | Warning Elimination | Zero warnings gate report |
| P2 | Modularization (Engine + Analysis separation) | All tests green, no behavior change |
| P3 | Optimization Infrastructure (Nullable, Dedup, Pools, Bench) | Benchmarks captured baseline vs infra |
| P4 | Leo Optimization | Right-recursive perf improves (>=2x) |
| P5 | SPPF (Packed Parse Forest) | Ambiguous memory reduction proven |
| P6 | Pruning + Aycock–Horspool + Final Tuning | Aggregate perf report complete |
| P7 | Documentation & Hardening | All docs, coverage, valgrind, final QA |

Each phase gated by: (1) All prior checklist items `[x]`, (2) build/test success, (3) zero warnings, (4) added tests.

---

## 2. Phase P0 – Baseline & Hygiene

| ID | Item | Details | Status |
|----|------|---------|--------|
| P0-001 | Baseline build log | Run `./build_and_test.sh` store `logs/baseline_build_01.log` | [ ] |
| P0-002 | Inventory warnings | Extract unique warnings → `LLM_REFACTOR/WARNINGS_BASELINE.md` | [ ] |
| P0-003 | Inventory tests | List current C & Python tests → `LLM_REFACTOR/TEST_INVENTORY.md` | [ ] |
| P0-004 | Establish performance micro-benchmark harness skeleton | Create `bench/` dir + placeholder driver (real logic added later) | [ ] |
| P0-005 | Capture initial perf (parsing 3 representative grammars) | Save JSON: `LLM_REFACTOR/perf_baseline.json` | [ ] |
| P0-006 | Valgrind full pass (sanity) | Store report `LLM_REFACTOR/valgrind_baseline.txt` | [ ] |
| P0-007 | Tag baseline (annotated) | `git tag refactor-baseline` post-verification | [ ] |

**Gate Report Artifact:** `LLM_REFACTOR/GATE_P0_REPORT.md`

---

## 3. Phase P1 – Warning Elimination (Strict Ordering)

Sub-Phases: P1A Type Conversions → P1B Shadowing → P1C Old-style Casts → P1D Parser/Generated.

| ID | Scope | Actions | Tests Added | Status |
|----|-------|---------|-------------|--------|
| P1A-001 | hashtab conversions | Replace implicit size_t→unsigned casts, add assert helpers | `test_hashtab_conversions.c` | [ ] |
| P1A-002 | objstack conversions | Normalize types to size_t/ptrdiff_t | Extend existing objstack test | [ ] |
| P1A-003 | vlobject conversions | Fix char/int narrowing, add boundary tests | New edge case test | [ ] |
| P1A-004 | Introduce safe cast inline funcs | Add `safe_size_to_uint` in header (with docs) | Verified via unit test of boundaries | [ ] |
| P1A-005 | Rebuild + zero new warnings check | Capture diff log | - | [ ] |
| P1B-001 | hashtab shadowing | Rename ctor params; update docs | Existing tests | [ ] |
| P1B-002 | objstack shadowing | Rename conflicting locals | Existing tests | [ ] |
| P1B-003 | Add naming convention doc | Add `LLM_REFACTOR/NAMING_GUIDE.md` | - | [ ] |
| P1C-001 | Replace casts vlobject.cpp | static_cast / reinterpret_cast | C++ test compile | [ ] |
| P1C-002 | Replace casts objstack.cpp | static_cast usage | C++ test compile | [ ] |
| P1C-003 | Replace casts test/ticker.cpp | static_cast usage | Re-run ticker tests | [ ] |
| P1D-001 | Grammar shift/reduce expectation clarity | Add %expect where needed | Grammar tests | [ ] |
| P1D-002 | Suppress only generated acceptable warnings | CMake property set | Build log check | [ ] |
| P1D-003 | Final warning sweep (grep) | Confirm zero warnings | - | [ ] |
| P1D-004 | Gate report P1 | `GATE_P1_REPORT.md` | - | [ ] |

**Gate Condition:** `grep -i "warning" build_warnings.log` returns 0 lines.

---

## 4. Phase P2 – Modularization (Behavior-Preserving)

Outputs: `earley_engine.{c,h}`, `grammar_analysis.{c,h}`, updated `CMakeLists.txt`.

| ID | Task | Constraints | Tests | Status |
|----|------|-------------|-------|--------|
| P2-001 | Extract prediction/scan/complete into engine file | No logic change; diff limited | Full suite | [ ] |
| P2-002 | Introduce opaque types for engine handle | No API break at C level | API smoke test | [ ] |
| P2-003 | Extract grammar analysis (nullable placeholder only) | No change in runtime behavior yet | Grammar tests | [ ] |
| P2-004 | Add module-level literate comments (design blocks) | Structured headers per file | - | [ ] |
| P2-005 | Update `doc/Internals.txt` architecture section | ASCII diagram added | - | [ ] |
| P2-006 | Gate diff review (ensure function-by-function move) | Verified via symbol map | - | [ ] |
| P2-007 | Gate report P2 | `GATE_P2_REPORT.md` | - | [ ] |

**Gate Condition:** All tests pass; performance variance <5% vs baseline.

---

## 5. Phase P3 – Optimization Infrastructure (No Behavioral Change)

| ID | Component | Action | Validation | Status |
|----|-----------|--------|------------|--------|
| P3-001 | Nullable infra | Add nullable bitset + compute fn (unused) | Unit test on synthetic grammars | [ ] |
| P3-002 | Dedup infra | Add item hash/fingerprint skeleton (unused) | Hash collision test | [ ] |
| P3-003 | Memory pool infra | Add pool API (not yet adopted) | Leak + stress test | [ ] |
| P3-004 | Benchmark harness | Implement driver: parse N times + timing JSON | Perf JSON existence | [ ] |
| P3-005 | Bench baseline capture | Store `perf_infra_baseline.json` | Diff vs P0 baseline | [ ] |
| P3-006 | Gate report P3 | `GATE_P3_REPORT.md` | - | [ ] |

**Gate Condition:** All infra compiled, inactive by default, zero test diffs.

---

## 6. Phase P4 – Leo Optimization Activation

| ID | Step | Action | Validation | Status |
|----|------|--------|-----------|--------|
| P4-001 | Enable Leo data integration | Link leo_items list in sets | Structural tests | [ ] |
| P4-002 | Implement right-recursive detection | Pattern tests (simple grammar) | [ ] |
| P4-003 | Integrate fast completion hook | Compare item count vs no-Leo run | [ ] |
| P4-004 | Add CLI flag / env to toggle (YAEP_ENABLE_LEO) | Toggle test on/off parity | [ ] |
| P4-005 | Performance measurement (right-recursive suite) | >=2x speed improvement | [ ] |
| P4-006 | Documentation: algorithm + diagrams | Update Internals + code blocks | [ ] |
| P4-007 | Gate report P4 | `GATE_P4_REPORT.md` | - | [ ] |

**Gate Condition:** Right-recursive parse time improvement ≥2x; correctness unchanged.

---

## 7. Phase P5 – SPPF (Packed Parse Forest)

| ID | Step | Action | Validation | Status |
|----|------|--------|-----------|--------|
| P5-001 | SPPF node structs + allocator | Compact union-based layout | Unit struct layout test | [ ] |
| P5-002 | Build path (ambiguous detection hook) | Controlled grammar test | [ ] |
| P5-003 | Packed node insertion logic | Duplicate alternative merging test | [ ] |
| P5-004 | Traversal API (iterator) | API unit + sample program | [ ] |
| P5-005 | Memory usage benchmark vs baseline | Report memory delta | [ ] |
| P5-006 | Toggle (YAEP_ENABLE_SPPF) | Off = legacy tree path parity | [ ] |
| P5-007 | Developer docs (SPPF format) | SPPF README section | [ ] |
| P5-008 | Gate report P5 | `GATE_P5_REPORT.md` | - | [ ] |

**Gate Condition:** Ambiguous grammar memory reduction demonstrated; API stable.

---

## 8. Phase P6 – Pruning, Aycock–Horspool, Final Tuning

| ID | Component | Action | Validation | Status |
|----|-----------|--------|------------|--------|
| P6-001 | Reachability analysis | Precompute reachable set | Test unreachable elimination | [ ] |
| P6-002 | Productive analysis | Precompute productive set | Productive grammar tests | [ ] |
| P6-003 | Prediction pruning integration | Reduced predicted item count | Metrics diff | [ ] |
| P6-004 | Early failure detection | Immediate abort on impossible continuation | Negative tests | [ ] |
| P6-005 | Aycock–Horspool scan grouping | Group by terminal; batch advance | Scan step micro-bench | [ ] |
| P6-006 | Combined optimization synergy test | End-to-end perf suite | Perf summary | [ ] |
| P6-007 | Final perf report draft | `PERF_FINAL_REPORT.md` | Peer review | [ ] |
| P6-008 | Gate report P6 | `GATE_P6_REPORT.md` | - | [ ] |

**Gate Condition:** Aggregate performance improvements documented; no regressions.

---

## 9. Phase P7 – Documentation, Hardening, QA Closure

| ID | Area | Action | Status |
|----|------|--------|--------|
| P7-001 | Valgrind full matrix (all toggles) | 0 leaks, 0 errors | [ ] |
| P7-002 | Warning re-audit (final) | 0 warnings confirmed | [ ] |
| P7-003 | Code coverage run (if infra available) | Coverage summary added | [ ] |
| P7-004 | Security pass (allocation bounds, unchecked casts) | Add notes file | [ ] |
| P7-005 | Public API diff vs baseline | Document intentional additions only | [ ] |
| P7-006 | Final architecture diagram | Embedded in Internals | [ ] |
| P7-007 | Consolidated CHANGELOG update | Added section for refactor | [ ] |
| P7-008 | Tag release `refactor-complete` | Git tag created | [ ] |
| P7-009 | Gate report P7 | `GATE_P7_REPORT.md` | [ ] |

**Gate Condition:** All success criteria from checklist satisfied; release tag created.

---

## 10. Commit Protocol (Every Logical Step)

1. Implement smallest viable cohesive unit (one table row)  
2. Add/extend tests FIRST if new path (red) → implement (green)  
3. Run: `./build_and_test.sh` (must pass)  
4. Run quick static checks:
   - `grep -R "TODO\|FIXME\|STUB" src/` → no output
   - `grep -i warning build_warnings.log` → no output
5. If perf-sensitive step: run benchmark harness & append delta
6. Git add only relevant files (avoid noise)
7. Commit message template:
```
[P<N><SubID>] <Short Task Title>

Summary:
- What changed
- Why (problem + rationale)
- Tests added/updated (list)
- Performance note (if applicable)
- References (issue, paper, doc section)

Validation:
- build_and_test.sh: PASS
- warnings: 0 new
- tests: <N>/<N> passing
- valgrind: (if run) 0 errors
```
8. Push; open PR (if workflow) labeled with phase.

---

## 11. Testing Additions Matrix (Planned New Test Artifacts)

| Area | Test File (Planned) | Purpose |
|------|---------------------|---------|
| Type Conversions | `test/test_cast_safety.c` | Boundary & overflow safety |
| Nullable | `test/test_nullable.c` | Correct fixpoint & edge cases |
| Dedup | `test/test_dedup.c` | Duplicate elimination correctness |
| Leo | `test/test_leo_right_recursion.c` | Complexity reduction proof |
| SPPF | `test/test_sppf_ambiguity.c` | Packed forest integrity |
| Pruning | `test/test_pruning.c` | No valid parse loss |
| Aycock–Horspool | `test/test_scan_grouping.c` | Scan grouping correctness |
| Memory Pool | `test/test_pool_reuse.c` | Allocation reuse behavior |
| Perf Harness | `bench/bench_driver.c` | Timing harness |

---

## 12. Performance Metrics (JSON Schema Target)

`perf_<phase>.json` structure:
```json
{
  "timestamp": "ISO-8601",
  "git_commit": "<hash>",
  "config": { "leo": true, "sppf": false, "pruning": false },
  "grammars": [
    {"name": "right_recursive", "tokens": 5000, "time_ms": 123.4, "items": 45678},
    {"name": "ambiguous", "tokens": 1200, "time_ms": 987.6, "forest_nodes": 3210}
  ],
  "memory": {"rss_kb": 10240, "arena_bytes": 8192},
  "notes": "<optional>"
}
```

---

## 13. Risk Mitigation Hooks

| Risk | Mitigation Step IDs |
|------|---------------------|
| Performance regression | P4-005, P5-005, P6-006, P6-007 |
| Memory growth | P3-003, P5-005, P6-006 |
| API break | P2-002, P5-004, P7-005 |
| Ambiguity handling regression | P5-002, P5-003, P5-004 |
| Over-pruning | P6-003, P6-004 (tests ensure no loss) |

---

## 14. Gate Report Templates (All Phases)

Each `GATE_P<N>_REPORT.md` MUST include:
```
# Gate P<N> Report

Commit: <hash>
Date: <date>

Checklist Summary:
- Items total: <X>
- Items complete: <Y>
- % Complete: <Y/X * 100>

Build/Test:
- build_and_test.sh: PASS
- Warnings: 0
- Tests Passed: <N>/<N>
- Valgrind (if required this phase): PASS/NA

Performance (if applicable):
- Metric table

Deviations / Exceptions:
- (list or 'None')

Approval:
- Reviewed by: <name/initials>
```

---

## 15. Completion Criteria (Exit P7)

| Criterion | Verified By |
|-----------|-------------|
| All checklist items `[x]` | IMPLEMENTATION_EXECUTION_PLAN.md audit |
| All gates have reports | GATE_P*_REPORT.md files |
| Zero warnings | Final build log |
| All new tests stable | 3 consecutive successful runs |
| Performance improvements documented | PERF_FINAL_REPORT.md |
| Memory targets achieved | P5-005, P6-006 artifacts |
| Public API stability documented | P7-005 report |
| Release tag present | `git tag -l refactor-complete` |

---

## 16. Quick Progress Snapshot (Manual Update)

```
P0 Baseline:        [ ]
P1 Warnings:        [ ]
P2 Modularization:  [ ]
P3 Infra:           [ ]
P4 Leo:             [ ]
P5 SPPF:            [ ]
P6 Pruning+Scan:    [ ]
P7 Final QA:        [ ]
```

---

## 17. FIRST ACTIONS (Immediately After Plan Merge)

| ID | Action | Owner | Status |
|----|--------|-------|--------|
| F-001 | Create `logs/` directory (gitignored except baseline logs) | | [ ] |
| F-002 | Run P0-001..P0-003 | | [ ] |
| F-003 | Draft WARNINGS_BASELINE.md from build log | | [ ] |
| F-004 | Create empty perf harness scaffolding | | [ ] |
| F-005 | Commit baseline artifacts (single commit) | | [ ] |

---

End of execution plan.
