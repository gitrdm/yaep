# YAEP Examples

This directory contains example YAEP grammar files demonstrating various parser capabilities.

## Files in this Directory

- `meta-grammar.yaep` - YAEP grammar for parsing EBNF files
- `parse_meta_grammar.c` - Example C program using the meta-grammar parser
- `test_simple.ebnf` - Simple test EBNF grammar for testing
- `Makefile` - Build system for examples
- `README.md` - This file

## Quick Start

```bash
# Build the example parser
make

# Parse the meta-grammar itself (self-parsing!)
./parse_meta_grammar ../meta-grammar.ebnf

# Parse a simple test grammar
./parse_meta_grammar test_simple.ebnf
```

## meta-grammar.yaep

A YAEP grammar for parsing EBNF meta-grammar files (like `../meta-grammar.ebnf`).

### Purpose

This grammar demonstrates how to use YAEP in scannerless mode to parse a complete grammar definition language. It parses the EBNF notation and can be used to:

- Validate EBNF grammar files
- Build AST representations of grammars
- Translate EBNF to other grammar formats
- Implement a meta-parser for grammar-driven tools

### Features

- **Scannerless parsing**: Characters are consumed directly by grammar rules
- **Explicit whitespace handling**: Comments and whitespace are grammar productions
- **Unicode support**: Handles Unicode identifiers and string literals
- **Character classes**: Supports regex-style character classes `[a-z]`, `[^abc]`
- **Escape sequences**: Full support for `\n`, `\x41`, `\u0041`, `\U00000041`, etc.
- **Quantifiers**: `?`, `*`, `+`, `{n}`, `{n,}`, `{n,m}`
- **Lookahead**: Positive `&` and negative `~` lookahead predicates
- **Directives**: `@directive(arg);` style meta-directives

### Structure

The grammar is organized into sections:

1. **Top-level**: `grammar` and `grammar_items`
2. **Directives**: Parse `@name(arg);` style directives
3. **Rules**: Parse `identifier [annotation] = expression ;`
4. **Expressions**: Alternation, sequence, terms, factors
5. **Character classes**: `[...]` with ranges and negation
6. **String literals**: Single and double-quoted strings with escapes
7. **Escape sequences**: All standard escape forms
8. **Identifiers and numbers**: Basic lexical elements
9. **Character categories**: Letters, digits, hex digits, etc.
10. **Whitespace and comments**: Explicit `sp`, line and block comments

### Implementation Notes

Some parts of this grammar are marked as placeholders for implementation:

- **Unicode letters**: `unicode_letter` needs expansion to full Unicode letter ranges
- **Character predicates**: Productions like `any_char_except_newline` would need to be:
  - Implemented as YAEP predicates in C/C++, OR
  - Expanded to explicit character enumerations

For example, `any_char_except_newline` could be implemented as:
```c
int predicate_any_except_newline(int codepoint) {
    return codepoint != '\n';
}
```

And registered with YAEP as a terminal predicate.

### Using the Grammar

#### In C:

```c
#include "yaep.h"

struct grammar *g = yaep_create_grammar();
const char *grammar_text = /* read meta-grammar.yaep */;

if (yaep_read_grammar(g, &grammar_text, 1) != 0) {
    fprintf(stderr, "Error: %s\n", yaep_error_message(g));
    exit(1);
}

/* Now parse an EBNF file using this grammar */
const char *ebnf_input = /* read some.ebnf */;
struct yaep_tree_node *root;
int ambiguous;

if (yaep_parse(g, read_token_func, syntax_error_func, 
               alloc_func, free_func, &root, &ambiguous) != 0) {
    fprintf(stderr, "Parse error: %s\n", yaep_error_message(g));
    exit(1);
}

/* Process the parse tree */
process_tree(root);

yaep_free_grammar(g);
```

#### In Python:

```python
import yaep

# Create grammar instance
g = yaep.Grammar()

# Load the meta-grammar
with open('examples/meta-grammar.yaep', 'r') as f:
    grammar_text = f.read()

g.read_grammar(grammar_text)

# Parse an EBNF file
with open('meta-grammar.ebnf', 'r') as f:
    ebnf_input = f.read()

try:
    tree, ambiguous = g.parse(ebnf_input)
    print(f"Parse successful! Ambiguous: {ambiguous}")
    # Process tree...
except yaep.ParseError as e:
    print(f"Parse error: {e}")
```

### Extending the Grammar

To add features:

1. **Add new directive types**: Modify the `directive` and `argument` rules
2. **Add annotation types**: Extend the `attribute` and `attr_value` rules
3. **Add new operators**: Add alternatives to `factor` or `quantifier`
4. **Implement character predicates**: Replace placeholder rules with actual implementations

### Testing

To test this grammar, you can:

1. Parse the `meta-grammar.ebnf` file itself (self-parsing!)
2. Create simple test EBNF grammars
3. Validate against known-good EBNF files

Example test:
```ebnf
/* Simple test grammar */
expr = term | expr '+' term ;
term = factor | term '*' factor ;
factor = '(' expr ')' | NUMBER ;
```

### Comparison with Original EBNF

The original `meta-grammar.ebnf` uses EBNF notation with:
- `{ ... }` for zero-or-more repetition
- `[ ... ]` for optional elements
- `|` for alternation
- `.` to end rules

This YAEP grammar uses YACC-like notation with:
- Recursive rules for repetition (e.g., `items : item | items item`)
- Optional alternatives (e.g., `opt : /* empty */ | value`)
- `|` for alternation
- `;` to end rules

### Future Enhancements

Potential improvements:

1. **Translation annotations**: Add `#` annotations to build proper AST nodes
2. **Error recovery**: Add error productions for better error messages
3. **Semantic actions**: Integrate with YAEP's translation mechanism
4. **Performance**: Optimize for common patterns
5. **Full Unicode**: Complete Unicode letter/digit category support

### References

- `../meta-grammar.ebnf` - The EBNF being parsed
- `../YAEP_GRAMMAR.md` - YAEP grammar syntax documentation
- `../doc/yaep.txt` - Full YAEP documentation
- `../test/C/` - Example test programs using YAEP
