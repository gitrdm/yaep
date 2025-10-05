#!/usr/bin/env python3
"""Example demonstrating yaep_set_debug_level.

This example sets the debug level for parsing. Debug levels control the amount of
tracing output produced during parsing operations.

- Level 0: No debug output (default)
- Higher levels: Increasing detail of parser operations

.. note::
   Debug output is only available if YAEP was compiled with debug support
   (typically not enabled in release builds). In this example, setting the level
   demonstrates the API, but output may not be visible depending on the build.
"""

import os
import sys

# Path setup
this_dir = os.path.abspath(os.path.dirname(__file__))
local_src = os.path.abspath(os.path.join(this_dir, '..', 'src'))
if local_src not in sys.path:
    sys.path.insert(0, local_src)

from yaep_python import Grammar


def main():
    """Main function demonstrating debug level setting."""
    g = Grammar()

    # Set debug level to 1 (some debug output)
    prev_level = g.set_debug_level(1)
    print(f"Previous debug level: {prev_level}")
    print(f"Current debug level: {g.set_debug_level(1)}")  # Should return 1

    # Parse a simple grammar to potentially trigger debug output
    desc = "TERM;\nS : 'a' ;\n"
    rc = g.parse_description(desc, strict=True)
    if rc != 0:
        print('Failed to parse grammar:', g.error_message())
        return 1

    print("Parsing with debug level set...")
    rc2, tree, syntax_err = g.parse([ord('a')])
    print(f'Parse rc: {rc2}')
    print("Check console output for debug information (if compiled with debug support)")

    g.free()
    return 0


if __name__ == '__main__':
    sys.exit(main())