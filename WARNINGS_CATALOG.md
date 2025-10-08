# YAEP Warning Catalog (C++17 Strict Mode)

**Date:** October 8, 2025  
**Branch:** yaep-expansion  
**Compiler:** GCC with `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wold-style-cast -Wcast-qual`

## Summary Statistics

| Category | Count | Priority |
|----------|-------|----------|
| **Total Warnings** | 346 | - |
| Old-style-cast | 263 | Medium |
| Sign-conversion | 48 | Medium-Low |
| Wconversion (value) | 26 | Medium-Low |
| Cast-qual | 8 | High (partially resolved) |
| Generated code (sgramm.y) | 27 | Low (defer/suppress) |

## Cast-Qual Analysis (8 warnings)

### Status: Mostly Resolved
We've centralized intentional qualifier discards and fixed critical bugs.

#### Remaining Sources

1. **src/yaep.c:2182** - `set_term_lookahead_hash`
   - Cause: Passing const structure address through hash API
   - Options: Accept as centralized policy; add comment referencing helper pattern
   
2. **src/yaep.c:6212, 6263** - `anode.name` storage/free
   - Cause: Names treated as const for most of pipeline, but hashed/freed via void*
   - **Action:** Audit field type; if always heap-allocated, change struct field to `char *`
   
3. **sgramm.y:492, 500, 694** (5 instances in generated code)
   - Comparators and rule reader in generated grammar
   - **Action:** Bracket generated inclusion with `#pragma GCC diagnostic ignored "-Wcast-qual"`

### Recommendation
- Document remaining 3 hand-code instances as intentional (2182, 6212, 6263)
- Suppress generated code warnings with pragma push/pop

---

## Old-Style-Cast Analysis (263 warnings)

### Breakdown by Target Type

| Target Type | Count | Remediation Approach |
|-------------|-------|---------------------|
| `void*` | 37 | `static_cast<void*>` for data pointers |
| `char*` | 32 | `static_cast<char*>` for pointer arithmetic; `reinterpret_cast` where needed |
| `hash_table_entry_t` (void*) | 13 | `const_cast` for table mutations |
| `int*` | 12 | `static_cast<int*>` for VLO/array access |
| `size_t` | 11 | `static_cast<size_t>` for hashing/indexing |
| `struct parse_state**` | 10 | `static_cast` for VLO boundary access |
| Various struct pointers | ~148 | `static_cast` (type-safe casts from base/void) |

### Strategy

**Phase 1: Macro Expansions (Quick Win)**
- Target: VLO_BEGIN, VLO_BOUND, OS_TOP_BEGIN macros
- These expand to typed pointers from `void*` methods
- Wrap expansion results in C++ conditional:
  ```cpp
  #ifdef __cplusplus
  #define VLO_BEGIN(vlo) static_cast<decltype((vlo).begin())>((vlo).begin())
  #else
  #define VLO_BEGIN(vlo) ((vlo).vlo_start)
  #endif
  ```

**Phase 2: Hash Table Casts**
- Centralize hash_table_entry_t casts in helpers (like find_hash_table_entry_c)
- Use const_cast for table mutations, static_cast for retrieval

**Phase 3: Pointer Arithmetic**
- Convert (char*) for pointer arithmetic to reinterpret_cast<char*>
- Convert back-casts to static_cast

**Phase 4: Generated Code**
- Pragma-suppress sgramm.y warnings

---

## Conversion Warnings (74 total)

### Sign-Conversion (48 instances)

**Common Patterns:**
- `int` → `size_t` (28 instances) - VLO/OS expansions, array indexing
- `int` → `unsigned int` (8 instances) - hash computation
- `long unsigned int` → `int` (12 instances) - debug counters

**Root Causes:**
- Legacy API uses `int` for sizes; modern stdlib uses `size_t`
- Hash functions return `unsigned` but intermediate math uses `int`

**Options:**
1. **Accept** - Sign conversions are mostly safe in this domain (parser sizes << INT_MAX)
2. **Suppress selectively** - Pragma around hot loops
3. **Refactor** (high effort) - Change internal APIs to size_t

**Recommendation:** Accept with comment; focus on cast-qual and old-style-cast first.

### Value-Conversion (26 instances)

- `size_t` → `int` (9 instances) - counters, indices
- `int` → `char` (5 instances) - flag fields in structs
- `long` → `int` (7 instances) - pointer diffs

**Recommendation:** Document assumptions (e.g., "grammar size fits in int"); low priority.

---

## Generated Code (sgramm.y - 27 warnings)

### Sources
- Bison-generated parser for grammar description language
- Old-style casts in comparators, lexer actions
- Cast-qual in qsort callbacks

### Strategy
Wrap inclusion in yaep.c with pragma:
```c
#ifdef __cplusplus
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include "sgramm.y"
#ifdef __cplusplus
#pragma GCC diagnostic pop
#endif
```

**Justification:** Generated code is stable; regeneration is complex; pragma localizes noise.

---

## Next Actions (Priority Order)

### High Priority (Correctness/API)
1. ✅ **DONE:** Fix abstract node name retention bug (tests 45/47)
2. ✅ **DONE:** Centralize const qualifier discards (find_hash_table_entry_c)
3. ✅ **DONE:** Fix missing prototypes, format warnings, shadow warnings
4. **TODO:** Audit anode.name field type (const char* vs char*)

### Medium Priority (Warning Reduction)
5. **TODO:** Add pragma suppression for sgramm.y (27 warnings eliminated)
6. **TODO:** Convert VLO/OS macro casts to C++ static_cast (est. ~50 warnings)
7. **TODO:** Convert hash table casts to const_cast/static_cast (est. ~25 warnings)

### Low Priority (Deferred)
8. Sign-conversion in size computations (accept with comment)
9. Old-style casts in rarely-executed debug/error paths
10. Value conversions in struct flag fields

---

## Build Validation

Current status: **All 128 tests pass** ✅

Baseline warning count: 346 (C++ build with strict flags)

Target after Phase 1 (pragmas + macro fixes): ~250 warnings  
Target after Phase 2 (hash helpers): ~200 warnings  
Final target: <100 warnings (conversion/sign warnings documented as accepted)

---

## Notes for Future Maintainers

1. **Const Correctness:** The hash table API stores `void*` but probes accept `const void*` via helper. This is intentional—keys are never mutated through the table.

2. **Generated Code:** sgramm.y warnings are suppressed via pragma. If regenerating grammar, review pragma scope.

3. **Size Types:** Internal APIs use `int` for historical reasons. All grammar sizes are validated << INT_MAX at parse time.

4. **C/C++ Hybrid:** yaep.c is compiled as both C and C++. Use `#ifdef __cplusplus` for cast style.
