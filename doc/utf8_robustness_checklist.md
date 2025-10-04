# YAEP UTF-8 Robustness Checklist

> **Purpose.** This checklist drives the remaining engineering work needed to deliver production-grade UTF-8 grammar handling in YAEP. It is deliberately self-contained so it can survive context window shifts when used by automated agents. Every item couples background rationale, concrete exit criteria, and test coverage expectations. Treat each checklist entry as *blocking*—check it off only when every acceptance condition is demonstrably met.

## Context Snapshot (2025-10-04)

- Branch: `unicode-feature` (baseline commit includes utf8proc integration, Unicode-aware lexer, and UTF-8 regression tests).
- Current capabilities:
  - Grammar identifiers, comments, and whitespace are tokenised with full Unicode awareness.
  - Parser tables and hash tables operate on byte-precise UTF-8 strings.
  - Regression suites `yaep-test-utf8` (C) and `yaep++-test-utf8` (C++) pass, plus smoke tests for repeated grammar loads.
- Verified gaps (must be closed before feature freeze):
  1. Multi-byte character literals still collapse to a single raw byte token code.
  2. Identifier continuation rejects combining marks (e.g., `a01`).
  3. Numeric literals remain ASCII-only despite Unicode digit predicate support.
  4. Grammar text is not pre-validated; lexer surfaces errors late and with limited diagnostics.
  5. Normalisation expectations (NFC vs. NFD) are undocumented, risking user confusion.

## Checklist

- [x] **Character Literal Unicode Fidelity**
  - *Background.* `struct sterm` currently derives terminal codes via `term.repr[1]`, so `'π'` produces the byte value of the first UTF-8 continuation byte. Downstream parser tables use this incorrect code, causing token mismatches.
  - *Deliverables.*
    - ✅ Lexer now copies the original UTF-8 bytes and appends the decoded scalar for downstream retrieval without `U+XXXX` placeholders.
    - ✅ `seq CHAR` reads the stored scalar and feeds it into terminal codes, eliminating byte-based truncation.
    - ✅ `test_utf8` / `test++_utf8` include a `'π'` production and parse tokens containing that character literal.
    - ✅ Full `ctest` suite run after change.
  - *Exit Criteria.* Code points above `0x7F` should round-trip through grammar parsing, token emission, and parse table lookup without loss; tests must document observed behaviour via assertions or log output.

- [x] **Identifier Continuation Supports Combining Marks**
  - *Background.* `yaep_utf8_isalnum` permits only `Lu/Ll/Lt/Lm/Lo` and `Nd`. Identifiers that rely on combining marks (`Mn`, `Mc`) or connector punctuation (`Pc`) terminate prematurely, hindering languages that depend on decomposed forms.
  - *Deliverables.*
  - ✅ `yaep_utf8_isalnum` now recognises `Mn`, `Mc`, and `Pc` categories with an inline UAX #31 reference.
  - ✅ UTF-8 regression grammars define and exercise `x̸_var`, a decomposed identifier using a combining overlay.
  - ✅ Lexer start-state unchanged; combining marks cannot start identifiers.
  - *Exit Criteria.* Identifiers containing combining marks parse successfully, and the lexer maintains strictness for leading code points; new tests remain stable across multiple runs.

- [x] **Unicode Digit Parsing**
  - *Background.* The lexer recognizes only ASCII digits (`'0'-'9'`) despite already invoking `yaep_utf8_isdigit`. This prevents grammars using non-Latin numerals (e.g., Arabic-Indic) from assigning terminal codes with readability.
  - *Deliverables.*
  - ✅ Lexer consumes any `Nd` digit using the new `yaep_utf8_digit_value` helper and rejects mixed-script literals with a targeted diagnostic.
  - ✅ Grammar assigns `x̸_var=١٢`, exercising Arabic-Indic digits end-to-end.
  - ✅ Unicode support guide documents accepted numeric forms.
  - *Exit Criteria.* Non-ASCII decimal digits are accepted, produce correct numeric values, and are covered by deterministic tests.

- [x] **Proactive UTF-8 Validation Pipeline**
  - *Background.* `yaep_utf8_validate` exists but isn’t invoked. Malformed UTF-8 therefore triggers lexer errors mid-parse, sometimes after partial state mutation.
  - *Deliverables.*
  - ✅ `yaep_parse_grammar` rejects malformed UTF-8 pre-lexing with byte-precise error messages sourced from utf8proc.
  - ✅ Validator uses `utf8proc_iterate` in a single pass, reporting offsets without rescanning.
  - ✅ `test_utf8_invalid` / `test++_utf8_invalid` assert the failure path.
  - ✅ Documentation references new behaviour in `doc/unicode_support.md`.
  - *Exit Criteria.* Invalid UTF-8 is rejected before any lexer state mutates; regression tests confirm stable diagnostics and memory hygiene.

- [x] **Unicode Normalisation Guidance**
  - *Background.* YAEP currently treats identifier byte sequences verbatim; NFC and NFD equivalents register as distinct symbols. Users must understand this to avoid silent duplicate symbol issues.
  - *Deliverables.*
  - ✅ `doc/unicode_support.md` captures the policy plus C/C++ helper snippets.
  - ✅ README now links to the Unicode notes.
  - ✅ Changelog entry (see below) onboards future readers.
  - *Exit Criteria.* Documentation is merged, cross-linked, and future contributors have clear marching orders concerning normalisation responsibilities.

## Execution Notes for Automated Agents

1. **Work serially.** Each checklist item touches shared lexer code; merge conflicts are less painful if tasks are completed and merged one at a time.
2. **Maintain test parity.** Whenever the C test suite gains coverage, mirror it in the C++ harness to avoid API drift.
3. **Profiling consideration.** After widening identifier/digit support, re-run performance benchmarks on large ASCII grammars to confirm ASCII fast paths remain hot (update this document with findings).
4. **Defensive logging.** Keep existing diagnostics user-friendly; avoid exposing internal pointers or allocator addresses in new error messages.
5. **Changelog discipline.** Every completed item should append a dated entry under `src/ChangeLog` summarising behaviour changes and new tests.

---
*Prepared for the UTF-8 sustainment effort, October 2025.*
