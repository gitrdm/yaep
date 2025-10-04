# UTF-8 Support — Gap Closure (v2)

Date: 2025-10-04

This document summarizes the current state of UTF-8/Unicode support in YAEP, lists the gaps identified during the audit, records low-risk fixes that were applied in this branch, and prescribes concrete remediation steps and verification criteria to bring the codebase to production-grade Unicode readiness.

Purpose
- Provide a compact, actionable plan and status for closing Unicode-related gaps discovered in the codebase.
- Serve as a checklist for reviewers and implementers when hardening Unicode handling.

Scope
- Files and areas examined: `src/unicode/yaep_unicode.{h,c}`, `src/sgramm.y` (grammar lexer), `src/yaep.c` (core), build tooling (CMake and autotools `Makefile.in`), and test fixtures under `test/`.

Summary of what was found
- The project already contains a dedicated Unicode wrapper using `utf8proc` (`src/unicode/yaep_unicode.*`) and the lexer (`src/sgramm.y`) was updated to use the wrapper primitives (`yaep_utf8_next`, `yaep_utf8_isalpha`, etc.).
- CMake builds link and compile the bundled `utf8proc`; CMake-based build and the full test suite were exercised and pass (132/132 tests).
- Autotools `Makefile.in` lacked distribution wiring for `third_party/utf8proc` and `src/unicode` content; that was fixed so `make dist-copy` packages `src/unicode` and `third_party/utf8proc` with preserved layout.

Small, low-risk fixes already applied (v2 branch)
- Replace unsafe formatting in `yaep_error` with `vsnprintf` and safe truncation in `src/yaep.c`.
  - File: `src/yaep.c`
  - Why: prevent buffer overruns and make truncation explicit and UTF-8-conscious in follow-ups.
- Replace `sprintf` calls in the lexer with `snprintf` for safer error diagnostics in `src/sgramm.y`.
  - File: `src/sgramm.y`
- Improve `Makefile.in` distribution copying so subdirectories (like `src/unicode` and `third_party/utf8proc`) are preserved in the generated tarball.
  - File: `Makefile.in`

Verification performed after these edits
- Rebuilt the project using CMake in `build/` and re-ran the full test suite: `ctest --output-on-failure`. Result: 132/132 tests passed.
- Executed `./configure && make dist-copy` to verify autotools packaging tree — `yaep-0.997/` created and includes `src/unicode` and `third_party/utf8proc`.

Detailed gap inventory and closure plan

1) Error message capacity and truncation semantics
   - Problem: The project used a fixed 200-byte `YAEP_MAX_ERROR_MESSAGE_LENGTH`. Prior code used unbounded `vsprintf`/`sprintf` in some places which is unsafe and may truncate or split UTF-8 sequences.
   - Status: Replaced uses of unbounded formatting with `vsnprintf`/`snprintf` (safe truncation) in `src/yaep.c` and `src/sgramm.y`.
   - Risk: Truncated UTF-8 messages may end mid-codepoint.
   - Recommended closure:
     - Increase `YAEP_MAX_ERROR_MESSAGE_LENGTH` to a larger value (1024).
     - Keep `grammar->error_message` as the primary message buffer; when formatting would exceed the static buffer, truncate safely at a UTF-8 codepoint boundary and append an ellipsis (`...`) to indicate truncation. Do not return partial (invalid) UTF-8 in `error_message`.
   - Tests:
     - Unit test that constructs long error messages (multibyte sequences) and verifies the stored message is valid UTF-8 and ends whole.
   - Acceptance: No buffer overflows; error_message contains valid UTF-8 and the full message.

2) Normalization (NFC/NFD) and canonical identifier equivalence
   - Problem: Identifiers are stored and compared as raw byte sequences. Canonically equivalent Unicode strings (composed vs decomposed) are treated as distinct. This may surprise users and break symbol lookups.
   - Status: Not implemented — the wrapper currently provides decoding and classification but not normalization.
   - Recommended closure (explicit policy):
     - Normalization is enabled by default. Normalize to NFC on ingestion for symbol names and literals.
     - Add a wrapper function `yaep_utf8_normalize(const char *in, char **out)` that uses `utf8proc_map` with NFC mapping.
     - Normalize symbol names at the point of insertion into symbol tables (before hashing or equality checks). Normalization is not applied to error messages or diagnostics.
   - Tests:
     - Insert two names that differ only in decomposition form and confirm they map to the same symbol.
   - Acceptance: Symbol insertion and lookups are canonical-equivalence safe.

3) Hashing and sign-extension hardening
   - Problem: Historically some code treated `char` as signed when computing hashes, which causes different hash values for identical byte sequences on different platforms.
   - Status: `yaep_utf8_hash` implements an unsigned-byte hash and `symb_repr_hash` uses it. General audit found the major hashing site covered; other locations were reviewed for obvious signed-char use.
   - Recommended closure:
     - Do a repository-wide audit for any `char` to `int` promotions in hashing/data indexing and ensure they cast to `unsigned char`.
   - Tests:
     - Cross-platform tests aren't available here; instead add unit tests that compute known hashes for multibyte strings and assert stability.
   - Acceptance: Hash results are deterministic and consistent across platforms that compile YAEP.

