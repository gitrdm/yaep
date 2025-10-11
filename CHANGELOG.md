# Changelog

All notable changes to the YAEP project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2025-10-10

### Major Changes

This release represents a comprehensive modernization of the YAEP library while maintaining API compatibility. As a fork of the original project (last updated ~2020), version 2.0.0 signals significant internal improvements and a change in build system.

### Added

- **Documentation:**
  - Markdown API documentation (YAEP_C_API.md, YAEP_CPP_API.md)
  - Doxygen integration for auto-generated code documentation
  - Comprehensive build and documentation modernization guides
  - Updated README with modern project status

- **Build System:**
  - Parallel build support (fixed race conditions)
  - Comprehensive test suite integration (126 automated tests)
  - BUILD_AND_DOC_MODERNIZATION.md documentation

- **Quality Assurance:**
  - Extensive fuzzing under Valgrind with no known issues
  - All memory leaks fixed
  - All segfaults fixed
  - Production-ready stability

### Changed

- **Breaking: Build System Migration**
  - **REMOVED:** Autotools build system (configure, Makefile.in templates)
  - **CMake is now the only supported build system**
  - Users must migrate from `./configure && make` to CMake workflow
  - See README.md for updated build instructions

- **Code Modernization:**
  - Upgraded from C99 to C17 standard
  - Upgraded to C++17 standard
  - Modern compiler optimizations enabled
  - Fixed all compiler warnings under strict mode

- **Documentation Format:**
  - **REMOVED:** SGML documentation pipeline
  - **REMOVED:** Binary documentation (PDF, DVI, PS, INFO files)
  - Replaced with Markdown + Doxygen approach
  - Documentation now maintainable without specialized tools

- **Library Versioning:**
  - Shared library version: 1.0.0 → 2.0.0
  - SOVERSION: 1 → 2
  - Reflects ABI changes from C17 modernization

### Removed

- Autotools build system (15 files)
  - configure, configure.ac
  - install-sh
  - Makefile.in templates
  - SGML template files
  
- Binary documentation formats (13 files)
  - PDF, DVI, PostScript, INFO files
  - Split HTML documentation
  
- Travis CI configuration (.travis.yml)
  - Obsolete CI/CD system (shut down 2021)

- Dino language references from README
  - Project now standalone, not part of Dino ecosystem

### Fixed

- Race condition in parallel builds
- Known memory leaks throughout codebase
- Segmentation faults in edge cases
- Compiler warnings under C17/C++17 strict mode

### Migration Guide

**For users upgrading from 1.x:**

1. **Build System Change (REQUIRED):**
   ```bash
   # Old (no longer supported):
   ./configure
   make
   make install
   
   # New (required):
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build .
   cmake --install . --prefix /usr/local
   ```

2. **Documentation Access:**
   - PDF/PS files removed - use Markdown docs in `doc/` directory
   - Generate Doxygen docs: `doxygen Doxyfile`

3. **API Compatibility:**
   - C API: Fully compatible, no code changes needed
   - C++ API: Fully compatible, no code changes needed
   - ABI: May differ due to C17 changes - **recompile your applications**

4. **Testing:**
   - Run test suite: `./build_and_test.sh` or `cd build && ctest`
   - Verify your application works with new version
   - Report any issues on GitHub

### Notes

- This is a community fork maintained by @gitrdm
- Original author: Vladimir Makarov (credited in documentation)
- MIT License unchanged
- API stability maintained for smooth migration

---

## [1.0.0] - 2015-10-10

Original release by Vladimir Makarov.

- Initial C and C++ API
- Earley parser implementation
- YACC-like grammar syntax
- Error recovery support
- Ambiguous grammar handling
- Cost-based parsing
- Autotools and CMake build systems
- SGML documentation

[2.0.0]: https://github.com/gitrdm/yaep/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/gitrdm/yaep/releases/tag/v1.0.0
