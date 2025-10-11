# YAEP - Yet Another Earley Parser

[![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)](CHANGELOG.md)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

**YAEP** is a high-performance Earley parser library designed for building production-quality parsers for programming languages and other complex grammars.

* Fast and memory-efficient implementation
* Supports ambiguous grammars with syntax-directed translation
* Built-in error recovery with minimal token skipping
* YACC-like grammar description syntax
* Licensed under the MIT license

## Key Features

  * **High Performance**: One of the fastest Earley parser implementations available, 
    with efficient memory usage suitable for parsing large source files.
  * **Syntax-Directed Translation**: Produces abstract syntax trees (AST) as output.
  
  * **Ambiguous Grammar Support**: Can parse input described by ambiguous grammars. 
    Parse results can be a single abstract tree or all possible parse trees. 
    Uses compact DAG (Directed Acyclic Graph) representation instead of duplicating trees.
  
  * **Cost-Based Parsing**: Supports abstract node costs for ambiguous grammars, 
    producing minimal cost parse trees. Useful for code selection in compilers.
  
  * **Error Recovery**: Intelligent syntax error recovery that minimizes the number 
    of ignored tokens, enabling high-quality error reporting.
  
  * **Fast Startup**: Minimal delay between grammar processing and parsing.
  
  * **Flexible Grammar Definition**: Grammars can be constructed programmatically 
    via function calls or using a YACC-like description syntax.
 
## Usage Example
* The following is a small example of how to use YAEP to parse expressions.
  We have omitted the functions `read_token`, `syntax_error_func`,
  and `parse_alloc_func` which are needed to provide tokens, print syntax
  error messages, and allocate memory for the parser.

```
static const char *description =
"\n"
"TERM NUMBER;\n"
"E : T         # 0\n"
"  | E '+' T   # plus (0 2)\n"
"  ;\n"
"T : F         # 0\n"
"  | T '*' F   # mult (0 2)\n"
"  ;\n"
"F : NUMBER    # 0\n"
"  | '(' E ')' # 1\n"
"  ;\n"
  ;

static void parse (void)
{
  struct grammar *g;
  struct earley_tree_node *root;
  int ambiguous_p;

  if ((g = earley_create_grammar ()) == NULL) {
      fprintf (stderr, "earley_create_grammar: No memory\n");
      exit (1);
  }
  if (earley_parse_grammar (g, TRUE, description) != 0) {
      fprintf (stderr, "%s\n", earley_error_message (g));
      exit (1);
    }
  if (earley_parse (g, read_token_func, syntax_error_func, parse_alloc_func,
                    NULL, &root, &ambiguous_p))
    fprintf (stderr, "earley_parse: %s\n", earley_error_message (g));
  earley_free_grammar (g);
}
```
  * To add error recovery, just add a reserved symbol ``error`` to
    the rules. Skipped terminals during error recovery will be
    represented in the resulting abstract tree by a node called ``error``.
    For example, if you want to include expression- and statement-level
    error-recovery in a programming language grammar, the rules could look
    like the following:
```
  stmt : IF '(' expr ')' stmt ELSE stmt # if (2 4 6)
       | ...
       | error # 0
       ;
  expr : IDENT # 0
       | ...
       | error # 0
       ;
``` 
For more details, see:
* **API Documentation**: `doc/YAEP_C_API.md` (C interface) and `doc/YAEP_CPP_API.md` (C++ interface)
* **Examples**: Test files in `test/C/` and `test/C++/` directories
* **Code Documentation**: Generate with `doxygen Doxyfile`

## Building and Installing
  * ``mkdir build``
  * ``cd build``
  * ``cmake .. -DCMAKE_BUILD_TYPE=Release``
  * ``cmake --build .``
  * ``ctest`` (optional, runs the test suite)
  * ``cmake --install . --prefix <install_prefix>``

## Performance Characteristics

YAEP is designed for production use with excellent performance characteristics:

* **Compared to other Earley parsers**: Benchmarks on ANSI C grammar parsing show YAEP is 
  approximately **20× faster** and uses **200× less memory** than alternative Earley 
  parser implementations (e.g., MARPA).

* **Compared to YACC/Bison**: While YACC-generated parsers remain the fastest option 
  (approximately **2-6× faster** than YAEP), YAEP provides crucial advantages:
  - Handles ambiguous grammars
  - Better error recovery
  - Simpler grammar descriptions
  - No need for grammar preprocessing

* **Trade-offs**: YAEP provides a sweet spot between the flexibility of general parsers 
  and the performance of specialized parser generators, making it ideal for language 
  development, prototyping, and production parsers where ambiguous grammars or superior 
  error recovery are needed.

The `test/compare_parsers/` directory contains benchmark code for comparing YAEP against 
other parser implementations.

## Documentation

* **[C API Reference](doc/YAEP_C_API.md)** - Complete C interface documentation
* **[C++ API Reference](doc/YAEP_CPP_API.md)** - Complete C++ interface documentation
* **[Changelog](CHANGELOG.md)** - Version history and migration guide
* **[Build System Modernization](BUILD_AND_DOC_MODERNIZATION.md)** - Details on recent improvements
* **[Doxygen Integration](DOXYGEN_INTEGRATION.md)** - Auto-generated code documentation

Generate comprehensive code documentation:
```bash
doxygen Doxyfile
xdg-open doc/doxygen/html/index.html
```

## Testing

Run the complete test suite (126 tests):
```bash
./build_and_test.sh
```

Or use CMake/CTest directly:
```bash
cd build
ctest --output-on-failure
```

## Project Status

This is a fork of the original YAEP project, modernized with:

**Build System:**
* Updated build system (CMake only)
* Parallel build support (race condition fixed)
* Comprehensive test suite with automated tests

**Code Quality:**
* Modernized to C17 from C99
* Fixed known memory leaks and segfaults
* Extensively fuzzed under Valgrind with no known issues
* Production-ready and stable

**Documentation:**
* Modern documentation (Markdown + Doxygen)
* Removed deprecated tooling (Autotools, SGML documentation)

Contributions welcome!

## License

MIT License - See [LICENSE](LICENSE) file for details.
