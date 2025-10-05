# YAEP Python prototype bindings

This `python/` subtree contains a small, documented prototype Python wrapper
around the YAEP C library. The wrapper is intentionally thin: it mirrors the
YAEP public API, adds Pythonic ownership semantics (context managers and
explicit free), and exposes utilities for converting YAEP parse trees into
Python-native structures and Graphviz DOT.

This README documents the current public Python API, points to the example
and tests that demonstrate usage, and gives quickstart instructions.

## Quickstart

- Build YAEP (out-of-tree):

```bash
mkdir -p build && cd build
cmake .. && cmake --build .
```

- Install local dev deps into the repository virtualenv (the workspace
  usually contains `.venv`):

```bash
/home/rdmerrio/gits/yaep/.venv/bin/python -m pip install -e python[dev]
```

- Run the Python tests from the repository root:

```bash
export PYTHONPATH=python/src
export YAEP_LIB_PATH=build/src/libyaep.so
/home/rdmerrio/gits/yaep/.venv/bin/python -m pytest -q python/tests
```

## Notes

- The wrapper uses CFFI to dynamically load the YAEP shared library. If
  YAEP was built to a non-default path set `YAEP_LIB_PATH` to point at the
  shared library (for example `build/src/libyaep.so`). If `YAEP_LIB_PATH`
  isn't set the wrapper attempts to resolve a sensible default.
- This is a prototype and aims to be documentation-through-examples; the
  Python API is intentionally small and well-tested — the tests and
  examples serve as living documentation.

## Types and exceptions

- `Grammar`
  - Thin RAII-style wrapper around a YAEP `struct grammar *`.
  - Methods demonstrated: `parse_description()`, `parse_description_bytes()`,
    `parse()`, `error_code()`, `error_message()`, `free()`,
    `set_lookahead_level()`.
  - See examples: `python/examples/visualize_parse_tree.py`.
  - See tests: `python/tests/test_parse_valid_and_invalid.py`,
    `python/tests/test_parse_with_tokens.py`.

- `ParseTree`
  - Lightweight wrapper holding the YAEP parse tree root pointer and an
    `ambiguous` flag. Supports context manager protocol and `free()`.
  - Demonstrated in `python/tests/test_parse_with_tokens.py` and consumed by
    `python/src/yaep_python/tree_utils.py` utilities.

- `ParseNode`
  - Small read-only wrapper for an individual yaep_tree_node pointer. Used
    by `ParseTree` for convenience access to the root node.

- Exceptions
  - `YaepError` — base class for YAEP-related Python exceptions.
  - `YaepInvalidUTF8Error` — raised when YAEP reports invalid UTF‑8; the
    wrapper maps YAEP's `YAEP_INVALID_UTF8` error code to this exception to
    provide more idiomatic Python error handling for tests and examples.

### Constants
- `YAEP_INVALID_UTF8` — mirrored constant (int) from YAEP for convenience.

### Key methods and behavior

- `Grammar.parse_description(description: str, strict: bool=True) -> int`
  - Parse a grammar description provided as a Python `str`; the wrapper
    UTF-8 encodes the text and forwards it to YAEP. Returns YAEP return code.
  - Demonstrated in: `python/examples/visualize_parse_tree.py`,
    `python/tests/test_parse_with_tokens.py`.

- `Grammar.parse_description_bytes(buf: bytes, strict: bool=True) -> int`
  - Pass raw bytes unchanged to YAEP (useful for testing YAEP's UTF-8
    validation); raises `YaepInvalidUTF8Error` on YAEP's invalid-UTF-8
    result. Demonstrated in `python/tests/test_parse_valid_and_invalid.py`.

- `Grammar.parse(tokens: Iterable[int]) -> Tuple[int, Optional[ParseTree], Optional[Tuple[int, int, int]]]`
  - Parse a stream of integer token codes. Returns (rc, ParseTree|None, syntax_error_info|None).
  - syntax_error_info: (err_tok_num, start_ignored_tok_num, start_recovered_tok_num) if syntax error occurred, else None.
  - Tokenization strategy: character literal terminals are represented by
    their `ord()` value in examples (see `visualize_parse_tree.py`).
  - Demonstrated in: `python/examples/visualize_parse_tree.py`, `python/tests/test_parse_with_tokens.py`.

- `Grammar.error_code() -> int`
- `Grammar.error_message() -> Optional[str]`
  - Diagnostic helpers that surface YAEP's last error code/message.

- `Grammar.set_lookahead_level(level: int) -> int`
  - Mirrors `yaep_set_lookahead_level()`; returns previously set level.

