YAEP Python example: visualize_parse_tree

This directory contains a small example `visualize_parse_tree.py` that
parses a compact expression grammar (taken from `test/C/test07.c`) and
prints a JSON-like dict and Graphviz DOT source for the YAEP-produced
translation (parse tree).

What the example shows
- How YAEP translation annotations (`# ...`) map into parse-tree nodes.
  - `# plus (0 2)` produces an ANODE named "plus" whose children are the
    translations of the RHS symbols at indexes 0 and 2.
  - `# 0` forwards the translation of a RHS symbol.
- How to feed YAEP a token stream from Python: for character-literal
  terminals, pass `ord(c)` for each character in the input string.
- How to convert a YAEP parse tree into a Python dict (`to_dict`) and into
  Graphviz DOT (`to_dot`) for visualization.

Render DOT -> PNG
-----------------
You can render the DOT output produced by the example into a PNG using
Graphviz's `dot` tool:

1. Run the example and capture DOT into a file:

```bash
python python/examples/visualize_parse_tree.py > /tmp/tree.dot
```

2. Render to PNG with `dot`:

```bash
dot -Tpng -o /tmp/tree.png /tmp/tree.dot
```

3. Open `/tmp/tree.png` with your preferred image viewer.

Notes
- The example prints both a JSON-friendly dict (TERM nodes include
  `code` and, when printable, `char`) and the DOT source that labels TERM
  nodes using the printable character and numeric code.
- If you do not have the `graphviz` Python package installed, `to_dot()`
  will be unavailable; in that case the example falls back to printing
  the Python dict only.

