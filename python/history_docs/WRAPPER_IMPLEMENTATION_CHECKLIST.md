YAEP Python Wrapper — Implementation Checklist
============================================

Purpose
-------
This file is a single, durable, and LLM-friendly checklist intended to guide a full production-grade Python wrapper for the YAEP C library. It is written so that an LLM coding agent (or a human) can pick up work at any point and continue safely without assuming prior context beyond the repository layout and the public header `src/yaep.h` and the `python/yaep.txt` guidance file.

Design principles (non-negotiable)
---------------------------------
- No technical debt: Do not add stubs, placeholders, or partially-implemented APIs as "temporary" fixes. Every added symbol or file must be production-ready, tested, and documented. If a feature is out-of-scope, list it on a TODO backlog but do not commit half-implemented code.
- Robust production code: Prefer clear ownership semantics, deterministic resource management, and explicit error handling. Use typed interfaces and runtime validation where helpful.
- Tests for all code: Every public function, class, or module must have unit tests and at least one integration test exercising the native C library end-to-end.
- Literate Sphinx-style docs: Document modules and public APIs with Sphinx/reStructuredText docstrings and include literate examples in tests or .rst files so docs can be auto-extracted.
- Examples & visualizations: Provide runnable example scripts that generate human-friendly visualizations of parse trees (Graphviz/dot, ASCII), and small demo notebooks if appropriate.

Repository assumptions and canonical paths
-----------------------------------------
- Repository root: project C and build artifacts exist at `build/`.
- YAEP C headers at `src/yaep.h` (public API). Use that file as the authoritative source for function signatures and error codes.
- Python package root: `python/src/yaep_python/`.
- Python tests: `python/tests/`.
- Docs: `python/docs/` (Sphinx sources).

High-level milestones
---------------------
1. Define the Python public surface (classes, exceptions, function names).
2. Implement low-level C bindings using CFFI (dynamic mode) in `python/src/yaep_python/_cffi.py`.
3. Implement high-level Python API wrappers in `python/src/yaep_python/__init__.py` (or submodules) that enforce ownership contracts, types, and memory management.
4. Add comprehensive unit tests and integration tests under `python/tests/`.
5. Add Sphinx-based docs and literate examples under `python/docs/` and integrate with `pyproject.toml`.
6. Add example scripts for visualizing parse trees using Graphviz (`python/examples/`).
7. Add CI to build the C library and run Python tests + docs build.

Step 1 — Public Python API (contract)
------------------------------------
Define a small, stable public API first. Design decisions must be explicit in this file.

Public classes and functions (suggested):
- class Grammar
  - Grammar() -> Resource-allocating constructor
  - parse_description(str, strict: bool=True) -> int
  - parse_description_bytes(buf: bytes, strict: bool=True) -> int | raises YaepInvalidUTF8Error
  - parse(tokens: Iterable[int]) -> Tuple[int, Optional[ParseTree]]
  - set_lookahead_level(level: int) -> int
  - set_debug_level(level: int) -> int
  - set_one_parse_flag(flag: int) -> int
  - set_cost_flag(flag: int) -> int
  - set_error_recovery_flag(flag: int) -> int
  - set_recovery_match(n_toks: int) -> int
  - error_code() -> int
  - error_message() -> Optional[str]
  - free() -> None
  - __enter__/__exit__ for context manager

- class ParseTree
  - factory returned by Grammar.parse
  - supports .root (ParseNode or None)
  - .ambiguous: bool
  - .free(), __enter__/__exit__, __del__ to ensure deterministic freeing via yaep_free_tree

- class ParseNode
  - .type() -> int (enum)
  - .is_term(), .term_code(), .is_anode(), .anode_name(), .children() -> Iterable[ParseNode]
  - RECOMMEND: minimal, read-only view; heavy traversal should convert to Python-native structures

- Exceptions
  - YaepError(base)
  - YaepInvalidUTF8Error(YaepError)
  - YaepParseError(YaepError) — general parse-time error mapping
  - Add others as they are mapped from `yaep.h` error codes

