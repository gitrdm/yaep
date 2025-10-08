#!/usr/bin/env python3
"""Example: build a grammar programmatically using read_grammar_from_lists.

This demonstrates yaep_read_grammar via Python lists, avoiding string parsing.
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
    g = Grammar()
    # Define terminals: (name, code) or (name, None) for auto-assign
    terminals = [
        ('a', 97),  # 'a' with explicit code
        ('b', 98),  # 'b' with explicit code
    ]
    # Define rules: each is a dict with lhs, rhs, abs_node?, anode_cost?, transl?
    rules = [
        # E -> 'a' 'b' with translation #concat(0,1)
        {'lhs': 'E', 'rhs': ['a', 'b'], 'abs_node': 'concat', 'anode_cost': 1, 'transl': [0, 1]},
    ]
    rc = g.read_grammar_from_lists(terminals, rules, strict=True)
    if rc != 0:
        print('Failed to read grammar:', g.error_message())
        return 1

    # Parse "ab"
    tokens = [97, 98]  # ord('a'), ord('b')
    rc2, tree, syntax_err = g.parse(tokens)
    print('Parse rc:', rc2)
    if tree:
        d = to_dict(tree)
        print('Parse tree dict:', d)
    g.free()
    return 0


if __name__ == '__main__':
    sys.exit(main())