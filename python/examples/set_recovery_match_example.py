#!/usr/bin/env python3
"""Example demonstrating yaep_set_recovery_match.

This example shows how to set the recovery match count for error recovery.
The recovery match specifies how many consecutive tokens must be successfully
parsed after an error before considering recovery complete.

Higher values require more correct input before resuming normal parsing,
making recovery more conservative.

.. note::
   This works in conjunction with error recovery. The example uses invalid
   input to trigger recovery and shows how the setting affects behavior.
"""

import os
import sys

# Path setup
this_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.abspath(os.path.join(this_dir, '..', 'src'))
if local_src not in sys.path:
    sys.path.insert(0, local_src)

from yaep_python import Grammar


def parse_with_recovery_match(match_count):
    """Parse with given recovery match count."""
    g = Grammar()
    g.set_error_recovery_flag(1)  # Enable recovery
    g.set_recovery_match(match_count)

    desc = "TERM;\nS : 'a' S 'b' | ;\n"
    rc = g.parse_description(desc, strict=True)
    if rc != 0:
        print(f'Failed to parse grammar: {g.error_message()}')
        g.free()
        return

    # Invalid input: 'a' followed by invalid, then valid sequence
    tokens = [ord('a'), ord('x'), ord('a'), ord('b')]  # 'x' is invalid
    rc2, tree, syntax_err = g.parse(tokens)
    print(f'Recovery match {match_count}: rc={rc2}, tree={tree is not None}, syntax_err={syntax_err}')
    if syntax_err:
        print(f'  Syntax error details: {syntax_err}')

    g.free()


def main():
    """Main function demonstrating recovery match setting."""
    print("Testing recovery match setting...")

    print("\nWith recovery match 1:")
    parse_with_recovery_match(1)

    print("\nWith recovery match 3:")
    parse_with_recovery_match(3)

    return 0


if __name__ == '__main__':
    sys.exit(main())