Ownership and memory contract (must be documented near the API)
- Grammar owns the C `struct grammar *` until free() is called (or __exit__).
- ParseTree wraps a C parse tree pointer. The wrapper must call `yaep_free_tree(root, NULL, NULL)` to release it unless the parse was invoked with a Python-managed allocator.
- Do not expose raw pointers publicly. All C ownership operations are mediated by wrapper methods.

Step 2 — Low-level binding with CFFI
-----------------------------------
Location: `python/src/yaep_python/_cffi.py`

Guidance:
- Use CFFI dynamic mode (`ffi = FFI()` + `ffi.cdef(...)` + `ffi.dlopen(libpath)`). This allows runtime loading of the built shared lib and easy CI.
- Prefer minimal, exact `cdef` signatures that match `src/yaep.h`. Copy prototypes; do not invent types.
- Provide helper wrappers in the module for:
  - creating and freeing grammars
  - parse_grammar (string and bytes variants)
  - error code & error message retrieval
  - yaep_parse helper that accepts C callbacks wired to Python callables (careful with callback lifetimes)
  - yaep_free_tree wrapper

Important: callbacks and lifetime
- CFFI callbacks must be kept referenced on the Python side as long as C may call them (store them in closures).
- Implement token iterator wrapper with a reference to the iterator and keep callbacks alive until parse returns.

Suggested CFFI cdef snippets (must match `yaep.h`):
```c
typedef struct grammar grammar;
grammar *yaep_create_grammar(void);
void yaep_free_grammar(grammar *g);
int yaep_parse_grammar(grammar *g, int strict_p, const char *description);
int yaep_parse_grammar_bytes(grammar *g, int strict_p, const char *buf); // if you add convenience wrapper
int yaep_error_code(struct grammar *g);
const char *yaep_error_message(struct grammar *g);
int yaep_read_grammar(struct grammar *g, int strict_p,
  const char *(*read_terminal) (int *code),
  const char *(*read_rule) (const char ***rhs, const char **abs_node, int *anode_cost, int **transl));
int yaep_parse(struct grammar *grammar,
  int (*read_token) (void **attr),
  void (*syntax_error) (int err_tok_num, void *err_tok_attr, int start_ignored_tok_num, void *start_ignored_tok_attr, int start_recovered_tok_num, void *start_recovered_tok_attr),
  void *(*parse_alloc) (int nmemb),
  void (*parse_free) (void *mem),
  struct yaep_tree_node **root,
  int *ambiguous_p);
void yaep_free_tree(struct yaep_tree_node * root, void (*parse_free) (void *), void (*termcb) (struct yaep_term * term));
```

Step 3 — High-level Python wrapper design
-----------------------------------------
Location: `python/src/yaep_python/__init__.py` or split across modules under `yaep_python/`

Requirements:
- Validate inputs early with clear exceptions and messages.
- Provide str API that accepts `str` and encodes to UTF-8; provide bytes API for raw bytes.
- Convert C strings to Python str using UTF-8 decoding; return None for NULL pointers.
- For parse() (the streaming API): accept any iterable of ints. Provide an adapter that turns it into a C callback while keeping callback alive.

Error handling policy
- Map documented YAEP error codes to Python exceptions where meaningful (e.g., invalid UTF-8 -> YaepInvalidUTF8Error).
- For non-mapped error codes, return the integer and provide error_message(); optionally provide YaepParseError with .code attribute.

Type hints
- Use type annotations on all public functions and classes.
- Provide `.pyi` stubs under `python/src/yaep_python/` if complex CFFI objects need to be typed for consumers.

Step 4 — Tests
--------------
Location: `python/tests/`

Test matrix (minimum):
- Unit tests for _cffi module (mocked where appropriate):
  - test_create_free_grammar
  - test_parse_description_success
  - test_parse_description_bytes_invalid_utf8 -> expect YaepInvalidUTF8Error
  - test_error_message_and_error_code
