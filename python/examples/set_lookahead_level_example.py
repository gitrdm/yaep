#!/usr/bin/env python3
"""Example demonstrating yaep_set_lookahead_level.

This example shows how to set the lookahead level for parsing.
Lookahead level 1 uses static lookaheads (default, good balance of speed and space),
while level 2 uses dynamic lookaheads (slightly slower but can handle more complex ambiguities).

In this simple example, the effect is not visible in the output, but it affects internal parser behavior.
For grammars with potential ambiguities, higher lookahead levels can improve accuracy.

.. note::
   The actual impact depends on the grammar complexity. This example uses a basic arithmetic grammar
   where the difference is minimal, but demonstrates the API usage.
"""

import os
import sys

# Path setup
this_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.abspath(os.path.join(this_dir, '..', 'src'))
if local_src not in sys.path:
    sys.path.insert(0, local_src)

from yaep_python import Grammar
from yaep_python.tree_utils import to_dict


def main():
    """Main function demonstrating lookahead level setting."""
    g = Grammar()

    # Set lookahead level to 2 (dynamic)
    prev_level = g.set_lookahead_level(2)
    print(f"Previous lookahead level: {prev_level}")
    print(f"Current lookahead level: {g.set_lookahead_level(2)}")  # Should return 2

    # Define a simple grammar
    desc = (
        "TERM;\n"
        "E : T         # 0\n"
        "  | E '+' T   # plus (0 2)\n"
        "  ;\n"
        "T : 'a'       # 0\n"
        "  ;\n"
    )
    rc = g.parse_description(desc, strict=True)
    if rc != 0:
        print('Failed to parse grammar:', g.error_message())
        return 1

    # Parse input
    input_text = "a+a"
    tokens = [ord(c) for c in input_text]
    rc2, tree, syntax_err = g.parse(tokens)
    print(f'Parse rc: {rc2}')
    if tree:
        d = to_dict(tree)
        print('Parse tree:', d)
    else:
        print('No parse tree')

    g.free()
    return 0


if __name__ == '__main__':
    sys.exit(main())