- `Grammar.read_grammar_from_lists(terminals, rules, strict=True)`
  - Build a grammar programmatically from Python lists of terminals and rules.
  - Terminals: `[(name, code?), ...]` where code is int or None for auto-assign.
  - Rules: `[{'lhs': str, 'rhs': [str], 'abs_node': str?, 'anode_cost': int?, 'transl': [int]?}, ...]`
  - See examples: `python/examples/read_grammar_example.py`.
  - See tests: `python/tests/test_read_grammar.py`.

- `Grammar.full_parse(read_token, syntax_error=None, parse_alloc=None, parse_free=None)`
  - Parse with full YAEP callbacks for custom token reading, error handling, and memory management.
  - See tests: `python/tests/test_full_parse.py`.

# YAEP Public API Coverage and Implementation Assumptions

This section lists the **full YAEP public C API** as defined in `src/yaep.h`. For each function, we describe the current Python wrapper implementation status, roundtrip assumptions (how Python types map to C types and back), memory safety considerations, and abstraction assumptions (how we simplify or expose the API for Python users).

The wrapper aims for full coverage with safe, idiomatic Python bindings. Where a function is not yet wrapped, we note it and plan to add it with tests and examples.

## Grammar Lifecycle and Diagnostics

### `yaep_create_grammar(void) -> struct grammar *`
- **Status**: Wrapped as `Grammar.__init__()` (high-level) and `_cffi.create_grammar()` (low-level).
- **Roundtrip**: Returns a `struct grammar *` pointer; Python wraps it in a `Grammar` instance with RAII (context manager and explicit `free()`).
- **Memory Safety**: The pointer is managed by the `Grammar` class; `free()` or `__exit__` calls `yaep_free_grammar`. No leaks if used as context manager.
- **Abstraction**: Hidden behind `Grammar()` constructor; users don't see raw pointers.
- **Tests**: `python/tests/test_smoke.py` (low-level), all other tests via `Grammar.__init__()`.
- **Examples**: All examples via `Grammar()`.

### `yaep_free_grammar(struct grammar *g)`
- **Status**: Wrapped as `Grammar.free()` (high-level) and `_cffi.free_grammar()` (low-level).
- **Roundtrip**: Takes the pointer; no return value.
- **Memory Safety**: Must be called after all parsing; the wrapper ensures it's called via RAII or explicit free. Double-free is prevented by setting `_g = None`.
- **Abstraction**: Called automatically in `Grammar.__exit__`; users call `free()` explicitly if needed.
- **Tests**: `python/tests/test_smoke.py` (low-level), all other tests via `Grammar.free()` or context manager.
- **Examples**: All examples via `Grammar.free()` or context manager.

### `yaep_error_code(struct grammar *g) -> int`
- **Status**: Wrapped as `Grammar.error_code()` (high-level) and `_cffi.error_code()` (low-level).
- **Roundtrip**: Returns int error code; Python returns `int`.
- **Memory Safety**: No issues; reads state from grammar pointer.
- **Abstraction**: Exposed directly; users check after operations that may fail.
- **Tests**: `python/tests/test_parse_valid_and_invalid.py` (checks error codes).
- **Examples**: `python/examples/visualize_parse_tree.py` (prints error code).

### `yaep_error_message(struct grammar *g) -> const char *`
- **Status**: Wrapped as `Grammar.error_message()` (high-level) and `_cffi.error_message()` (low-level).
- **Roundtrip**: Returns UTF-8 string pointer; Python decodes to `str` or returns `None` if NULL.
- **Memory Safety**: String is owned by YAEP; Python copies it immediately.
- **Abstraction**: Exposed directly; users get human-readable messages.
- **Tests**: `python/tests/test_parse_valid_and_invalid.py` (checks error messages).
- **Examples**: `python/examples/visualize_parse_tree.py` (prints error message).

## Grammar Population

### `yaep_read_grammar(struct grammar *g, int strict_p, const char *(*read_terminal)(int *code), const char *(*read_rule)(const char ***rhs, const char **abs_node, int *anode_cost, int **transl)) -> int`
- **Status**: Wrapped as `Grammar.read_grammar_from_lists()` (high-level) and `_cffi.read_grammar_from_lists()` (low-level).
- **Roundtrip**: Callbacks return C strings and arrays; Python allocates CFFI-managed arrays in callbacks and keeps references alive until YAEP consumes them.
- **Memory Safety**: Callbacks allocate C arrays (e.g., for RHS and transl) that persist until YAEP reads them; CFFI buffers and closure state prevent GC.
- **Abstraction**: High-level wrapper accepts `terminals = [(name, code?), ...]` and `rules = [{'lhs': str, 'rhs': [str], 'abs_node': str?, 'anode_cost': int?, 'transl': [int]?}, ...]`; builds C callbacks internally.
- **Tests**: `python/tests/test_read_grammar.py`.
- **Examples**: `python/examples/read_grammar_example.py`.

