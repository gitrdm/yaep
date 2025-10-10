#!/bin/bash
# auto_expand_corpus.sh: Automate fuzzing, mutation, and corpus expansion for YAEP
# Usage: ./auto_expand_corpus.sh [fuzz_binary] [seed_corpus_dir] [output_corpus_dir] [cycles] [fuzz_time_sec] [mutate_count]
# Example: ./auto_expand_corpus.sh ./build/fuzz_yaep minimized_corpus/ expanded_corpus/ 5 60 100

set -e

if [[ "$1" == "--help" ]]; then
    echo "Usage: $0 [fuzz_binary] [seed_corpus_dir] [output_corpus_dir] [cycles] [fuzz_time_sec] [mutate_count]"
    echo "  fuzz_binary:      Path to the fuzzing binary (default: ./build/fuzz_yaep)"
    echo "  seed_corpus_dir:  Directory with initial corpus (default: minimized_corpus/)"
    echo "  output_corpus_dir:Directory for expanded corpus (default: expanded_corpus/)"
    echo "  cycles:           Number of fuzz/mutate/minimize cycles (default: 5)"
    echo "  fuzz_time_sec:    Fuzzing time per cycle in seconds (default: 60)"
    echo "  mutate_count:     Number of mutations per cycle (default: 100)"
    echo "Example: $0 ./build/fuzz_yaep minimized_corpus/ expanded_corpus/ 5 60 100"
    exit 0
fi

FUZZ_BIN=${1:-./build/fuzz_yaep}
SEED_CORPUS=${2:-minimized_corpus/}
OUT_CORPUS=${3:-expanded_corpus/}
CYCLES=${4:-5}
TIME_PER_CYCLE=${5:-60}
MUTATE_COUNT=${6:-100}

mkdir -p "$OUT_CORPUS"
cp -r "$SEED_CORPUS"/* "$OUT_CORPUS" 2>/dev/null || true

mutate_file() {
    # Simple mutation: randomly flip, insert, or delete bytes
    local infile="$1"
    local outfile="$2"
    local size=$(stat -c%s "$infile")
    local muttype=$((RANDOM % 3))
    case $muttype in
        0) # Flip a random byte
            local pos=$((RANDOM % size))
            dd if="$infile" bs=1 count=$pos 2>/dev/null | cat > "$outfile"
            dd if="$infile" bs=1 skip=$pos count=1 2>/dev/null | tr '\000-\377' '\377\376\375\374\373\372\371\370\367\366\365\364\363\362\361\360\357\356\355\354\353\352\351\350\347\346\345\344\343\342\341\340\337\336\335\334\333\332\331\330\327\326\325\324\323\322\321\320\317\316\315\314\313\312\311\310\307\306\305\304\303\302\301\300\277\276\275\274\273\272\271\270\267\266\265\264\263\262\261\260\257\256\255\254\253\252\251\250\247\246\245\244\243\242\241\240\237\236\235\234\233\232\231\230\227\226\225\224\223\222\221\220\217\216\215\214\213\212\211\210\207\206\205\204\203\202\201\200\177\176\175\174\173\172\171\170\167\166\165\164\163\162\161\160\157\156\155\154\153\152\151\150\147\146\145\144\143\142\141\140\137\136\135\134\133\132\131\130\127\126\125\124\123\122\121\120\117\116\115\114\113\112\111\110\107\106\105\104\103\102\101\100\077\076\075\074\073\072\071\070\067\066\065\064\063\062\061\060\057\056\055\054\053\052\051\050\047\046\045\044\043\042\041\040\037\036\035\034\033\032\031\030\027\026\025\024\023\022\021\020\017\016\015\014\013\012\011\010\007\006\005\004\003\002\001\000' >> "$outfile"
            dd if="$infile" bs=1 skip=$((pos+1)) 2>/dev/null >> "$outfile"
            ;;
        1) # Insert a random byte
            local pos=$((RANDOM % (size+1)))
            dd if="$infile" bs=1 count=$pos 2>/dev/null | cat > "$outfile"
            printf "\\$(printf '%03o' $((RANDOM % 256)))" >> "$outfile"
            dd if="$infile" bs=1 skip=$pos 2>/dev/null >> "$outfile"
            ;;
        2) # Delete a random byte
            if [ "$size" -gt 1 ]; then
                local pos=$((RANDOM % size))
                dd if="$infile" bs=1 count=$pos 2>/dev/null | cat > "$outfile"
                dd if="$infile" bs=1 skip=$((pos+1)) 2>/dev/null >> "$outfile"
            else
                cp "$infile" "$outfile"
            fi
            ;;
    esac
}

echo "Starting automated corpus expansion with mutation..."
for ((i=1; i<=CYCLES; i++)); do
    echo "--- Cycle $i/$CYCLES ---"
    # Fuzz and let new coverage-increasing files be added
    ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1 "$FUZZ_BIN" "$OUT_CORPUS" -max_total_time=$TIME_PER_CYCLE -max_len=4096 -timeout=1
    # Mutate random files in the corpus
    for ((m=1; m<=MUTATE_COUNT; m++)); do
        files=("$OUT_CORPUS"/*)
        fcount=${#files[@]}
        if [ "$fcount" -eq 0 ]; then break; fi
        srcfile="${files[$((RANDOM % fcount))]}"
        mutfile="$OUT_CORPUS/mut_$(date +%s%N)_$m"
        mutate_file "$srcfile" "$mutfile"
    done
    # Minimize corpus to keep only coverage-increasing files
    "$FUZZ_BIN" -merge=1 "$OUT_CORPUS" "$OUT_CORPUS"
    echo "Cycle $i complete. Corpus size: $(ls \"$OUT_CORPUS\" | wc -l) files."
done

echo "Automated corpus expansion with mutation complete. Final corpus in $OUT_CORPUS"

# Final minimization: reduce expanded corpus to coverage-increasing files in minimized_corpus
MINIMIZED_DIR="minimized_corpus"
echo "Final minimization: reducing expanded corpus to coverage-increasing files in $MINIMIZED_DIR"
mkdir -p "$MINIMIZED_DIR"
"$FUZZ_BIN" -merge=1 "$MINIMIZED_DIR" "$OUT_CORPUS"
# Remove duplicate files by content hash
cd "$MINIMIZED_DIR"
declare -A hash_seen
for f in *; do
    [ -f "$f" ] || continue
    hash=$(sha1sum "$f" | awk '{print $1}')
    if [[ -n "${hash_seen[$hash]}" ]]; then
        rm "$f"
    else
        hash_seen[$hash]=1
    fi
done
cd - >/dev/null

echo "Minimization complete. Final minimized corpus in $MINIMIZED_DIR (deduplicated by content)"

# Clean up expanded corpus directory
rm -f "$OUT_CORPUS"/*
echo "Expanded corpus directory $OUT_CORPUS has been cleaned up."