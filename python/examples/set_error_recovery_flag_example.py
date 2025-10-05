#!/usr/bin/env python3
"""Example demonstrating yaep_set_error_recovery_flag.

This example shows the effect of error recovery during parsing.
When error recovery is enabled (default), the parser attempts to continue
parsing after syntax errors by skipping invalid tokens.
When disabled, parsing stops immediately on the first error.

.. note::
   Error recovery makes parsers more robust for interactive input or
   incomplete data, but may produce incorrect parse trees. This example
   uses invalid input to demonstrate the difference.
"""

import os
import sys

# Path setup
this_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.abspath(os.path.join(this_dir, '..', 'src'))
if local_src not in sys.path:
    sys.path.insert(0, local_src)

from yaep_python import Grammar


def parse_with_recovery_flag(flag):
    """Parse with given error recovery flag."""
    g = Grammar()
    g.set_error_recovery_flag(flag)

    desc = "TERM;\nS : 'a' S 'b' | ;\n"
    rc = g.parse_description(desc, strict=True)
    if rc != 0:
        print(f'Failed to parse grammar: {g.error_message()}')
        g.free()
        return

    # Invalid input: missing 'b'
    tokens = [ord('a')]
    rc2, tree, syntax_err = g.parse(tokens)
    print(f'Error recovery flag {flag}: rc={rc2}, tree={tree is not None}, syntax_err={syntax_err}')
    if syntax_err:
        print(f'  Syntax error details: {syntax_err}')

    g.free()


def main():
    """Main function demonstrating error recovery flag."""
    print("Testing error recovery flag...")

    print("\nWith error recovery enabled (flag=1):")
    parse_with_recovery_flag(1)

    print("\nWith error recovery disabled (flag=0):")
    parse_with_recovery_flag(0)

    return 0


if __name__ == '__main__':
    sys.exit(main())