### `yaep_parse_grammar(struct grammar *g, int strict_p, const char *description) -> int`
- **Status**: Wrapped as `Grammar.parse_description()` and `Grammar.parse_description_bytes()` (high-level); `_cffi.parse_grammar()` and `_cffi.parse_grammar_bytes()` (low-level).
- **Roundtrip**: Takes UTF-8 string; Python encodes `str` to bytes or passes `bytes` directly.
- **Memory Safety**: String is copied by YAEP; no lifetime issues.
- **Abstraction**: `parse_description_bytes()` passes raw bytes to avoid re-encoding; raises `YaepInvalidUTF8Error` on invalid UTF-8.
- **Tests**: `python/tests/test_parse_valid_and_invalid.py`.
- **Examples**: `python/examples/visualize_parse_tree.py`.

## Parser Configuration

### `yaep_set_lookahead_level(struct grammar *g, int level) -> int`
- **Status**: Wrapped as `Grammar.set_lookahead_level()` (high-level) and `_cffi.set_lookahead_level()` (low-level).
- **Roundtrip**: Takes/returns int; Python int <-> C int.
- **Memory Safety**: No issues; sets state.
- **Abstraction**: Exposed directly; users set level (1=default static, 2=dynamic).
- **Tests**: `python/tests/test_set_lookahead_level.py` (checks behavior with different levels).
- **Examples**: None.

### `yaep_set_debug_level(struct grammar *g, int level) -> int`
- **Status**: Wrapped as `Grammar.set_debug_level()` (high-level) and `_cffi.set_debug_level()` (low-level).
- **Roundtrip**: Takes/returns int; Python int <-> C int.
- **Memory Safety**: No issues; sets state.
- **Abstraction**: Exposed directly; users set level (0=default no debug).
- **Tests**: Not specifically tested.
- **Examples**: None.

### `yaep_set_one_parse_flag(struct grammar *g, int flag) -> int`
- **Status**: Wrapped as `Grammar.set_one_parse_flag()` (high-level) and `_cffi.set_one_parse_flag()` (low-level).
- **Roundtrip**: Takes/returns int; Python int <-> C int.
- **Memory Safety**: No issues; sets state.
- **Abstraction**: Exposed directly; users set flag (1=default one parse tree).
- **Tests**: `python/tests/test_set_one_parse_flag.py` (checks behavior with different flags).
- **Examples**: None.

### `yaep_set_cost_flag(struct grammar *g, int flag) -> int`
- **Status**: Wrapped as `Grammar.set_cost_flag()` (high-level) and `_cffi.set_cost_flag()` (low-level).
- **Roundtrip**: Takes/returns int; Python int <-> C int.
- **Memory Safety**: No issues; sets state.
- **Abstraction**: Exposed directly; users set flag (0=default no cost minimization).
- **Tests**: Not specifically tested.
- **Examples**: None.

### `yaep_set_error_recovery_flag(struct grammar *g, int flag) -> int`
- **Status**: Wrapped as `Grammar.set_error_recovery_flag()` (high-level) and `_cffi.set_error_recovery_flag()` (low-level).
- **Roundtrip**: Takes/returns int; Python int <-> C int.
- **Memory Safety**: No issues; sets state.
- **Abstraction**: Exposed directly; users set flag (1=default error recovery enabled).
- **Tests**: `python/tests/test_set_error_recovery_flag.py` (checks behavior with different flags).
- **Examples**: None.

### `yaep_set_recovery_match(struct grammar *g, int n_toks) -> int`
- **Status**: Wrapped as `Grammar.set_recovery_match()` (high-level) and `_cffi.set_recovery_match()` (low-level).
- **Roundtrip**: Takes/returns int; Python int <-> C int.
- **Memory Safety**: No issues; sets state.
- **Abstraction**: Exposed directly; users set number of tokens to match for recovery (3=default).
- **Tests**: `python/tests/test_set_recovery_match.py` (checks behavior with different recovery match settings).
- **Examples**: None.

## Parsing

### `yaep_parse(struct grammar *g, int (*read_token)(void **attr), void (*syntax_error)(int, void *, int, void *, int, void *), void *(*parse_alloc)(int nmemb), void (*parse_free)(void *mem), struct yaep_tree_node **root, int *ambiguous_p) -> int`
- **Status**: Wrapped as `Grammar.parse()` (token iterable) and `Grammar.full_parse()` (full callbacks); `_cffi.parse_with_tokens()` and `_cffi.parse_full()` (low-level).
- **Roundtrip**: Callbacks for tokens/syntax errors/alloc/free; Python callables converted to CFFI callbacks. Tree root returned as pointer, wrapped in `ParseTree`.
- **Memory Safety**: Tree root freed via RAII; custom alloc/free allow user control but require careful cdata management.
- **Abstraction**: `parse()` builds token callback from iterable; `full_parse()` exposes all callbacks for advanced use.
- **Tests**: `python/tests/test_parse_with_tokens.py`, `python/tests/test_full_parse.py`.
- **Examples**: `python/examples/visualize_parse_tree.py`.

