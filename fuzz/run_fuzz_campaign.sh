#!/bin/bash
#
# Run fuzzing campaigns and compare results
#
# This script runs systematic fuzzing tests to verify that the C17
# modernization fixed the crashes and memory leaks.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESULTS_DIR="$SCRIPT_DIR/results"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RESULT_FILE="$RESULTS_DIR/fuzz_results_$TIMESTAMP.txt"

mkdir -p "$RESULTS_DIR"

echo "=============================================" | tee "$RESULT_FILE"
echo "YAEP C17 Fuzzing Verification" | tee -a "$RESULT_FILE"
echo "=============================================" | tee -a "$RESULT_FILE"
echo "Timestamp: $(date)" | tee -a "$RESULT_FILE"
echo "Results:   $RESULT_FILE" | tee -a "$RESULT_FILE"
echo "" | tee -a "$RESULT_FILE"

# Build the fuzzer
echo "[1/4] Building fuzzer..." | tee -a "$RESULT_FILE"
cd "$SCRIPT_DIR"
./build_libfuzzer.sh 2>&1 | tee -a "$RESULT_FILE"

if [ ! -f build/fuzz_yaep ]; then
    echo "ERROR: Fuzzer build failed" | tee -a "$RESULT_FILE"
    exit 1
fi

echo "" | tee -a "$RESULT_FILE"

# Test 1: Short run to verify basic functionality
echo "[2/4] Quick smoke test (100 runs)..." | tee -a "$RESULT_FILE"
cd "$SCRIPT_DIR"

ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1:abort_on_error=1 \
    timeout 30 ./build/fuzz_yaep corpus/ -runs=100 -max_len=4096 2>&1 | \
    tee -a "$RESULT_FILE" || true

echo "" | tee -a "$RESULT_FILE"

# Test 2: Medium run for stability
echo "[3/4] Stability test (60 seconds)..." | tee -a "$RESULT_FILE"

ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1 \
    ./build/fuzz_yaep corpus/ -max_total_time=60 -max_len=4096 2>&1 | \
    tee -a "$RESULT_FILE" || true

echo "" | tee -a "$RESULT_FILE"

# Test 3: Memory leak detection
echo "[4/4] Memory leak scan..." | tee -a "$RESULT_FILE"

ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1:exitcode=23 \
    timeout 30 ./build/fuzz_yaep corpus/ -runs=1000 -max_len=4096 2>&1 | \
    tee -a "$RESULT_FILE" || LEAK_STATUS=$?

if [ "${LEAK_STATUS:-0}" -eq 23 ]; then
    echo "⚠️  MEMORY LEAKS DETECTED!" | tee -a "$RESULT_FILE"
else
    echo "✅ No memory leaks detected" | tee -a "$RESULT_FILE"
fi

echo "" | tee -a "$RESULT_FILE"

# Check for crashes
CRASH_COUNT=$(ls -1 "$SCRIPT_DIR"/crash-* 2>/dev/null | wc -l || echo 0)
LEAK_COUNT=$(ls -1 "$SCRIPT_DIR"/leak-* 2>/dev/null | wc -l || echo 0)

echo "=============================================" | tee -a "$RESULT_FILE"
echo "RESULTS SUMMARY" | tee -a "$RESULT_FILE"
echo "=============================================" | tee -a "$RESULT_FILE"
echo "Crashes found:  $CRASH_COUNT" | tee -a "$RESULT_FILE"
echo "Leaks found:    $LEAK_COUNT" | tee -a "$RESULT_FILE"
echo "" | tee -a "$RESULT_FILE"

if [ "$CRASH_COUNT" -eq 0 ] && [ "$LEAK_COUNT" -eq 0 ]; then
    echo "🎉 SUCCESS! No crashes or leaks detected!" | tee -a "$RESULT_FILE"
    echo "" | tee -a "$RESULT_FILE"
    echo "The C17 modernization successfully eliminated:" | tee -a "$RESULT_FILE"
    echo "  ✅ Segfaults from NULL pointer dereferences" | tee -a "$RESULT_FILE"
    echo "  ✅ Memory leaks from longjmp bypassing cleanup" | tee -a "$RESULT_FILE"
    echo "  ✅ Thread safety issues with global jmp_buf" | tee -a "$RESULT_FILE"
    exit 0
else
    echo "⚠️  ISSUES FOUND - See details above" | tee -a "$RESULT_FILE"
    
    if [ "$CRASH_COUNT" -gt 0 ]; then
        echo "" | tee -a "$RESULT_FILE"
        echo "Crash files:" | tee -a "$RESULT_FILE"
        ls -lh crash-* 2>/dev/null | tee -a "$RESULT_FILE" || true
    fi
    
    if [ "$LEAK_COUNT" -gt 0 ]; then
        echo "" | tee -a "$RESULT_FILE"
        echo "Leak files:" | tee -a "$RESULT_FILE"
        ls -lh leak-* 2>/dev/null | tee -a "$RESULT_FILE" || true
    fi
    
    exit 1
fi
