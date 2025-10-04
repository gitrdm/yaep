# YAEP Unicode Support Guide

This note summarises the production-level guarantees YAEP now offers when parsing UTF-8 grammar descriptions and highlights the remaining responsibilities for client code.

## Overview

* **Encoding.** All public APIs expect UTF-8 encoded byte strings. Input is validated up front; malformed UTF-8 is rejected before the lexer allocates any parser state so clients never observe partial mutations.
* **Identifiers.** Identifier start characters follow the Unicode `Letter` classes (`Lu`, `Ll`, `Lt`, `Lm`, `Lo`) plus underscore. Continuation characters additionally allow decimal digits (`Nd`), combining marks (`Mn`, `Mc`), and connector punctuation (`Pc`) per [Unicode® Standard Annex #31](https://www.unicode.org/reports/tr31/).
* **Character literals.** Character literals (`'…'`) now accept any single Unicode scalar value. The literal retains its original UTF-8 bytes for diagnostics while the parser stores the decoded code point exactly once, preventing stale byte-based hashes.
* **Numeric literals.** Decimal numbers inside grammar descriptions accept any `Nd` code point. Literals must use digits from a single script; mixed-script numerals are rejected with a descriptive error to prevent subtle token-code mismatches.

## Normalisation Policy

YAEP deliberately treats byte sequences verbatim. Identifiers that differ only by Unicode normalisation form (e.g., NFC vs. NFD) are considered distinct symbols. This keeps the parser deterministic and avoids implicit data rewrites, but it means clients must decide how aggressively to normalise content.

**Recommended workflow:**

1. Normalise grammar descriptions to **NFC** (or a project-specific form) before passing them to YAEP.
2. Apply the same normalisation to runtime tokens (terminal attributes) so that hash lookups and equality checks remain stable.
3. If normalisation is undesirable, document the decision in your project so that tooling and reviewers know mixed forms are intentional.

### Sample helper (C API)

```c
#include "utf8proc.h"

static char *
normalize_to_nfc (const char *input, YaepAllocator *alloc)
{
  utf8proc_uint8_t *output = NULL;
  utf8proc_map ((const utf8proc_uint8_t *)input,
                0,
                &output,
                UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_COMPAT);
  return (char *)output; /* caller frees via utf8proc_free */
}
```

Use the helper before invoking `yaep_parse_grammar` or before emitting terminal identifiers.

### Sample helper (C++ API)

```cpp
#include <memory>
#include "utf8proc.h"

std::string normalize_to_nfc(const std::string &text) {
  utf8proc_uint8_t *buffer = nullptr;
  if (utf8proc_map(reinterpret_cast<const utf8proc_uint8_t *>(text.c_str()),
                   0,
                   &buffer,
                   UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_COMPAT) < 0) {
    throw std::runtime_error("utf8proc_map failure");
  }
  std::unique_ptr<utf8proc_uint8_t, decltype(&utf8proc_free)>
    holder(buffer, utf8proc_free);
  return std::string(reinterpret_cast<char *>(holder.get()));
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