- Integration tests (require YAEP built and libs available):
  - test_end_to_end_parse_simple_grammar (grammar, token stream -> correct parse/return codes)
  - test_parse_with_tokens_callback (validate read_token wiring and ambiguous flag)
  - test_free_tree_and_context_manager (ensure no leaks/double frees)
- Property/fuzz tests (optional but recommended):
  - fuzz small malformed UTF-8 inputs to ensure we consistently report YAEP_INVALID_UTF8

Testing rules
- Each public function/class must have at least one unit test and one integration test where applicable.
- Tests must be deterministic and avoid reliance on external network or environment state.
- Use `pytest` and keep tests fast; integration tests that are slow should be marked and run in CI with appropriate tags.

Step 5 — Documentation (Sphinx, literate style)
----------------------------------------------
Location: `python/docs/`

Requirements:
- Use Sphinx with the napoleon extension (numPy / Google style docstrings) OR ReST with literal blocks for examples.
- Each public class/function must have a docstring that includes:
  - Description
  - Parameters and return types
  - Exceptions raised
  - Small example code block (that doubles as a doctest where sensible)
- Provide `python/docs/api/*.rst` that uses `automodule`/`autoclass` and includes additional narrative and diagrams.
- Literate tests: embed usage examples in tests with extended comments so they can be lifted into docs (use Sphinx doctest or include test snippets as examples in .rst files).

Docs build and tooling
- Sphinx conf: use `sphinx.ext.autodoc`, `sphinx.ext.napoleon`, and `sphinx.ext.viewcode`.
- Provide `make html` target in `python/docs/Makefile` and CI step to build docs as part of the pipeline.

Step 6 — Examples & Visualization
--------------------------------
Location: `python/examples/`

Examples to include:
- `basic_parse.py` — create Grammar, parse a small grammar description, and print the success/error.
- `visualize_parse_tree.py` — parse a small input and emit a Graphviz dot file and optionally render a PNG/SVG (use `graphviz` Python package).
- `notebook_demo.ipynb` — short notebook showing parse and visualization (optional).

Visualization guidance
- Convert YAEP's parse tree into a simple Python-native structure (nested dicts) before visualizing.
- Use `graphviz.Digraph` to build node labels with node types and token/term codes.
- Provide a helper `to_dot(parse_tree) -> str` and CLI entrypoint to write files.

Step 7 — Packaging and CI
-------------------------
Packaging:
- Provide `pyproject.toml` (already present) with metadata and extras (`dev = ["pytest", "cffi"]`).
- Support `pip install -e python` for development.

CI pipeline (GitHub Actions recommended):
1. Checkout repo
2. Configure matrix with OS (ubuntu-latest) and Python versions (3.8, 3.10, 3.12)
3. Install build deps (cmake, compiler) and build YAEP as shared libs (out-of-tree build to `build/`)
4. Setup Python venv, pip install -e python[dev]
5. Run unit tests (pytest)
6. Build docs (sphinx) and optionally upload to pages or as artifact
7. (Optional) Run mypy and linting

Sample GH action job sketch (documented in this file so LLM agents can reproduce):
- name: Build and test
  runs-on: ubuntu-latest
  steps:
    - uses: actions/checkout@v4
    - uses: actions/setup-python@v4
      with: {python-version: 3.10}
    - run: sudo apt-get update && sudo apt-get install -y cmake build-essential graphviz
    - run: mkdir -p build && cd build && cmake .. && cmake --build .
    - run: python -m pip install -e python[dev]
    - run: PYTHONPATH=python/src YAEP_LIB_PATH=build/src/libyaep.so python -m pytest -q

Step 8 — Quality gates and metrics
---------------------------------
- Tests: All unit tests and integration tests must pass in CI for merge.
- Typechecking: run mypy in CI (start with ignore-missing-imports=True, then tighten).
- Lint: run black/isort/flake8; set project pre-commit hooks.
- Coverage: aim for >80% coverage for Python wrapper code. Use coverage.py in CI to enforce threshold.

