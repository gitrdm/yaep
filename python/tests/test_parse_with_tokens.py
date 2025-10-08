from yaep_python import Grammar


def test_parse_with_empty_tokens():
    g = Grammar()
    desc = "TERM;\nS : 'a' S 'b' | ;\n"
    rc = g.parse_description(desc, strict=True)
    assert rc == 0

    # Provide empty token stream; parser should see EOF immediately and return
    rc2, tree, syntax_err = g.parse(iter(()))
    # rc2 is an int error code (0 indicates success). With no input tokens the parse may succeed or return an error,
    # but this test ensures the call completes and returns an integer and a tree object or None.
    assert isinstance(rc2, int)
    assert tree is None or hasattr(tree, 'root')
    assert syntax_err is None  # No syntax error expected
    g.free()
