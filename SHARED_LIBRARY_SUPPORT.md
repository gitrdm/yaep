# Shared Library Support Added to YAEP

## Summary
Both CMake and Autotools build systems now build **shared libraries** (`.so`) in addition to static libraries (`.a`).

## Libraries Built

### C Libraries
- **Static**: `libyaep.a`
- **Shared**: `libyaep.so` (with versioned symlinks)
  - `libyaep.so` → `libyaep.so.1` → `libyaep.so.1.0.0`

### C++ Libraries  
- **Static**: `libyaep++.a`
- **Shared**: `libyaep++.so` (with versioned symlinks)
  - `libyaep++.so` → `libyaep++.so.1` → `libyaep++.so.1.0.0`

## Version Information
- **SOVERSION**: 1
- **VERSION**: 1.0.0

## Changes Made

### CMake Build (`src/CMakeLists.txt`)
1. Added `yaep_shared` target for C shared library
2. Added `yaep++_shared` target for C++ shared library  
3. Set VERSION and SOVERSION properties for proper library versioning
4. Updated install target to include shared libraries

### Autotools Build
1. **`configure.ac`**: Added `-fPIC` flag setup for GCC
   ```autoconf
   if test "$GCC" = yes; then
     GCC_FPIC="-fPIC"
   fi
   ```

2. **`Makefile.in`**: 
   - Added shared library variables (`Csharedlibrary`, `Cppsharedlibrary`)
   - Added version variables (`SOVERSION=1`, `VERSION=1.0.0`)
   - Added build rules for shared libraries using `-shared` linker flag
   - Added `-fPIC` to both CFLAGS and CXXFLAGS
   - Created versioned symlinks (`.so` → `.so.1` → `.so.1.0.0`)
   - Updated `install` target to install shared libraries
   - Updated `uninstall` and `clean` targets to handle shared libraries

## Testing

### CMake
```bash
cd build
cmake .. && make
ls -lh src/*.so* src/*.a
```

### Autotools
```bash
autoconf  # if configure.ac was modified
./configure
make
ls -lh src/*.so* src/*.a
```

## Installation
Both build systems install shared libraries to `$(libdir)` with proper symlinks:
- `libyaep.so.1.0.0` (actual library file)
- `libyaep.so.1` (SOVERSION symlink)
- `libyaep.so` (development symlink)

The same structure applies to `libyaep++.so`.
