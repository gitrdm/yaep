from yaep_python import Grammar


def test_free_with_termcb():
    """Test ParseTree.free_with_termcb with dummy termcb."""
    g = Grammar()
    desc = "TERM;\nS : 'a' ;\n"
    rc = g.parse_description(desc, strict=True)
    assert rc == 0

    rc2, tree, syntax_err = g.parse([ord('a')])
    assert rc2 == 0
    assert syntax_err is None
    assert tree is not None

    # Use free_with_termcb with a dummy callback
    def dummy_termcb(term_ptr):
        pass  # In real use, free term_ptr.val.attr if set

    tree.free_with_termcb(dummy_termcb)
    g.free()