#!/usr/bin/env python3
"""
corpus_stats.py: Analyze YAEP fuzzing corpus for input diversity.

Features:
- File count, min/max/mean/median size
- Unique character and token statistics
- Histogram of file sizes
- Unicode vs ASCII ratio
- (Optional) Nesting depth if EBNF-like structure is detected

Usage:
  python3 corpus_stats.py /path/to/corpus/
"""
import sys
import os
import statistics
from collections import Counter, defaultdict

def is_ascii(s):
    try:
        s.encode('ascii')
        return True
    except UnicodeEncodeError:
        return False

def analyze_file(path):
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        data = f.read()
    chars = set(data)
    ascii_only = is_ascii(data)
    size = len(data)
    # Tokenize on whitespace and EBNF symbols
    tokens = [t for t in data.replace(';',' ; ').replace('=',' = ').replace('|',' | ').replace('::=',' ::= ').split() if t]
    return {
        'size': size,
        'chars': chars,
        'ascii_only': ascii_only,
        'tokens': set(tokens),
    }

def main(corpus_dir):
    files = [os.path.join(corpus_dir, f) for f in os.listdir(corpus_dir) if os.path.isfile(os.path.join(corpus_dir, f))]
    sizes = []
    ascii_count = 0
    unicode_count = 0
    all_chars = set()
    all_tokens = set()
    size_hist = defaultdict(int)
    for path in files:
        info = analyze_file(path)
        sizes.append(info['size'])
        all_chars.update(info['chars'])
        all_tokens.update(info['tokens'])
        size_hist[min(10*(info['size']//10), 100)] += 1
        if info['ascii_only']:
            ascii_count += 1
        else:
            unicode_count += 1
    print(f"Corpus directory: {corpus_dir}")
    print(f"File count: {len(files)}")
    if not sizes:
        print("No files found.")
        return
    print(f"File size: min={min(sizes)}, max={max(sizes)}, mean={statistics.mean(sizes):.1f}, median={statistics.median(sizes)}")
    print(f"Unique characters: {len(all_chars)}")
    print(f"Unique tokens: {len(all_tokens)}")
    print(f"ASCII-only files: {ascii_count}")
    print(f"Unicode files: {unicode_count}")
    print("File size histogram (bucketed by 10 bytes, capped at 100):")
    for bucket in range(0, 110, 10):
        print(f"  {bucket:3d}-{bucket+9:3d}: {size_hist.get(bucket,0)}")
    print("Sample unique tokens:", ', '.join(list(all_tokens)[:20]))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 corpus_stats.py /path/to/corpus/")
        sys.exit(1)
    main(sys.argv[1])
