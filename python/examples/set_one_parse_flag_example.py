#!/usr/bin/env python3
"""Example demonstrating yaep_set_one_parse_flag.

This example shows how to control whether the parser builds one parse tree
or multiple trees for ambiguous grammars.

- Flag = 1 (default): Build only one parse tree (faster, less memory)
- Flag = 0: Build all possible parse trees for ambiguous grammars

For unambiguous grammars, this setting has no effect.

.. note::
   Ambiguity detection is indicated by the 'ambiguous' flag in ParseTree.
   This example uses a simple grammar where ambiguity is unlikely, but
   demonstrates the API usage for grammars that may be ambiguous.
"""

import os
import sys

# Path setup
this_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.abspath(os.path.join(this_dir, '..', 'src'))
if local_src not in sys.path:
    sys.path.insert(0, local_src)

from yaep_python import Grammar


def parse_with_one_parse_flag(flag):
    """Parse with given one_parse_flag."""
    g = Grammar()
    g.set_one_parse_flag(flag)

    desc = "TERM;\nS : 'a' | S S ;\n"  # Potentially ambiguous
    rc = g.parse_description(desc, strict=True)
    if rc != 0:
        print(f'Failed to parse grammar: {g.error_message()}')
        g.free()
        return

    tokens = [ord('a'), ord('a')]
    rc2, tree, syntax_err = g.parse(tokens)
    ambiguous = tree.ambiguous if tree else False
    print(f'One parse flag {flag}: rc={rc2}, tree={tree is not None}, ambiguous={ambiguous}')

    g.free()


def main():
    """Main function demonstrating one parse flag."""
    print("Testing one parse flag...")

    print("\nWith one parse flag = 1 (single tree):")
    parse_with_one_parse_flag(1)

    print("\nWith one parse flag = 0 (all trees for ambiguous):")
    parse_with_one_parse_flag(0)

    return 0


if __name__ == '__main__':
    sys.exit(main())