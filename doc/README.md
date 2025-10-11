# YAEP Documentation

This directory contains documentation for the YAEP (Yet Another Earley Parser) project.

## Documentation Files

### API Documentation (Markdown)

- **`YAEP_C_API.md`** - Complete C interface API reference
- **`YAEP_CPP_API.md`** - Complete C++ interface API reference

These are hand-written, tutorial-style guides for using the YAEP API.

### Legacy Documentation

- **`yaep.txt`** - Plain text C API documentation (legacy format)
- **`yaep++.txt`** - Plain text C++ API documentation (legacy format)
- **`yaep.html`** - HTML index for C documentation
- **`yaep++.html`** - HTML index for C++ documentation
- **`Internals.txt`** - Internal implementation details

### Auto-Generated Documentation (Doxygen)

Run `doxygen Doxyfile` from the project root to generate comprehensive code documentation:

```bash
cd /path/to/yaep
doxygen Doxyfile
xdg-open doc/doxygen/html/index.html
```

The generated documentation provides:
- Complete function reference
- Data structure documentation
- Call graphs and dependency diagrams
- Source code browser
- Cross-referenced symbols

**Note:** The `doc/doxygen/` directory is not tracked in git (it's regenerated from source).

## Which Documentation Should I Use?

- **Learning the API?** → Start with `YAEP_C_API.md` or `YAEP_CPP_API.md`
- **Quick reference?** → Use the Markdown files or legacy `.txt` files
- **Working on the code?** → Generate Doxygen documentation
- **Need implementation details?** → Check `Internals.txt` and Doxygen

## Generating Documentation

### Doxygen HTML

```bash
doxygen Doxyfile
```

### Markdown to HTML (optional)

```bash
pandoc YAEP_C_API.md -s -o YAEP_C_API.html
pandoc YAEP_CPP_API.md -s -o YAEP_CPP_API.html
```

### Markdown to PDF (optional)

```bash
pandoc YAEP_C_API.md -o YAEP_C_API.pdf
pandoc YAEP_CPP_API.md -o YAEP_CPP_API.pdf
```

## Documentation Formats

| Format | Purpose | Source | Generated |
|--------|---------|--------|-----------|
| Markdown (`.md`) | User API guide | Hand-written | No |
| Doxygen HTML | Code reference | Source comments | Yes |
| Plain text (`.txt`) | Legacy format | Hand-written | No |
| HTML (`.html`) | Legacy format | Hand-written | No |

## Installation

Documentation files are installed by CMake to `${CMAKE_INSTALL_DOCDIR}` (typically `/usr/local/share/doc/yaep`).

```bash
cmake --install build --prefix /usr/local
```

Installed files:
- `Internals.txt`
- `YAEP_C_API.md`
- `YAEP_CPP_API.md`
- `yaep.html`, `yaep.txt`
- `yaep++.html`, `yaep++.txt`
