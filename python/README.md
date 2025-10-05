YAEP Python prototype bindings

This directory contains a small prototype Python wrapper around the YAEP C library.

Quickstart

- Build YAEP (create an out-of-tree build):

```bash
mkdir -p build && cd build
cmake .. && cmake --build .
```

- Install dev deps in the repo venv (the workspace has a .venv already configured):

```bash
/home/rdmerrio/gits/yaep/.venv/bin/python -m pip install -e python[dev]
```

- Run tests (from repo root):

```bash
export PYTHONPATH=python/src
export YAEP_LIB_PATH=build/src/libyaep.so
/home/rdmerrio/gits/yaep/.venv/bin/python -m pytest -q python/tests
```

Notes

- The wrapper uses cffi to dynamically load the YAEP shared library. Set `YAEP_LIB_PATH` if you built the shared lib to a non-default location.
- This is a prototype: for production packaging, add proper wheels, CI, typing, and richer API surface.

str vs bytes
-----------

Use `parse_description()` when you have a normal Python `str` (Unicode text). The wrapper
will UTF-8 encode the string before calling YAEP.

Use `parse_description_bytes()` when you have raw bytes that you want passed unchanged to
YAEP (for example, to reproduce malformed inputs or to validate YAEP's UTF-8 validator).

Example (invalid UTF-8 test):

```py
from yaep_python import Grammar
from yaep_python import YaepInvalidUTF8Error

g = Grammar()
try:
	g.parse_description_bytes(b"TERM;\nS : 'a' S 'b' | ;\n\xC3")
except YaepInvalidUTF8Error as e:
	print('YAEP rejected input:', e)
finally:
	g.free()
```