## Tree Management

### `yaep_free_tree(struct yaep_tree_node *root, void (*parse_free)(void *), void (*termcb)(struct yaep_term *term))`
- **Status**: Wrapped as `ParseTree.free()` (default) and `ParseTree.free_with_termcb()` (with termcb); `_cffi.free_tree()` (low-level).
- **Roundtrip**: Takes root pointer and optional callbacks; Python callable for termcb converted to CFFI callback.
- **Memory Safety**: Freed after `yaep_free_grammar`; RAII ensures safety. Termcb allows freeing custom term attributes.
- **Abstraction**: Automatic in `ParseTree.__exit__`; `free_with_termcb()` for custom term attribute freeing.
- **Tests**: `python/tests/test_free_with_termcb.py`.
- **Examples**: None specific.

## C++ Class Methods (Mirrored from C Functions)

The C++ class `yaep` mirrors the C functions above. Since the Python wrapper targets the C API, we don't separately wrap the C++ methods. Users can access equivalent functionality through the Python `Grammar` class.

## Summary of Coverage

- **Fully Wrapped**: All YAEP public C API functions are fully covered with safe, idiomatic Python bindings, including advanced callback support for parsing and tree freeing.
- **Not Wrapped**: None; complete coverage achieved.

## Design goals

- Keep the wrapper small and well-tested — prefer tests and examples to
  large API docs: the tests are the definitive documentation for behavior.
- Next incremental improvements (open tasks):
  - Convert YAEP parse trees to a small set of Python dataclasses to make
    traversals and transformations easier.
  - Expand exception mapping beyond invalid UTF-8 to other common YAEP
    error codes and add tests that assert those mappings.
  - Add a CI workflow that builds YAEP and runs the Python tests on every
    push.

## Examples and tests (living documentation)

### Examples (human-oriented):
- `python/examples/visualize_parse_tree.py`
  - Uses the `test/C/test07.c` grammar, parses the input
    `"a+a*(a*a+a)"`, prints a JSON-friendly representation of YAEP's
    translation tree (TERM nodes include `code` and `char` when printable),
    and emits Graphviz DOT source via `tree_utils.to_dot()` for visualization.
  - See `python/examples/README.md` for DOT -> PNG workflow.

- `python/examples/read_grammar_example.py`
  - Demonstrates `Grammar.read_grammar_from_lists()` to build a grammar programmatically from Python lists, parse input, and convert the tree to a dict.

### Tests (machine-verifiable documentation):
- `python/tests/test_smoke.py`
  - Low-level smoke test using `_cffi` to create/parse/free a grammar.
- `python/tests/test_parse_valid_and_invalid.py`
  - Tests `Grammar.parse_description()` for a normal grammar and
    `Grammar.parse_description_bytes()` for detecting invalid UTF-8; shows
    how the wrapper maps YAEP_INVALID_UTF8 to `YaepInvalidUTF8Error`.
- `python/tests/test_parse_with_tokens.py`
  - Demonstrates `Grammar.parse()` with an empty token stream and
    verifies the wrapper returns an integer rc and either a `ParseTree`
    or `None` safely.
- `python/tests/test_tree_utils.py`
  - Verifies `tree_utils.to_dict()` handles empty/None parse trees and
    returns a dict when appropriate.
- `python/tests/test_full_parse.py`
  - Demonstrates `Grammar.full_parse()` with custom token reading and error handling.
- `python/tests/test_free_with_termcb.py`
  - Tests `ParseTree.free_with_termcb()` with a dummy term callback.
- `python/tests/test_read_grammar.py`
  - Tests `Grammar.read_grammar_from_lists()` with various terminal and rule configurations.
- `python/tests/test_set_lookahead_level.py`
  - Tests `Grammar.set_lookahead_level()` with different lookahead levels and verifies the behavior.
- `python/tests/test_set_debug_level.py`
  - Tests `Grammar.set_debug_level()` with different debug levels and verifies the behavior.
- `python/tests/test_set_one_parse_flag.py`
  - Tests `Grammar.set_one_parse_flag()` with different parse flag settings and verifies the behavior.
- `python/tests/test_set_cost_flag.py`
  - Tests `Grammar.set_cost_flag()` with different cost flag settings and verifies the behavior.
- `python/tests/test_set_error_recovery_flag.py`
  - Tests `Grammar.set_error_recovery_flag()` with different error recovery flag settings and verifies the behavior.
- `python/tests/test_set_recovery_match.py`
  - Tests `Grammar.set_recovery_match()` with different recovery match settings and verifies the behavior.
