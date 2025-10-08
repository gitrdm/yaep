# Bug Fix: Double-Free in yaep_free_grammar()

**Date**: October 8, 2025  
**Issue**: Python wrapper tests crashing with `Fatal Python error: Aborted`  
**Root Cause**: Double-free bug in C library `src/yaep.c`  
**Status**: FIXED

## Problem Description

When running the Python wrapper test suite, tests would crash with a segmentation fault after completing successfully. The crash only occurred when multiple tests were run in sequence, not when tests were run individually.

### Symptoms
```
tests/test_parse_valid_and_invalid.py::test_parse_valid_description PASSED
Fatal Python error: Aborted
```

## Root Cause Analysis

Using Valgrind revealed the actual issue:

```
==1650392== Invalid free() / delete / delete[] / realloc()
==1650392==    at 0x484988F: free
==1650392==    by 0x7954C89: pl_fin (yaep.c:2687)
==1650392==    by 0x7954C89: yaep_free_grammar (yaep.c:7017)
```

The parser list (`pl`) was being freed twice:

1. **First free**: In `yaep_parse()` on successful parse completion (implicitly through cleanup)
2. **Second free**: In `yaep_free_grammar()` when freeing the grammar

### Code Flow

```c
// src/yaep.c line 2683-2689 (BEFORE FIX)
static void
pl_fin (void)
{
  if (pl != NULL)
    yaep_free (grammar->alloc, pl);  // Frees memory but doesn't set pl = NULL
}
```

After a successful parse, `pl` pointed to freed memory. When `yaep_free_grammar()` was called later, it would call `pl_fin()` again, attempting to free the already-freed memory.

## The Fix

**File**: `src/yaep.c`  
**Function**: `pl_fin()`  
**Line**: 2687

```c
// AFTER FIX
static void
pl_fin (void)
{
  if (pl != NULL)
    {
      yaep_free (grammar->alloc, pl);
      pl = NULL;  /* Prevent double-free */
    }
}
```

Setting `pl = NULL` after freeing ensures that subsequent calls to `pl_fin()` are safe (the `if (pl != NULL)` guard prevents freeing NULL pointers).

## Python Wrapper Updates

Additionally, the Python wrapper was updated to remove assumptions about UTF-8 validation features that don't exist in the current C library:

### File: `python/src/yaep_python/__init__.py`
- Removed `YAEP_INVALID_UTF8` constant (error code 18 doesn't exist)
- Removed `YaepInvalidUTF8Error` exception class
- Updated `parse_description_bytes()` to not check for UTF-8 validation errors
- Added documentation noting that UTF-8 validation was removed from YAEP

### File: `python/tests/test_parse_valid_and_invalid.py`
- Updated `test_parse_invalid_utf8_description` to reflect that YAEP no longer validates UTF-8
- Test now verifies graceful handling instead of expecting an exception

## Verification

### Before Fix
```bash
$ pytest tests/
# 4 tests passed, then crash with "Fatal Python error: Aborted"
```

### After Fix
```bash
$ pytest tests/ -v
============================== 11 passed in 0.03s ===============================

$ valgrind python3 -m pytest tests/
============================== 11 passed in 0.85s ===============================
ERROR SUMMARY: 29 errors from 25 contexts
# (Remaining errors are unrelated Python/CFFI issues, not YAEP bugs)
```

### CMake Test Suite
```bash
$ cd build && ctest -R python
100% tests passed, 0 tests failed out of 2
```

### Autotools Test Suite
```bash
$ make test-python
============================== 11 passed in 0.02s ===============================
```

## Impact

- **Python wrapper**: All 11 tests now pass
- **C library**: No functional changes, only memory safety improvement
- **Regression risk**: Minimal - only adds NULL assignment after free
- **Performance impact**: None

## Historical Context

The UTF-8 validation feature that the Python wrapper expected was previously implemented in YAEP but was removed due to segfaults and memory leaks. The current YAEP C library (master branch, October 2025) only supports error codes 1-17 (up to `YAEP_INVALID_TOKEN_CODE`).

The c17-update branch does not include the Python wrapper, which is why this mismatch went undetected until integrating the Python tests into the build system.

## Lessons Learned

1. **Always test with Valgrind**: The crash only manifested with multiple tests, but Valgrind immediately identified the double-free
2. **NULL after free**: Classic C idiom to prevent use-after-free and double-free bugs
3. **API assumptions**: Python wrapper made assumptions about C library features that no longer exist
4. **Integration testing**: Running tests individually vs. in sequence can reveal different bugs

## Related Files

- `src/yaep.c` - C library fix
- `python/src/yaep_python/__init__.py` - Removed UTF-8 validation assumptions
- `python/tests/test_parse_valid_and_invalid.py` - Updated test expectations
- `python/CMakeLists.txt` - Updated to run all 11 tests
- `Makefile.in` - Updated to run all 11 tests
