#!/bin/bash
#
# Build script for YAEP libFuzzer harness
#
# This script builds the fuzzer with AddressSanitizer to detect:
# - Memory leaks (from longjmp bypassing cleanup)
# - Use-after-free bugs
# - Heap buffer overflows
# - NULL pointer dereferences (the previous crash)
#

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
CORPUS_DIR="$SCRIPT_DIR/corpus"

echo "============================================="
echo "Building YAEP LibFuzzer Harness"
echo "============================================="
echo "Project root: $PROJECT_ROOT"
echo "Build dir:    $BUILD_DIR"
echo "Corpus dir:   $CORPUS_DIR"
echo ""

# Create build directory
mkdir -p "$BUILD_DIR"
mkdir -p "$CORPUS_DIR"

# Step 1: Build YAEP library with sanitizers
echo "[1/3] Building YAEP library with ASan..."
cd "$PROJECT_ROOT/build"

# Clean and rebuild with sanitizers
rm -f src/libyaep.a src/yaep_test
make clean 2>/dev/null || true

# Build with ASan and debugging
CC=clang CXX=clang++ \
CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1" \
CXXFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1" \
cmake -DCMAKE_BUILD_TYPE=Debug ..

make yaep_static -j$(nproc)

if [ ! -f src/libyaep.a ]; then
    echo "ERROR: Failed to build libyaep.a"
    exit 1
fi

# Step 2: Build fuzzer
echo "[2/3] Building fuzzer with LibFuzzer + ASan..."
cd "$BUILD_DIR"

clang -fsanitize=fuzzer,address \
      -fno-omit-frame-pointer \
      -g -O1 \
      -I"$PROJECT_ROOT/src" \
      "$SCRIPT_DIR/fuzz_yaep.c" \
      -L"$PROJECT_ROOT/build/src" \
      -lyaep \
      -o fuzz_yaep

if [ ! -f fuzz_yaep ]; then
    echo "ERROR: Failed to build fuzzer"
    exit 1
fi

# Step 3: Create seed corpus
echo "[3/3] Creating seed corpus..."

# Create some initial test cases
cat > "$CORPUS_DIR/simple_grammar.txt" << 'EOF'
%token NUMBER
%%
expr : NUMBER
     ;
EOF

cat > "$CORPUS_DIR/arithmetic.txt" << 'EOF'
%token NUM
%%
expr : expr '+' term
     | term
     ;
term : NUM
     ;
EOF

cat > "$CORPUS_DIR/recursive.txt" << 'EOF'
%token ID
%%
list : list ',' ID
     | ID
     ;
EOF

cat > "$CORPUS_DIR/malformed1.txt" << 'EOF'
%token
%%
EOF

cat > "$CORPUS_DIR/malformed2.txt" << 'EOF'
This is not a valid grammar at all!
Random garbage to test error handling.
EOF

cat > "$CORPUS_DIR/empty.txt" << 'EOF'
EOF

cat > "$CORPUS_DIR/null_bytes.txt" << 'EOF'
%token TEST
%%
rule : TEST
EOF
# Add some null bytes
printf '\0\0\0' >> "$CORPUS_DIR/null_bytes.txt"

echo ""
echo "============================================="
echo "✅ Build complete!"
echo "============================================="
echo ""
echo "Fuzzer:       $BUILD_DIR/fuzz_yaep"
echo "Corpus:       $CORPUS_DIR"
echo "Seed inputs:  $(ls -1 $CORPUS_DIR | wc -l) files"
echo ""
echo "To run fuzzer:"
echo "  cd $SCRIPT_DIR"
echo "  ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1 \\"
echo "    ./build/fuzz_yaep corpus/ -max_len=4096 -timeout=1"
echo ""
echo "To run with crash detection:"
echo "  ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1:abort_on_error=1 \\"
echo "    ./build/fuzz_yaep corpus/ -max_len=4096 -timeout=1 -runs=10000"
echo ""
