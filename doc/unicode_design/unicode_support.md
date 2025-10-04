# YAEP Unicode Support Guide

This note summarises the production-level guarantees YAEP now offers when parsing UTF-8 grammar descriptions and highlights the remaining responsibilities for client code.

## Overview

* **Encoding.** All public APIs expect UTF-8 encoded byte strings. Input is validated up front; malformed UTF-8 is rejected before the lexer allocates any parser state so clients never observe partial mutations.
* **Identifiers.** Identifier start characters follow the Unicode `Letter` classes (`Lu`, `Ll`, `Lt`, `Lm`, `Lo`) plus underscore. Continuation characters additionally allow decimal digits (`Nd`), combining marks (`Mn`, `Mc`), and connector punctuation (`Pc`) per [Unicode® Standard Annex #31](https://www.unicode.org/reports/tr31/).
* **Character literals.** Character literals (`'…'`) now accept any single Unicode scalar value. The literal retains its original UTF-8 bytes for diagnostics while the parser stores the decoded code point exactly once, preventing stale byte-based hashes.
* **Numeric literals.** Decimal numbers inside grammar descriptions accept any `Nd` code point. Literals must use digits from a single script; mixed-script numerals are rejected with a descriptive error to prevent subtle token-code mismatches.

## Normalisation policy

YAEP now performs NFC normalization of grammar symbol names at insertion time.
When a grammar description is read, YAEP will attempt to normalise symbol
names to NFC (using the bundled unicode wrapper). If normalization succeeds,
the normalized form is used for hashing and internal comparisons; identifiers
that are canonically equivalent therefore map to the same internal symbol.

This makes grammars more robust in the presence of equivalent Unicode forms
and reduces the need for callers to pre-normalize grammar text. However,
client code still needs to ensure that runtime tokens (terminal attributes)
match the normalized forms used by YAEP. In practice you can either:

* Produce terminal attributes that are already NFC-normalized (recommended),
  or
* Use token codes instead of textual attributes so runtime matching is
  independent of string normalization.

Note: YAEP's normalization is conservative — if normalization fails during
symbol insertion YAEP falls back to the original byte sequence rather than
aborting grammar construction. If you require strict behaviour, validate and
normalize inputs before calling YAEP's APIs.

### Recommended workflow

1. You may pass grammar descriptions to YAEP without pre-normalizing them; YAEP will normalize symbol names to NFC when reading the grammar.
2. Ensure runtime tokens (terminal attributes) are normalized to NFC before being passed to the parser, or emit token codes to avoid string comparisons entirely.
3. If you prefer to control normalization yourself, use YAEP's helper `yaep_utf8_normalize_nfc()` to obtain allocator-owned normalized strings.

### Sample helper (C API)

```c
#include "yaep_unicode.h" /* exposes yaep_utf8_normalize_nfc */

/* Normalize a NUL-terminated string to NFC into allocator-managed memory.
 * On success returns pointer via *out (owned by allocator), and function
 * returns 1. On failure *out is set to NULL and the function returns 0.
 */
char *maybe_norm = NULL;
if (yaep_utf8_normalize_nfc(original, &maybe_norm, grammar->alloc) == 1) {
    /* use maybe_norm (allocator-owned) as the canonical representation */
} else {
    /* fallback: use original */
}
```

### Sample helper (C++ API)

```cpp
#include "yaep_unicode.h"
std::string normalize_if_needed(const char *s, YaepAllocator *alloc) {
  char *out = nullptr;
  if (yaep_utf8_normalize_nfc(s, &out, alloc) == 1) {
    return std::string(out); /* allocator still owns out; copy as needed */
  }
  return std::string(s);
}
```

## Error Reporting

* Invalid UTF-8 yields `YAEP_INVALID_UTF8` with an error message of the form:
  
  > `invalid UTF-8 in grammar description at byte N: <utf8proc error>`

  The byte index counts from zero.
* Mixed-script digits trigger `mixed-digit Unicode number literal` while retaining the offending byte offset for continued parsing.
* Character literal issues (`invalid` or `unterminated`) highlight the literal verbatim, even for supplementary-plane code points.

## Regression Coverage

| Scenario                               | C Test              | C++ Test                  |
|----------------------------------------|---------------------|---------------------------|
| Multilingual grammar parsing           | `test_utf8`         | `test++_utf8`             |
| ASCII fallback after UTF-8 grammar     | `test_utf8`         | `test++_utf8`             |
| Repeated grammar creation/free         | `test_utf8_double`  | _n/a_ (covered by C)      |
| UTF-8 validation (malformed input)     | `test_utf8_invalid` | `test++_utf8_invalid`     |

Future contributors should extend this table as additional edge cases are captured.
```}