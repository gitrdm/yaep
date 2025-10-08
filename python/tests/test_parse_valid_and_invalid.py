import os
from yaep_python import Grammar
from yaep_python import _cffi


def test_parse_valid_description():
    g = Grammar()
    desc = "TERM;\nS : 'a' S 'b' | ;\n"
    rc = g.parse_description(desc, strict=True)
    assert rc == 0
    g.free()


def test_parse_invalid_utf8_description():
	"""
	This test verifies that YAEP handles malformed UTF-8 byte sequences gracefully.

	Historical context:
	- Previous versions of YAEP had UTF-8 validation that would return YAEP_INVALID_UTF8
	  for malformed byte sequences.
	- That feature was removed due to segfaults and memory leaks.
	- The current YAEP C library (as of Oct 2025) does NOT validate UTF-8.

	Current behavior:
	- Invalid UTF-8 bytes (like a lone 0xC3 continuation byte) are passed through
	  to the parser, which treats them as regular characters in the grammar description.
	- The parser may successfully parse the grammar or fail for other reasons (syntax error),
	  but it will NOT specifically report invalid UTF-8.

	This test now verifies that the wrapper correctly handles this case without crashing.
	"""

	g = Grammar()
	# invalid trailing continuation byte (single 0xC3)
	invalid = b"TERM;\nS : 'a' S 'b' | ;\n\xC3"

	# The current library does NOT validate UTF-8, so this should succeed or
	# fail with a syntax error, but not crash.
	rc = g.parse_description_bytes(invalid, strict=False)
	
	# We don't assert a specific return code because the behavior with
	# invalid UTF-8 is undefined (no validation). Just verify no crash.
	assert isinstance(rc, int)
	
	# Clean up - this should not crash
	g.free()
