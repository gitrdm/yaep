# Valgrind Fix: Use-After-Free in Python Wrapper

**Date**: October 8, 2025  
**Issue**: Invalid read/use-after-free in `yaep_read_grammar` callbacks  
**Root Cause**: Python CFFI temporary strings freed before C library finished using them  
**Status**: FIXED

## Problem Description

When running Python tests with Valgrind, multiple "Invalid read" errors were detected in the C library when it accessed string pointers returned by Python callbacks.

### Valgrind Errors
```
==1653994== Invalid read of size 1
==1653994==    at 0x7948407: symb_repr_hash (yaep.c:424)
==1653994==    by 0x7947C8C: find_hash_table_entry (hashtab.c:220)
==1653994==    by 0x794F438: symb_find_by_repr (yaep.c:649)
==1653994==    by 0x794F438: yaep_read_grammar_internal (yaep.c:4098)

==1653994==  Address 0x53ebba1 is 49 bytes inside a block of size 50 free'd
==1653994==    at 0x484988F: free
==1653994==    by 0x5432F9: list_dealloc (listobject.c:565)
```

The C library was reading from freed memory - classic use-after-free bug.

## Root Cause Analysis

The Python wrapper's `read_grammar_from_lists()` function uses CFFI callbacks to provide grammar data to the C library. The problem was in how C strings were created:

### BEFORE (Buggy Code)
```python
@_ffi.callback("const char *(int *)")
def read_terminal(code_ptr):
    # ... code ...
    return _ffi.new("char[]", name.encode('utf-8'))  # ❌ TEMPORARY - gets freed!

@_ffi.callback("const char *(const char ***, const char **, int *, int **)")  
def read_rule(rhs_ptr, abs_node_ptr, anode_cost_ptr, transl_ptr):
    # ... code ...
    rhs_c = [_ffi.new("char[]", s.encode('utf-8')) for s in rhs]  # ❌ TEMPORARY
    rhs_array = _ffi.new("const char *[]", rhs_c)  # ❌ TEMPORARY
    # ... code ...
    return _ffi.new("char[]", rule['lhs'].encode('utf-8'))  # ❌ TEMPORARY
```

The issue:
1. `_ffi.new()` creates C memory managed by CFFI
2. When the callback returns, Python's reference counting allows these objects to be freed
3. The C library stores the pointers and uses them later
4. By the time the C library accesses the strings, they've been freed
5. **Result**: Use-after-free → undefined behavior → crashes or corruption

### Call Flow Timeline
```
1. Python calls yaep_read_grammar(...)
2. C library calls read_terminal() callback
3. Python creates temp string with _ffi.new("char[]", ...)
4. Python returns pointer to C
5. Callback returns, Python GC may free the string  ← BUG HERE
6. C library stores pointer in hash table
7. Later: C library calls symb_repr_hash() on stored pointer
8. ❌ Access freed memory → Valgrind error
```

## The Fix

**File**: `python/src/yaep_python/_cffi.py`  
**Function**: `read_grammar_from_lists()`

Add a `keepalive` list to the callback state that holds references to all C string allocations until `yaep_read_grammar()` completes:

### AFTER (Fixed Code)
```python
state = {
    'terminals': terminals,
    'rules': rules,
    'term_idx': 0,
    'rule_idx': 0,
    'keepalive': []  # ✅ Keep C strings alive
}

@_ffi.callback("const char *(int *)")
def read_terminal(code_ptr):
    # ... code ...
    # Keep the C string alive by storing it in keepalive list
    name_c = _ffi.new("char[]", name.encode('utf-8'))
    state['keepalive'].append(name_c)  # ✅ Prevents premature free
    return name_c

@_ffi.callback("const char *(const char ***, const char **, int *, int **)")  
def read_rule(rhs_ptr, abs_node_ptr, anode_cost_ptr, transl_ptr):
    # ... code ...
    
    # RHS: array of strings - keep all C strings alive
    rhs_c = [_ffi.new("char[]", s.encode('utf-8')) for s in rhs]
    state['keepalive'].extend(rhs_c)  # ✅ Keep alive
    rhs_array = _ffi.new("const char *[]", rhs_c)
    state['keepalive'].append(rhs_array)  # ✅ Keep alive
    
    # abs_node - keep C string alive
    if abs_node:
        abs_node_c = _ffi.new("char[]", abs_node.encode('utf-8'))
        state['keepalive'].append(abs_node_c)  # ✅ Keep alive
    
    # transl array - keep alive
    transl_array = _ffi.new("int[]", transl_c)
    state['keepalive'].append(transl_array)  # ✅ Keep alive
    
    # LHS - keep C string alive
    lhs_c = _ffi.new("char[]", rule['lhs'].encode('utf-8'))
    state['keepalive'].append(lhs_c)  # ✅ Keep alive
    return lhs_c
```

