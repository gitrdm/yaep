#!/usr/bin/env python3
import os
import random
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CORPUS = ROOT / 'fuzz' / 'corpus'

# Files to sample from
SAMPLE_FILES = [
    'README.md',
    'YAEP_GRAMMAR.md',
    'yaep.txt',
    'doc/yaep.txt',
]

# Fallback: gather .c, .h, .y, .txt, .md files
EXTS = ('.c', '.h', '.y', '.txt', '.md', '.sgml', '.in')

random.seed(0xC0FFEE)

def gather_text():
    texts = []
    for f in SAMPLE_FILES:
        p = ROOT / f
        if p.exists():
            texts.append(p.read_text(errors='ignore'))
    # Fallback scan
    for p in ROOT.rglob('*'):
        if p.is_file() and p.suffix in EXTS:
            try:
                texts.append(p.read_text(errors='ignore'))
            except Exception:
                pass
    return '\n'.join(texts)


def make_seed_from_text(txt):
    seeds = set()
    words = [w for w in txt.split() if len(w) > 0]
    # tokens and substrings
    for _ in range(200):
        if random.random() < 0.4 and words:
            n = random.randint(1, min(10, len(words)))
            i = random.randint(0, max(0, len(words)-n))
            seeds.add(' '.join(words[i:i+n]))
        else:
            start = random.randint(0, max(0, len(txt)-1))
            length = random.randint(1, min(200, len(txt)-start))
            seeds.add(txt[start:start+length])
    return seeds


def make_random_seed():
    # random bytes but try to keep them utf-8 friendly sometimes
    if random.random() < 0.6:
        size = random.randint(1, 200)
        s = ''.join(chr(random.randint(32, 126)) for _ in range(size))
    else:
        size = random.randint(1, 200)
        s = bytes(random.getrandbits(8) for _ in range(size))
        try:
            s = s.decode('utf-8', errors='ignore')
        except Exception:
            s = ''.join(chr(b) for b in s)
    return s


def main(n_new=1000):
    CORPUS.mkdir(parents=True, exist_ok=True)
    txt = gather_text()
    seeds = set()
    seeds.update(make_seed_from_text(txt))
    while len(seeds) < n_new:
        seeds.add(make_random_seed())
    # Write seeds
    existing = set(p.name for p in CORPUS.iterdir() if p.is_file())
    i = 0
    for s in seeds:
        name = f'auto-{i:05d}.txt'
        i += 1
        if name in existing:
            continue
        try:
            with open(CORPUS / name, 'wb') as f:
                if isinstance(s, str):
                    f.write(s.encode('utf-8', errors='ignore'))
                else:
                    f.write(s)
        except Exception:
            pass
    print('Wrote', i, 'new seeds to', CORPUS)

if __name__ == '__main__':
    n = 1000
    if len(sys.argv) > 1:
        n = int(sys.argv[1])
    main(n)
