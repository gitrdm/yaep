# Doxygen Integration Guide

## Overview

Doxygen has been integrated into the YAEP project to provide automatic code documentation extraction. Doxygen generates comprehensive HTML documentation from the existing code comments in the C/C++ source files.

---

## What is Doxygen?

**Doxygen** is a documentation generator that creates HTML, LaTeX, and other formats from annotated source code. It can extract documentation from:
- Function and method comments
- Structure and class definitions  
- Macro definitions
- Variable declarations
- File-level documentation

Unlike the hand-written SGML/Markdown API documentation, Doxygen **automatically extracts** documentation from source code comments.

---

## Documentation Strategy

The YAEP project now has **two complementary documentation systems**:

### 1. **Hand-Written API Documentation** (Markdown)
   - **Files**: `doc/YAEP_C_API.md`, `doc/YAEP_CPP_API.md`
   - **Purpose**: High-level API guide for users
   - **Content**: Tutorial-style, organized by topic
   - **Target**: Users learning the API
   - **Format**: Markdown (easy to read on GitHub)

### 2. **Auto-Generated Code Documentation** (Doxygen)
   - **Files**: Generated in `doc/doxygen/html/`
   - **Purpose**: Comprehensive reference for all code
   - **Content**: Every function, struct, macro, variable
   - **Target**: Developers and contributors
   - **Format**: HTML with cross-references, call graphs

**Why Both?**
- **Markdown docs**: Tell you HOW to use the API (tutorial)
- **Doxygen docs**: Tell you WHAT every function does (reference)

---

## Files Added

### Configuration
- **`Doxyfile`** (2,863 lines) - Doxygen configuration file

### Generated Output (NOT in git)
- **`doc/doxygen/html/`** - Generated HTML documentation (~21MB, 2,128 files)
  - `index.html` - Main entry point
  - Per-file documentation
  - Call graphs
  - Dependency graphs
  - Source code with syntax highlighting

---

## Doxygen Configuration

The `Doxyfile` is configured with these key settings:

```ini
PROJECT_NAME           = "YAEP (Yet Another Earley Parser)"
PROJECT_NUMBER         = "1.0.0"
PROJECT_BRIEF          = "Fast Earley parser with error recovery..."
OUTPUT_DIRECTORY       = doc/doxygen
INPUT                  = src README.md
RECURSIVE              = YES
EXTRACT_ALL            = YES
GENERATE_LATEX         = NO
USE_MDFILE_AS_MAINPAGE = README.md
```

**Key Features Enabled:**
- `EXTRACT_ALL = YES` - Document all code, even without Doxygen comments
- `RECURSIVE = YES` - Process all subdirectories
- `USE_MDFILE_AS_MAINPAGE` - Use README.md as the landing page
- Graphs enabled - Shows function call graphs and dependencies
- Source browsing enabled - View annotated source code

---

## Generating Documentation

### Manual Generation

```bash
# Generate HTML documentation
doxygen Doxyfile

# View the documentation
xdg-open doc/doxygen/html/index.html
```

### Clean Documentation

```bash
# Remove generated documentation
rm -rf doc/doxygen/
```

---

## What Doxygen Extracts

Even though the YAEP code uses **standard C comments** (not Doxygen-style `/**` comments), Doxygen can still extract:

### From `src/yaep.h`:
- All public API functions with descriptions
- All data structures (structs, enums)
- All macros and constants
- Error code definitions
- Function signatures and parameters

### From `src/yaep.c`:
- Internal implementation details
- Static functions
- Helper functions
- Data structures
- Call graphs showing function dependencies

### Example Extraction

**Source code:**
```c
/* The following function creates undefined grammar.  The function
   returns NULL if there is no memory.  This function should be called
   the first. */
extern struct grammar *yaep_create_grammar (void);
```

**Doxygen generates:**
- Function signature with syntax highlighting
- Function description
- Return value documentation
- Cross-references to related functions
- List of functions that call this
- Source code location link

---

## Documentation Statistics

| Metric | Value |
|--------|-------|
| Generated HTML files | 2,128 |
| Total documentation size | ~21 MB |
| Documented files | ~40 source files |
| Documented functions | ~200+ |
| Documented structures | ~20+ |
| Documented macros | ~30+ |

---

## Git Integration

### `.gitignore` Entry

```gitignore
# Doxygen generated documentation
doc/doxygen/
```

The generated documentation is **NOT committed** to git because:
- It's large (~21MB)
- It's auto-generated (reproducible)
- It changes with every code change
- Users can generate it locally with `doxygen Doxyfile`

### Tracked Files
- `Doxyfile` - Configuration file (tracked)
- `doc/doxygen/` - Generated output (ignored)

---

## Improving Doxygen Output

The current code uses standard C comments, which work fine. However, you can enhance the documentation by adding Doxygen-style comments:

### Current Style (works):
```c
/* The following function creates undefined grammar.
   Returns NULL if there is no memory. */
extern struct grammar *yaep_create_grammar (void);
```

### Enhanced Doxygen Style (better):
```c
/**
 * @brief Creates a new parser grammar
 * 
 * Creates an undefined grammar for a new YAEP parser instance.
 * You can use multiple parsers simultaneously.
 * 
 * @return Pointer to newly created grammar, or NULL if out of memory
 * @note This function should be called first
 * @see yaep_free_grammar()
 */
extern struct grammar *yaep_create_grammar (void);
```