4) Lexer robustness and backtracking (performance and clarity)
   - Problem: Lexer backtracking currently rewinds by re-parsing from a start pointer to find the byte boundary of previously-read codepoints. It works but is slightly inefficient.
   - Status: Correctness established (tests pass). Performance is acceptable due to ASCII fast-paths.
   - Recommended closure:
     - Expose a `yaep_utf8_next_with_len(const char **p, int *bytes_len)` or variant that returns bytes consumed to allow direct pointer arithmetic and avoid reparsing.
   - Tests:
     - Measure lexer throughput for a large ASCII grammar and an input with many non-ASCII identifiers; ensure no regression.
   - Acceptance: No behavioral change; optional micro-optimization completed if implemented.

5) Decimal digit block detection (mixed-script prevention)
   - Problem: The implementation of `yaep_utf8_digit_value` uses a loop of `utf8proc_get_property(start - 1)` to find the block start. This works because Unicode decimal digits are contiguous (Nd), but code should document the assumption.
   - Status: Implemented and used by lexer to reject mixed-script numeric literals.
   - Recommended closure:
     - Add a comment with the Unicode guarantee. Consider switching to property `digit_value` if `utf8proc` exposes it directly for clarity.
   - Tests:
     - Tests already include mixed-digit literals; expand with more scripts' digits (Arabic-Indic, Devanagari, etc.).
   - Acceptance: Lexer rejects mixed-script numbers and accepts valid single-script numeric sequences.

6) Embedded NUL (U+0000) and EOS usage
   - Problem: `YAEP_CODEPOINT_EOS` is defined as 0 (U+0000) because strings are NUL-terminated C strings. YAEP does not support embedded U+0000 in grammar descriptions or identifiers.
   - Status: Documented and used consistently.
   - Recommended closure:
     - Explicitly document that input strings must be NUL-free and encoded in UTF-8.
   - Tests:
     - Negative test: pass an input with embedded NUL and verify it is handled/rejected clearly.
   - Acceptance: Documentation updated and behaviour unchanged.

7) Test coverage expansion (high priority)
   - Problem: Current tests include several UTF-8 cases but do not exhaustively cover canonical equivalence, non-BMP code points, complex combining sequences, and long-message truncation edge cases.
   - Status: There are UTF-8 focused tests under `test/` and they pass.
   - Recommended closure:
     - Add tests covering:
       - NFC vs NFD canonical equivalence for identifiers.
       - Identifiers containing combining marks (U+0300..), zero-width joiner sequences, and emoji (non-BMP > U+FFFF).
       - Malformed UTF-8 at buffer boundaries and in the middle of tokens.
       - Long error messages containing multi-byte sequences and confirm boundary-safe truncation.
   - Tests should run under both the C and C++ front-ends.
   - Acceptance: New tests added and pass.

Regression and verification checklist (must pass before merge)
- Build: CMake and autotools build should both succeed.
- Tests: All existing tests plus newly added Unicode tests must pass (CTest all green).
- Packaging: `make dist-copy` must include `src/unicode` and `third_party/utf8proc`.
- Documentation: README and `src/yaep.h` must clearly state the UTF-8 requirement and policy for normalization.

Rollout plan and recommended milestones
1. Short-term (this PR / v2 branch)
   - Merge the safe formatting fixes and Makefile.in distribution adjustments (done).
   - Increase `YAEP_MAX_ERROR_MESSAGE_LENGTH` to 1024 and add a UTF-8 safe truncation helper (low risk).
   - Add a small suite of tests for truncation and non-BMP parsing.

2. Medium-term
   - Implement normalization on symbol ingestion (NFC) with `yaep_utf8_normalize` and normalize symbol names before symbol-table insertion and hashing.
   - Add canonical-equivalence tests.
   - Add a `yaep_utf8_next_with_len` helper if profiling shows repetitive reparsing overhead in the lexer.

3. Long-term
   - Consider providing opt-in runtime configuration (e.g., `YAEP_NORMALIZE_IDENTIFIERS`) for backward compatibility during migration.
   - Optionally provide a locale/collation-aware extension or an API for users who want locale-specific identifier semantics.

PR / reviewer checklist
- Ensure C and C++ builds succeed on CI.
- Run `ctest` to confirm test suite is green.
- Review doc changes (`README`, `src/yaep.h`, this closure document).
- Review new tests for correctness and absence of flaky behavior.

Appendix — code pointers (most relevant files)
- Unicode wrapper: `src/unicode/yaep_unicode.h`, `src/unicode/yaep_unicode.c`
- Grammar lexer: `src/sgramm.y`
- Core/diagnostics: `src/yaep.c`
- Hashing: `src/yaep.c` (symbol hash uses `yaep_utf8_hash`)
- Tests: `test/*_utf8*`, `test/C/*`, `test/C++/*`
- Build: `CMakeLists.txt`, `Makefile.in`

Notes
- The main remaining policy decision is normalization. Implementing NFC normalization on ingestion is the most impactful change (behavioural) and should be discussed with consumers of YAEP (if any). If you want I can implement the NFC normalization helper and wire symbol insertion in a follow-up change together with canonical-equivalence tests.

Contact
- If you'd like me to implement any of the recommended fixes (normalization, message resizing, helper to return byte-length with codepoint), tell me which item to pick first and I'll implement it, add tests, and run the full test suite.

End of document
