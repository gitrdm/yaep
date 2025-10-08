#!/usr/bin/env python3
"""Replace C-style casts with YAEP_STATIC_CAST macro."""

import re
import sys

def replace_casts(content):
    """Replace simple C-style casts with YAEP_STATIC_CAST."""
    
    # Patterns for simple casts: (type)expr where type is a simple type
    patterns = [
        # Pointer casts
        (r'\(void \*\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(void *, \1)'),
        (r'\(int \*\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(int *, \1)'),
        (r'\(char \*\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(char *, \1)'),
        (r'\(const char \*\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(const char *, \1)'),
        (r'\(struct symb \*\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(struct symb *, \1)'),
        (r'\(const struct symb \*\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(const struct symb *, \1)'),
        
        # Simple numeric casts (but be careful - only match simple identifiers)
        (r'\(unsigned\)\s*([a-zA-Z_][a-zA-Z0-9_]*(?:->[a-zA-Z_][a-zA-Z0-9_]*)?(?:\.[a-zA-Z_][a-zA-Z0-9_]*)?)', r'YAEP_STATIC_CAST(unsigned, \1)'),
        (r'\(int\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(int, \1)'),
        (r'\(short\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(short, \1)'),
        (r'\(char\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(char, \1)'),
        (r'\(size_t\)\s*([a-zA-Z_][a-zA-Z0-9_]*)', r'YAEP_STATIC_CAST(size_t, \1)'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content)
    
    return content

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <file>")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    with open(filename, 'r') as f:
        content = f.read()
    
    new_content = replace_casts(content)
    
    with open(filename, 'w') as f:
        f.write(new_content)
    
    print(f"Fixed casts in {filename}")

if __name__ == '__main__':
    main()
