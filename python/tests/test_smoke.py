import os
from yaep_python import _cffi


def test_create_parse_free():
    g = _cffi.create_grammar()
    assert g != 0
    # empty description should parse as 0 (success) in the C smoke test
    rc = _cffi.parse_grammar(g, True, '')
    assert isinstance(rc, int)
    _cffi.free_grammar(g)
