# YAEP Fuzzing Infrastructure

This directory contains LibFuzzer-based fuzzing harnesses to verify that the C17 modernization successfully eliminated crashes and memory leaks.

## Background

The C17 modernization was motivated by crashes and memory leaks discovered through fuzzing:

1. **NULL pointer dereferences** - Tokens with NULL `symb` pointers added before longjmp
2. **Memory leaks** - Resources allocated between setjmp() and longjmp() not freed
3. **Thread safety** - Global `jmp_buf` causing race conditions

The modernization replaced all setjmp/longjmp with explicit error returns and thread-local error context.

## Structure

```
fuzz/
├── README.md                    # This file
├── fuzz_yaep.c                 # LibFuzzer harness
├── build_libfuzzer.sh          # Build script
├── run_fuzz_campaign.sh        # Automated testing
├── corpus/                     # Seed inputs (generated)
├── build/                      # Fuzzer binary (generated)
└── results/                    # Test results (generated)
```

## Quick Start

### Build and Run

```bash
cd fuzz
./run_fuzz_campaign.sh
```

This will:
1. Build YAEP with AddressSanitizer
2. Build the fuzzer
3. Run multiple fuzzing campaigns
4. Report any crashes or leaks

### Manual Testing

```bash
# Build
./build_libfuzzer.sh

# Run for 60 seconds
ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1 \
  ./build/fuzz_yaep corpus/ -max_total_time=60 -max_len=4096

# Run specific number of iterations
ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1 \
  ./build/fuzz_yaep corpus/ -runs=10000 -max_len=4096
```

## What's Being Tested

The fuzzer targets code paths that previously crashed:

1. **Grammar Parsing** (`yaep_parse_grammar`)
   - Bison/yyparse error handling
   - Grammar validation errors
   - Malformed input handling

2. **Token Processing** (`yaep_parse`)
   - Token validation (NULL pointer bug location)
   - Token array management
   - Parse state cleanup

3. **Error Handling**
   - Thread-local error context
   - Error propagation
   - Resource cleanup on errors

4. **Memory Management**
   - Allocation failures
   - Cleanup paths
   - Leak detection

## Sanitizer Options

### AddressSanitizer (ASan)

Detects:
- Heap/stack/global buffer overflows
- Use-after-free
- Double-free
- Memory leaks

Options used:
- `detect_leaks=1` - Enable leak detection
- `allocator_may_return_null=1` - Allow allocation failures (for testing)
- `abort_on_error=1` - Stop immediately on first error (optional)

### LibFuzzer Options

Common options:
- `-max_len=4096` - Maximum input size
- `-timeout=1` - Timeout per test case (seconds)
- `-runs=N` - Number of iterations
- `-max_total_time=N` - Total run time (seconds)
- `-dict=file` - Use mutation dictionary (optional)

## Interpreting Results

### Success (Expected with C17 code)

```
#1000: cov: 245 ft: 512 corp: 25 exec/s: 100
✅ No crashes or leaks detected
```

### Crash (Should NOT happen)

```
==12345==ERROR: AddressSanitizer: SEGV on unknown address
    #0 0x... in build_pl yaep.c:4659
    
⚠️ CRASH DETECTED - Check crash-* files
```

### Memory Leak (Should NOT happen)

```
==12345==ERROR: LeakSanitizer: detected memory leaks
    #0 0x... in malloc
    #1 0x... in yaep_create_grammar
    
⚠️ LEAK DETECTED - Check leak-* files
```

## Corpus Management

The corpus grows over time as the fuzzer discovers interesting inputs:

```bash
# Check corpus size
ls -lh corpus/ | wc -l

# Minimize corpus (remove redundant inputs)
./build/fuzz_yaep -merge=1 corpus_minimized/ corpus/

# Review interesting cases
cat corpus/some-case
```

## Crash Reproduction

If a crash is found:

```bash
# Crash file will be in fuzz/ directory
ls -lh crash-*

# Reproduce the crash
ASAN_OPTIONS=detect_leaks=1:symbolize=1 \
  ./build/fuzz_yaep crash-<hash> -runs=1

# Debug with GDB
gdb --args ./build/fuzz_yaep crash-<hash> -runs=1
```

## Continuous Testing

For CI/CD integration:

```bash
# Exit with error if crashes found
./run_fuzz_campaign.sh || exit 1

# Time-limited run
timeout 300 ./build/fuzz_yaep corpus/ -max_len=4096
```

## Comparison with Pre-C17 Code

To test the old code (if you have a pre-C17 branch):

```bash
# Switch to old branch
git checkout old-branch

# Run fuzzer
./fuzz/run_fuzz_campaign.sh

# Expected: crashes and leaks in old code
# Actual (C17): no crashes or leaks ✅
```

## Advanced Usage

### Custom Dictionaries

Create `fuzz/yaep.dict` with grammar tokens:

```
kw1="%%"
kw2="%token"
kw3="%left"
kw4="%right"
sym1=":"
sym2="|"
sym3=";"
```

Run with dictionary:

```bash
./build/fuzz_yaep corpus/ -dict=yaep.dict -max_len=4096
```

### Parallel Fuzzing

Run multiple fuzzer instances:

```bash
# Instance 1 (master)
./build/fuzz_yaep corpus/ -jobs=4 -workers=4

# Instances 2-4 will sync automatically
```

### Coverage Analysis

Build with coverage:

```bash
clang -fsanitize=fuzzer -fprofile-instr-generate -fcoverage-mapping \
      fuzz_yaep.c -I../src -L../build/src -lyaep -o fuzz_yaep_cov

# Run and generate report
LLVM_PROFILE_FILE="yaep.profraw" ./fuzz_yaep_cov corpus/ -runs=10000
llvm-profdata merge -sparse yaep.profraw -o yaep.profdata
llvm-cov show fuzz_yaep_cov -instr-profile=yaep.profdata
```

## Troubleshooting

### "Cannot find -lyaep"

Rebuild YAEP library:

```bash
cd ../build
make clean && make yaep_static
```

### "Fuzzer runs too slow"

- Reduce `-max_len` (try 1024)
- Increase `-timeout` (try 5)
- Use `-jobs=1 -workers=1`

### "Out of memory"

- Reduce corpus size
- Use `-rss_limit_mb=2048`
- Minimize corpus periodically

## References

- [LibFuzzer Documentation](https://llvm.org/docs/LibFuzzer.html)
- [AddressSanitizer Documentation](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- YAEP C17 Modernization: `../LLM_Modern/LONGJMP_ELIMINATION_COMPLETE.md`

## Results Archive

Fuzzing results are saved in `results/` directory with timestamps:
- `fuzz_results_YYYYMMDD_HHMMSS.txt` - Detailed output
- Check these files for historical comparison

---

**Last Updated:** October 7, 2024  
**Status:** Ready for production fuzzing
