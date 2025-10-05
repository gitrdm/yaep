from yaep_python import Grammar


def test_setters_bindings():
    g = Grammar()
    # Call each setter to ensure it is callable and returns an int (previous value)
    prev = g.set_lookahead_level(1)
    assert isinstance(prev, int)
    prev = g.set_debug_level(0)
    assert isinstance(prev, int)
    prev = g.set_one_parse_flag(1)
    assert isinstance(prev, int)
    prev = g.set_cost_flag(0)
    assert isinstance(prev, int)
    prev = g.set_error_recovery_flag(1)
    assert isinstance(prev, int)
    prev = g.set_recovery_match(3)
    assert isinstance(prev, int)
    g.free()
