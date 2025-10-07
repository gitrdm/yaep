# Bison Error Handling Analysis

## Executive Summary

**Question:** Is Bison the problem with setjmp/longjmp in YAEP?

**Answer:** **NO.** Bison is not the problem. Our misuse of Bison's error handling is the problem.

## Background

During C17 modernization planning, we identified Phase 4 as "Grammar Parser Migration" with the deliverable:
> "Eliminate setjmp/longjmp in Bison-generated code"

This implied Bison itself was using setjmp/longjmp. **This is incorrect.**

## Investigation

### What We Found

1. **Bison-generated code does NOT use setjmp/longjmp**
   - Examined `fuzz/build-fuzz/sgramm.c` (1825 lines of generated code)
   - No occurrences of `YYABORT`, `YYACCEPT`, or `YYERROR` (which might use longjmp)
   - Bison uses normal function calls and return values

2. **The problem is in OUR code:**
   ```c
   // src/sgramm.y:388
   int yyerror(const char *str) {
       yaep_error(YAEP_DESCRIPTION_SYNTAX_ERROR_CODE,
                  "description syntax error on ln %d", ln);  // ← longjmp here!
       return 0;  // Never reached
   }
   ```

3. **The setjmp is also in OUR code:**
   ```c
   // src/sgramm.y:423
   static int set_sgrammar(struct grammar *g, const char *grammar) {
       // ...
       if ((code = setjmp(error_longjump_buff)) != 0) {  // ← Catches longjmp
           free_sgrammar();
           return code;
       }
       // ...
       yyparse();  // ← May call yyerror which does longjmp
   ```

## How Bison Error Handling Actually Works

### Normal Flow (How Bison Expects It)

```
┌─────────────────────────┐
│ yyparse() running       │
└────────┬────────────────┘
         │
         ├─ Syntax error detected
         │
         ▼
┌─────────────────────────┐
│ yyerror(msg) called     │  ← Custom error handler
└────────┬────────────────┘
         │
         ├─ Log error message
         ├─ Set error flag
         │
         ▼
┌─────────────────────────┐
│ yyerror() RETURNS       │  ← Returns to Bison
└────────┬────────────────┘
         │
         ▼
┌─────────────────────────┐
│ Bison error recovery    │  ← Attempts to recover
└────────┬────────────────┘
         │
         ├─ Recovery successful? Continue
         ├─ Recovery failed?
         │
         ▼
┌─────────────────────────┐
│ yyparse() returns 1     │  ← Returns error code
└────────┬────────────────┘
         │
         ▼
┌─────────────────────────┐
│ Caller checks result    │
└─────────────────────────┘
```

### What We Were Doing (Wrong)

```
┌─────────────────────────┐
│ yyparse() running       │
└────────┬────────────────┘
         │
         ├─ Syntax error detected
         │
         ▼
┌─────────────────────────┐
│ yyerror(msg) called     │
└────────┬────────────────┘
         │
         ├─ yaep_error() called
         │
         ▼
┌─────────────────────────┐
│ longjmp() executed!     │  ← BYPASSES BISON
└────────┬────────────────┘
         │
         ├─ Stack unwound
         ├─ Bison context lost
         │
         ▼
┌─────────────────────────┐
│ setjmp() in             │
│ set_sgrammar() catches  │
└─────────────────────────┘

Result:
- Bison's error recovery never runs
- Bison's cleanup never runs
- Thread-unsafe (global jmp_buf)
- Complex control flow
```

## The Fix

### Simple Change to yyerror()

**Before:**
```c
int yyerror(const char *str) {
    yaep_error(YAEP_DESCRIPTION_SYNTAX_ERROR_CODE,
               "description syntax error on ln %d", ln);
    return 0;  // Never reached due to longjmp
}
```

**After:**
```c
int yyerror(const char *str) {
    (void)str;  // Bison's message less informative
    
    // Just record error - DON'T longjmp
    yaep_set_error(grammar, YAEP_DESCRIPTION_SYNTAX_ERROR_CODE,
                   "description syntax error on ln %d", ln);
    
    return 0;  // Return to Bison
}
```

### Updated set_sgrammar()

**Before:**
```c
static int set_sgrammar(struct grammar *g, const char *grammar) {
    int code;
    
    if ((code = setjmp(error_longjump_buff)) != 0) {
        free_sgrammar();
        return code;
    }
    
    // ... setup ...
    yyparse();  // May longjmp
    // ... cleanup ...
    return 0;
}
```

**After:**
```c
static int set_sgrammar(struct grammar *g, const char *grammar_desc) {
    // ... setup with auto-cleanup ...
    
    // Call parser - it returns normally even on error
    int result = yyparse();
    if (result != 0) {
        // Error already recorded by yyerror()
        return grammar->error_code;
    }
    
    // ... post-processing ...
    return 0;
}
```

## Benefits of Correct Usage

### 1. **Simpler Code**
- No setjmp/longjmp
- Normal function returns
- Clear control flow

### 2. **Better Error Recovery**
- Bison can attempt recovery
- Multiple errors can be reported
- More informative error messages

### 3. **Thread-Safe**
- No global jmp_buf
- Each parse independent
- Concurrent parsing possible

### 4. **Standard Practice**
- Follows Bison documentation
- Works like every other Bison parser
- Easier to maintain

### 5. **Easier Debugging**
- Stack traces make sense
- No mysterious jumps
- gdb works normally

## Bison's Error Recovery Features We Were Missing

### 1. **Error Token**
Bison has a special `error` token for error productions:
```yacc
stmt: IF expr THEN stmt
    | WHILE expr DO stmt
    | error ';'  // ← Error recovery production
    ;
```

We could add these to improve error reporting!

### 2. **Multiple Error Reporting**
With normal returns, Bison can report multiple errors in one parse.

### 3. **Error Position Tracking**
Bison maintains position information we were discarding.

## Impact on Migration Plan

### Original Estimate
- **Phase 4:** 2-3 days
- **Risk:** Medium
- **Deliverables:** 
  1. Refactor sgramm.y parser
  2. Update set_sgrammar()
  3. Eliminate setjmp/longjmp in Bison-generated code

### Revised Estimate
- **Phase 4:** 1 day
- **Risk:** Low
- **Deliverables:**
  1. Change yyerror() to use yaep_set_error()
  2. Update set_sgrammar() to check yyparse() result
  3. No changes to Bison code needed

### Timeline Impact
- **Original:** 16 days total
- **Revised:** 13-15 days total
- **Savings:** 1-3 days

## Lessons Learned

### 1. **Don't Fight Your Tools**
Bison is designed to work a certain way. Fighting it with longjmp made things worse, not better.

### 2. **Read the Documentation**
Bison's documentation clearly explains error handling. We should have followed it.

### 3. **Simple is Better**
The "clever" solution (setjmp/longjmp) was more complex and buggy than the simple solution (check return value).

### 4. **Test Assumptions**
We assumed Bison was the problem without investigating. Always verify assumptions.

## Conclusion

**Bison is not the problem.** It's a well-designed tool that uses standard error handling:

- Errors reported via `yyerror()` callback
- Parser returns error code
- No setjmp/longjmp involved

Our mistake was implementing `yyerror()` to do `longjmp()`, which:
- Bypassed Bison's error recovery
- Created thread-safety issues
- Complicated the codebase
- Prevented us from using Bison's features

**The fix is trivial:** Let `yyerror()` return normally and check `yyparse()`'s return value.

This is a perfect example of how understanding your tools leads to simpler, better code.

---

**Document Version:** 1.0  
**Date:** October 7, 2025  
**Status:** Analysis Complete
