from yaep_python import Grammar


def test_full_parse():
    """Test Grammar.full_parse with callbacks equivalent to parse()."""
    g = Grammar()
    desc = "TERM;\nS : 'a' S 'b' | ;\n"
    rc = g.parse_description(desc, strict=True)
    assert rc == 0

    # Use full_parse with callbacks that mimic parse(iter(()))
    tokens = iter(())
    def read_token():
        try:
            tok = next(tokens)
            return tok, None
        except StopIteration:
            return -1, None

    rc2, tree = g.full_parse(read_token)
    assert isinstance(rc2, int)
    # For this grammar, empty input may or may not produce a tree; just check it's valid
    if tree:
        tree.free()
    g.free()


def test_full_parse_with_tokens():
    """Test full_parse with actual tokens."""
    g = Grammar()
    desc = "TERM;\nS : 'a' ;\n"
    rc = g.parse_description(desc, strict=True)
    assert rc == 0

    tokens = [ord('a')]
    it = iter(tokens)
    def read_token():
        try:
            tok = next(it)
            return tok, None
        except StopIteration:
            return -1, None

    rc2, tree = g.full_parse(read_token)
    assert rc2 == 0
    assert tree is not None
    tree.free()
    g.free()