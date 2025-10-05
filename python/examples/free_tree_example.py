#!/usr/bin/env python3
"""Example demonstrating yaep_free_tree with termcb.

This example shows how to use ParseTree.free_with_termcb() to free parse trees
with a custom callback for cleaning up terminal node attributes.

The termcb callback is called for each terminal node in the tree, allowing
custom cleanup of attributes attached to tokens (e.g., freeing memory for
complex token data).

.. note::
   In this simple example, tokens have no attributes, so the callback is dummy.
   In real usage with attributed tokens (via full_parse), termcb would free
   the attribute memory.
"""

import os
import sys

# Path setup
this_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.abspath(os.path.join(this_dir, '..', 'src'))
if local_src not in sys.path:
    sys.path.insert(0, local_src)

from yaep_python import Grammar


def dummy_termcb(term_ptr):
    """Dummy callback for freeing term attributes.

    In real usage, this would free memory pointed to by term_ptr.val.attr.
    Since we don't set attributes in this example, it just prints.
    """
    print(f"Term callback called for terminal code: {term_ptr.val.term.code}")


def main():
    """Main function demonstrating free_with_termcb."""
    g = Grammar()

    desc = "TERM;\nS : 'a' 'b' ;\n"
    rc = g.parse_description(desc, strict=True)
    if rc != 0:
        print(f'Failed to parse grammar: {g.error_message()}')
        g.free()
        return 1

    tokens = [ord('a'), ord('b')]
    rc2, tree, syntax_err = g.parse(tokens)
    if rc2 != 0 or not tree:
        print(f'Parse failed: rc={rc2}')
        g.free()
        return 1

    print("Parse successful, now freeing with termcb...")
    tree.free_with_termcb(dummy_termcb)
    print("Tree freed with custom term callback.")

    g.free()
    return 0


if __name__ == '__main__':
    sys.exit(main())