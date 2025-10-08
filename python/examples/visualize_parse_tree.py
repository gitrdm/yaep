#!/usr/bin/env python3
"""Example: parse a simple grammar and emit Graphviz dot for the parse tree.

Requires `graphviz` package to render the dot source.
"""
import os
import sys

# Make the example runnable directly from the repo without requiring PYTHONPATH.
# If the package `yaep_python` isn't importable, prepend the local `python/src`
# directory to sys.path so that `import yaep_python` works when running this
# script from the repository checkout.
try:
    import yaep_python  # type: ignore
except Exception:
    this_dir = os.path.abspath(os.path.dirname(__file__))
    local_src = os.path.abspath(os.path.join(this_dir, '..', 'src'))
    if local_src not in sys.path:
        sys.path.insert(0, local_src)

from yaep_python import Grammar
from yaep_python.tree_utils import to_dot


def main():
    g = Grammar()
    # Use the grammar from `test/C/test07.c`.
    #
    # Explanation (why this grammar is chosen):
    # - It's a compact expression grammar that demonstrates both left
    #   recursion (E -> E '+' T) and binary operator translations.
    # - The grammar uses YAEP translation annotations (the parts that
    #   start with `#`). Those annotations tell YAEP what translation
    #   (AST) to produce for each alternative. Examples used below:
    #     - `# plus (0 2)` means: create an abstract node named "plus"
    #       and set its fields to the translations of the RHS symbols
    #       at indexes 0 and 2 (RHS indexes are zero-based). Those
    #       fields become the ANODE's children in the parse tree.
    #     - `# 0` means: the translation of the alternative is the
    #       translation of RHS symbol 0 (usually a TERM node for a
    #       terminal). If no translation annotation is present the
    #       translation is treated as NIL (no node produced).
    #
    # Token model and input:
    # - This example mirrors the C test "test07.c" which uses single-character
    #   terminals like 'a', '+' and '*'. YAEP represents terminals as
    #   integer token codes. For character literals the code is simply
    #   the character code (ord).
    # - We therefore feed YAEP with a stream of integer token codes
    #   produced from the Python string `input_text` using `ord()`.
    #
    # Output produced by this script:
    # - `to_dict(tree)` converts the YAEP parse tree into a nested
    #   Python dictionary. TERM nodes include both `code` and (when
    #   printable) `char` fields. ANODE nodes include `name`, `cost`,
    #   and `children`.
    # - `to_dot(tree)` renders the same tree as Graphviz DOT source
    #   for quick visualization; TERM nodes try to display the printable
    #   character (e.g. "'a' (code=97)") while still preserving the
    #   numeric code for unambiguous debugging.
    desc = (
        "\n"
        "TERM;\n"
        "E : T         # 0\n"
        "  | E '+' T   # plus (0 2)\n"
        "  ;\n"
        "T : F         # 0\n"
        "  | T '*' F   # mult (0 2)\n"
        "  ;\n"
        "F : 'a'       # 0\n"
        "  | '(' E ')' # 1\n"
        "  ;\n"
    )
    rc = g.parse_description(desc, strict=True)
    if rc != 0:
        print('Failed to parse grammar', g.error_message())
        return 1

    # For demonstration we run a parse that produces a real tree.
    # Use the same input as the C test driver:
    input_text = "a+a*(a*a+a)"
    # Tokenize by emitting the character code for each input character.
    tokens = iter([ord(c) for c in input_text])
    rc2, tree, syntax_err = g.parse(tokens)
    # Always show diagnostics. In many grammars the parse tree may be empty
    # (e.g., when the grammar reduces to NIL or when error recovery suppressed output).
    print('parse rc:', rc2)
    print('yaep error code:', g.error_code())
    print('yaep error message:', g.error_message())
    print('ambiguous:', bool(tree.ambiguous) if tree is not None else False)
    print('syntax error info:', syntax_err)

    # Try to print a Python dict representation even if the tree is empty.
    from yaep_python.tree_utils import to_dict
    d = to_dict(tree)
    if d is None:
        print('Parse tree (dict): None or empty')
    else:
        print('Parse tree (dict):')
        import json

        print(json.dumps(d, indent=2, ensure_ascii=False))

    try:
        dot = to_dot(tree)
        print(dot)
    except Exception as e:
        # to_dot requires optional `graphviz` package; if it's missing just print the dict
        from yaep_python.tree_utils import to_dict
        print('graphviz not available; print Python dict instead:')
        print(to_dict(tree))
    return 0


if __name__ == '__main__':
    sys.exit(main())
