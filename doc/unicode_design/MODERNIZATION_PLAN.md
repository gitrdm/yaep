# Modernization Review for YAEP

## Objectives
- Raise the baseline toolchain to a well-supported modern C standard (target: **C11** with optional C99 fallback).
- Improve maintainability and developer experience by tightening warnings, sanitizers, and continuous integration coverage.
- Prepare the code base for Unicode enablement (utf8proc integration) and future feature work without accumulating technical debt.
- Document risks, sequencing, and the scope of non-functional changes so the modernization effort can be tracked and reviewed independently of feature additions.

## Current State Snapshot
- Dual build systems: Autotools (`configure`, `Makefile.in`) and CMake; both default to pre-C99 compilers with minimal warning flags.
- Custom allocation stacks (`objstack`, `vlobject`) and bespoke hash utilities without modern safety checks.
- Sparse automated tests, mostly golden output comparisons in `test/`; no continuous integration pipeline configured.
- Limited compiler warning coverage (no `-Wall -Wextra`), no sanitizers, and no static analysis gating.
- Documentation highlights performance but does not describe toolchain requirements or coding standards.

## Recommended Baseline
- **Language standard**: adopt `-std=c11` (or `/std:c11` for MSVC) as default. Provide a configure-time option `--with-legacy-c99` to compile without newer constructs if a downstream user needs it.
- **Compilers**: support GCC ≥ 5, Clang ≥ 5, MSVC ≥ 2017. Enumerate toolchain versions in `README.md` and CI matrix.
- **Warnings**: enable `-Wall -Wextra -Wpedantic` (GCC/Clang) and `/W4` (MSVC). Treat warnings as errors in CI while allowing opt-out locally.
- **Sanitizers**: add AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) builds to CI for Linux/Clang.
- **CI provider**: GitHub Actions (or equivalent) to run Autotools + Make, CMake + Ninja, and sanitizer jobs on each push/PR.

## Modernization Work Packages

### 1. Build System Updates
- Ensure both Autotools and CMake detect a C11-capable compiler and fall back gracefully to C99 when configured.
- Add optional `-fsanitize` flags via configure/CMake options (`--enable-sanitizers`, `-DYAEP_ENABLE_SANITIZERS=ON`).
- Introduce shared warning flag macros; for Autotools, use `AX_CHECK_COMPILE_FLAG`, for CMake, leverage `target_compile_options` per target.
- Provide a toolchain summary in the build output to aid debugging.

### 2. Codebase Hygiene
- Replace deprecated constructs (e.g., implicit function declarations, K&R style prototypes) with modern equivalents.
- Introduce `static inline` helpers for repeated macros where readability improves without performance loss.
- Add `_Static_assert` guards for assumptions (e.g., sizeof checks, enum ranges).
- Wrap platform-specific sections with feature-test macros to maintain portability.
- Convert manual string copies to `memcpy`/`strncpy` with explicit lengths; audit `sprintf` usage and replace with `snprintf`.

### 3. Tooling & Automation
- Configure `clang-format` and/or `astyle` profile (opt-in) and document usage; defer mass reformatting until after functional changes stabilize.
- Add `clang-tidy` configuration for optional static analysis, focusing on `readability`, `bugprone`, and `security` checks.
- Script `make format` / `make lint` targets that developers can run locally; do not gate CI initially to avoid blocking early progress.

### 4. Testing Enhancements
- Expand the existing `make test` harness to include self-checking unit tests (e.g., using a lightweight framework or custom assertions) for core utilities (`hashtab`, `objstack`).
- Introduce regression tests that exercise Unicode pathways once utf8proc integration lands.
- Record test coverage targets (e.g., `gcov`/`llvm-cov`) for future tracking; treat coverage as informational initially.

### 5. Documentation & Developer Onboarding
- Update `README.md` with new build instructions, required toolchain versions, sanitizer usage, and CI badge.
- Add a `docs/modernization.md` (or reference this plan) outlining expectations for new contributions (coding style, testing matrix).
- Document the utf8proc dependency and how it slots into the build when vendored.

## Sequencing & Coordination
1. **Scoping Phase**: agree on C11 baseline, finalize supported compilers, and set up CI skeleton (without new warnings enforced yet).
2. **Warnings First**: enable warnings, triage/fix the resulting diagnostics in small, reviewable batches.
3. **Sanitizers + Tests**: integrate ASan/UBSan builds, add targeted tests to ensure sanitizer runs stay green.
4. **Feature Integration Prep**: vendor utf8proc and stub the `unicode_utf8proc` wrapper to verify the build remains stable under stricter checks.
5. **Deferred Cleanups**: once Unicode work is merged and stable, revisit larger refactors (allocator simplification, removal of Autotools if desired).

## Risks & Mitigations
- **Toolchain variance**: some downstream environments may lack C11 support. Mitigate with a documented fallback and CI job that compiles under `-std=c99` to keep the pathway tested.
- **Warning deluge**: enabling strict warnings may surface hundreds of diagnostics. Address incrementally, grouping related fixes and keeping commits focused.
- **CI complexity**: dual build systems and sanitizers increase job time. Use build matrix caching, limit sanitizer jobs to nightly schedule if runtime becomes excessive.
- **Churn & merge conflicts**: modernization touches many files. Coordinate with feature branches, avoid automated reformatting until feature work slows.

## Review Checklist
- [ ] C11 compiler detection and fallback implemented in Autotools and CMake.
- [ ] Warning flags enabled and codebase builds cleanly under GCC, Clang, MSVC.
- [ ] CI workflow running: standard builds + sanitizers; documented status badge.
- [ ] Sanitizer runs pass; key modules audited for undefined behavior.
- [ ] utf8proc vendored and wrapper stub compiles (even if feature work pending).
- [ ] Documentation updated with new requirements and developer guidance.

## Follow-up Opportunities
- Consider deprecating the Autotools path once CMake parity is proven.
- Explore migrating ad-hoc data structures to standard C containers (e.g., use `uthash`/`khash` if it simplifies maintenance).
- Investigate optional C++17 facade improvements in `yaep.cpp`, leveraging `std::string_view` and RAII wrappers.
- Add performance benchmarks into CI (using existing `src/compare_parsers`) to detect regressions introduced by modern safety features.
