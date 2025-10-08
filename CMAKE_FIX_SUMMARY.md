# CMake Build Reliability Fix

## Problem
The CMake build was **unreliable** - sometimes succeeding and sometimes failing, especially in parallel builds with `-j` flag. This was caused by a **race condition** in the build dependencies.

## Root Cause
The file `src/yaep.c` (and `src/yaep.cpp`) directly includes the Bison-generated file `sgramm.c`:

```c
#ifdef CMAKE_BINARY_DIR
#  include "sgramm.c"
#else
#  include "../build/src/sgramm.c"
#endif
```

The original CMakeLists.txt used `add_dependencies()` to create a target-level dependency, but this only ensures the custom target runs before the library target starts building, not that `sgramm.c` is generated before the individual source file `yaep.c` is compiled.

## Solution
Use `set_source_files_properties()` with `OBJECT_DEPENDS` to create an explicit file-level dependency that prevents race conditions in parallel builds.

## Testing
- 10 consecutive fresh builds with -j16 parallelism: 10/10 succeeded
- Multiple builds with -j8: All succeeded
- Comparison with Autotools build: Both now reliable

