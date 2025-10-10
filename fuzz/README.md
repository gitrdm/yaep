# YAEP Fuzzing Infrastructure

This directory contains all scripts, harnesses, and tools for fuzzing the YAEP parser using LibFuzzer and AddressSanitizer (ASan). The workflow is designed to ensure the C17 modernization is robust against crashes, memory leaks, and error-handling bugs.

---

## Directory Structure

```
fuzz/
├── README.md                # This file
├── build_libfuzzer.sh       # Build YAEP and fuzzer harness with ASan
├── run_fuzz_campaign.sh     # Run a full fuzzing campaign (build, test, report)
├── auto_expand_corpus.sh    # Automate corpus expansion, mutation, minimization
├── expand_corpus.py         # Generate new seed corpus files from project sources
├── corpus_stats.py          # Analyze corpus diversity and statistics
├── fuzz_yaep.c              # LibFuzzer harness for YAEP
├── corpus/                  # Main seed corpus (inputs for fuzzing)
├── expanded_corpus/         # Temporary expanded corpus (auto_expand_corpus.sh)
├── minimized_corpus/        # Final deduplicated, coverage-increasing corpus
├── build/                   # Fuzzer build output
└── results/                 # Fuzzing results and logs
```

---

## Fuzzing Workflow

### 1. Build the Fuzzer

```bash
cd fuzz
./build_libfuzzer.sh
```

- Builds YAEP and the LibFuzzer harness with AddressSanitizer.
- Output: `build/fuzz_yaep`

---

### 2. Generate or Expand the Corpus

- **To generate new seeds from project files:**
  ```bash
  python3 expand_corpus.py 1000
  ```
  (Creates up to 1000 new files in `corpus/`.)

- **To analyze corpus diversity:**
  ```bash
  python3 corpus_stats.py corpus/
  ```

- **To automatically expand, mutate, and minimize the corpus:**
  ```bash
  ./auto_expand_corpus.sh ./build/fuzz_yaep minimized_corpus/ expanded_corpus/ 5 60 100
  ```
  - Arguments: `[fuzz_binary] [seed_corpus_dir] [output_corpus_dir] [cycles] [fuzz_time_sec] [mutate_count]`
  - After all cycles, the script:
    - Minimizes the expanded corpus into `minimized_corpus/`
    - Deduplicates by file content hash
    - Cleans up `expanded_corpus/` to prevent unbounded growth

  - For help:
    ```bash
    ./auto_expand_corpus.sh --help
    ```

---

### 3. Run a Fuzzing Campaign

```bash
./run_fuzz_campaign.sh
```
- Builds, runs smoke/stability/leak tests, and saves results in `results/`.

---

### 4. Manual Fuzzing

```bash
ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1 \
  ./build/fuzz_yaep minimized_corpus/ -max_total_time=60 -max_len=4096
```

---

### 5. Corpus Minimization (Manual)

To deduplicate and keep only coverage-increasing files:
```bash
ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1 \
  ./build/fuzz_yaep -merge=1 minimized_corpus/ corpus/
```

---

## Crash Reproduction

If a crash file is found:
```bash
ASAN_OPTIONS=detect_leaks=1:symbolize=1 \
  ./build/fuzz_yaep crash-<hash> -runs=1
```

---

## Advanced Tools

- **expand_corpus.py**: Generates new seed files from project sources and random mutations.
- **corpus_stats.py**: Prints statistics and diversity metrics for a given corpus directory.
- **auto_expand_corpus.sh**: Automates cycles of fuzzing, mutation, minimization, deduplication, and cleanup.

---

## Best Practices

- Periodically run `auto_expand_corpus.sh` to keep your corpus fresh and compact.
- Use `corpus_stats.py` to monitor input diversity.
- Always minimize and deduplicate your corpus before long fuzzing runs.
- Archive results from `results/` for regression and progress tracking.

---

**Last Updated:** October 10, 2025