Step 9 — Migration and future-proofing notes
-------------------------------------------
- Avoid exposing CFFI internals directly to consumers — provide thin stable wrappers.
- If later moving to a C-extension (C API or pybind11) for performance, keep the public API stable to avoid breaking users.
- When adding features that require callbacks into Python from C, always document reentrancy and thread-safety. Use GIL management patterns in CFFI callbacks if needed.

Concrete file-by-file TODO checklist
----------------------------------
- python/src/yaep_python/_cffi.py
  - [ ] Ensure `cdef` matches `src/yaep.h` prototypes required by the wrapper.
  - [ ] Implement dynamic loader with YAEP_LIB_PATH override.
  - [ ] Implement parse_with_tokens helper and keep callbacks alive until parse returns.
  - [ ] Implement free_tree wrapper.
  - [ ] Unit tests for each helper.

- python/src/yaep_python/__init__.py (or small submodules)
  - [ ] Implement `Grammar` with context manager and full method set.
  - [ ] Implement exception hierarchy and mapping from YAEP error codes to exceptions.
  - [ ] Implement `ParseTree` and `ParseNode` with context manager and safe free.
  - [ ] Type hints for all public APIs and provide `py.typed` marker.
  - [ ] Unit tests for behavior and type contract.

- python/tests/
  - [ ] Unit tests for all public methods including error paths.
  - [ ] Integration tests requiring built YAEP lib (mark appropriately if slow).
  - [ ] Fuzz tests for UTF-8 validator (optional but recommended).

- python/docs/
  - [ ] Sphinx conf.py with napoleon and autodoc configured.
  - [ ] API rst files per module with examples included.

- python/examples/
  - [ ] basic_parse.py
  - [ ] visualize_parse_tree.py (Graphviz output)
  - [ ] optional notebook

- CI
  - [ ] Add GitHub Actions workflow for build & test & docs & mypy & lint.

Developer workflow notes (commands)
----------------------------------
- Build YAEP (out-of-tree):
```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```
- Run Python tests locally (from repo root):
```bash
export PYTHONPATH=python/src
export YAEP_LIB_PATH=build/src/libyaep.so
/path/to/venv/bin/python -m pytest python/tests -q
```
- Build docs locally:
```bash
cd python/docs && make html
```

Literate doc style and examples
------------------------------
- For every public method include a short example in the docstring that is also a valid doctest snippet where feasible.
- Keep longer usage narratives in `python/docs/examples.rst` and include `.. literalinclude::` references to example scripts so docs and code are the same source.

Guidelines for LLM coding agents (copy these into prompts)
---------------------------------------------------------
When an LLM is asked to modify or extend the wrapper, follow these rules:

1. Always re-check `src/yaep.h` for authoritative signatures before adding or changing a CFFI `cdef`.
2. Never commit 'TODO' placeholders for public APIs. If a feature is incomplete, instead:
   - Add a clear entry to `python/WRAPPER_IMPLEMENTATION_CHECKLIST.md` with a description and expected signature.
   - Leave the codebase in a build/test-clean state.
3. Add unit tests first for any new public behavior. Implement the functionality only after tests are written (red/green/refactor).
4. For bindings that involve callbacks or memory ownership, write integration tests exercising the real library; mocks are acceptable for unit tests but must not be the only tests.
5. Add Sphinx-friendly docstrings (Google or NumPy style) to every public class and function. Add longer rationales and examples to `python/docs/`.
6. If adding visualizations, ensure `graphviz` is an optional extra in `pyproject.toml` and the example scripts check for its availability and fail gracefully.

Final notes
-----------
This checklist is intentionally prescriptive to reduce ambiguity for human and LLM contributors. Follow it strictly to maintain code quality and long-term maintainability. Update this file (by PR) when you add new public APIs or change ownership semantics so future contributors (human or AI) get a correct, single source of truth.