**Benefits of Doxygen-style comments:**
- `@param` tags document parameters
- `@return` tags document return values
- `@brief` provides one-line summary
- `@see` creates cross-references
- `@note`, `@warning` add special sections

**Current Status:** Not required! Doxygen works well with existing comments.

---

## Viewing the Documentation

### Local Viewing

```bash
# Generate and view
doxygen Doxyfile
xdg-open doc/doxygen/html/index.html

# Or use a web server
cd doc/doxygen/html
python3 -m http.server 8000
# Then visit: http://localhost:8000
```

### Publishing Options

You could publish the Doxygen HTML to:
1. **GitHub Pages** - Host as a static website
2. **ReadTheDocs** - Integrate with git
3. **Project website** - Include in downloads
4. **CI/CD** - Auto-generate on every commit

Example GitHub Pages workflow:
```yaml
# .github/workflows/doxygen.yml
name: Doxygen
on: [push]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install Doxygen
        run: sudo apt-get install doxygen graphviz
      - name: Generate docs
        run: doxygen Doxyfile
      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./doc/doxygen/html
```

---

## Integration with CMake (Optional)

You can add a CMake target to generate Doxygen documentation:

```cmake
# Add to doc/CMakeLists.txt
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating Doxygen documentation"
        VERBATIM
    )
    message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")
    message(STATUS "Run 'make doxygen' to generate API documentation")
else()
    message(STATUS "Doxygen not found, documentation generation disabled")
endif()
```

Then users can run:
```bash
cmake --build build --target doxygen
```

---

## Comparison: Markdown vs Doxygen

| Aspect | Markdown API Docs | Doxygen Code Docs |
|--------|-------------------|-------------------|
| **Source** | Hand-written | Auto-extracted |
| **Content** | High-level API guide | Complete code reference |
| **Organization** | By topic/concept | By file/function |
| **Format** | Markdown | HTML (+ LaTeX, PDF) |
| **Size** | 40KB (2 files) | 21MB (2,128 files) |
| **Maintenance** | Manual editing | Regenerate from code |
| **Target Audience** | API users | Developers/contributors |
| **Examples** | Tutorial-style | From code comments |
| **Cross-references** | Manual links | Automatic |
| **Call graphs** | No | Yes |
| **Source browsing** | No | Yes |
| **Version Control** | Track in git | Ignore (regenerate) |

**Recommendation:** Keep both!
- **Markdown** for users learning the API
- **Doxygen** for developers working on the code

---

## Benefits of Doxygen Integration

### For Contributors
1. **Complete reference**: Every function documented
2. **Call graphs**: Understand code flow visually
3. **Cross-references**: Click to navigate dependencies
4. **Source browsing**: Read annotated source code
5. **Up-to-date**: Regenerate after any change

### For Maintainers
1. **No manual work**: Documentation extracted automatically
2. **Consistency**: Same format for all code
3. **Completeness**: Won't miss undocumented functions
4. **Graphs**: Visualize complex relationships

### For Users
1. **Searchable**: Find any function quickly
2. **Complete**: Every function has signature
3. **Examples**: See function usage in source
4. **Dependencies**: Understand what calls what

---

## Current Status

✅ **Doxygen configured and tested**
- Configuration file created: `Doxyfile`
- Documentation successfully generated
- Output directory added to `.gitignore`
- 2,128 HTML files generated (~21MB)
- All source files documented

⚠️ **Not Yet Done:**
- CMake integration (optional)
- Enhanced Doxygen-style comments (optional)
- GitHub Pages deployment (optional)
- CI/CD auto-generation (optional)

---

## Usage Summary

```bash
# Generate documentation
doxygen Doxyfile

# View documentation
xdg-open doc/doxygen/html/index.html

# Clean documentation
rm -rf doc/doxygen/

# Check configuration
doxygen -g Doxyfile.new  # Generate new config to compare
```

---

## Recommendations

1. **Keep Doxygen generation as-is**
   - Works great with existing code comments
   - No need to rewrite all comments in Doxygen style
   
2. **Add CMake target** (optional but nice)
   - Makes it easy: `make doxygen`
   - Documents availability in build output
   
3. **Consider GitHub Pages** (future enhancement)
   - Auto-publish documentation on every commit
   - Users can browse online without building
   
4. **Enhance new code** (recommended)
   - Use Doxygen-style comments for new functions
   - Adds `@param`, `@return`, `@brief` tags
   - Improves generated documentation quality

---

## Examples of Generated Documentation

### Main Page
- README.md content
- Project overview
- Links to modules

### File Documentation
- File description
- Include dependency graphs
- List of functions, structs, macros
- Source code with line numbers

### Function Documentation  
- Function signature
- Description from comments
- Parameters (if documented)
- Return value (if documented)
- Caller/callee graphs
- Link to source code

### Structure Documentation
- Member list with types
- Member descriptions
- Usage examples

### Macro Documentation
- Definition
- Description
- Value

---

## Conclusion

Doxygen integration provides comprehensive **code-level documentation** that complements the hand-written **API-level documentation** in Markdown. Together, they give users both:
- **What** every function does (Doxygen)
- **How** to use the API (Markdown)

The integration is complete and ready to use. Documentation can be regenerated at any time with a single command: `doxygen Doxyfile`.