The `keepalive` list maintains Python references to all CFFI-allocated C strings. Python's reference counting ensures they stay alive until the `state` dict goes out of scope (when `read_grammar_from_lists()` returns), which is after `yaep_read_grammar()` completes.

## Verification

### Before Fix
```bash
$ valgrind python3 -m pytest tests/test_read_grammar.py
==1653994== Invalid read of size 1
==1653994==    at 0x7948407: symb_repr_hash (yaep.c:424)
# ... many more errors ...
ERROR SUMMARY: 29 errors from 25 contexts
```

### After Fix
```bash
$ valgrind --error-exitcode=1 python3 -m pytest tests/
============================== 11 passed in 1.51s ===============================
ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
# Exit code: 0 ✅
```

### All Tests with Valgrind
```bash
$ valgrind --leak-check=full --track-origins=yes --error-exitcode=1 \
    python3 -m pytest tests/ -v

11 passed in 1.51s
HEAP SUMMARY:
    in use at exit: 706,924 bytes in 403 blocks (Python runtime, not leaks)
    definitely lost: 0 bytes in 0 blocks
    indirectly lost: 0 bytes in 0 blocks
    possibly lost: 0 bytes in 0 blocks
ERROR SUMMARY: 0 errors from 0 contexts ✅
```

## Impact

- **Memory safety**: Eliminated use-after-free bugs in Python wrapper
- **Tests**: All 11 tests pass cleanly under Valgrind
- **Performance**: Minimal (small temporary list during grammar reading)
- **Code quality**: Proper CFFI memory management
- **Compatibility**: No API changes, purely internal fix

## Related Bugs

This fix complements the earlier double-free fix in `yaep.c:pl_fin()`. Both issues were memory management bugs but in different parts of the codebase:

1. **C library bug** (BUGFIX_DOUBLE_FREE.md): Double-free of parser list
2. **Python wrapper bug** (this fix): Use-after-free of callback strings

## Technical Details

### CFFI Memory Management
CFFI's `ffi.new()` creates objects with Python reference counting:
- Reference count > 0: Memory stays alive
- Reference count = 0: Python may free it anytime
- Storing in Python list: Increments reference count

### Why Lists Work
```python
state['keepalive'].append(name_c)
```
- `state` is a local variable in `read_grammar_from_lists()`
- `state['keepalive']` holds references to all C strings
- Python keeps these alive until `state` goes out of scope
- This happens after `yaep_read_grammar()` returns
- Timeline: Create → Use in C → Return from C → Free ✅

### Alternative Solutions Considered

1. **`ffi.gc()`**: Would work but overkill for short-lived allocations
2. **Global state**: Would work but bad practice
3. **Manual memory management**: Complex, error-prone
4. **Copying in C**: Would require modifying C library
5. **`keepalive` list**: ✅ Simple, Pythonic, correct

## Lessons Learned

1. **Valgrind is essential** for catching memory bugs in C/Python interop
2. **CFFI lifetime rules**: Must keep references to prevent premature GC
3. **Callback pitfalls**: Data lifetime extends beyond callback return
4. **Test with tools**: Bugs invisible to pytest are visible to Valgrind

## Related Files

- `python/src/yaep_python/_cffi.py` - Fixed `read_grammar_from_lists()`
- `src/yaep.c` - C library (unchanged, but shows where pointers are used)
