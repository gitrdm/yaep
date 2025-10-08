from yaep_python import Grammar
from yaep_python.tree_utils import to_dict


def test_to_dict_on_empty_parse():
    g = Grammar()
    desc = "TERM;\nS : 'a' S 'b' | ;\n"
    rc = g.parse_description(desc, strict=True)
    assert rc == 0

    rc2, tree, syntax_err = g.parse(iter(()))
    # tree may be None depending on grammar and inputs; ensure function handles None
    d = to_dict(tree)
    assert d is None or isinstance(d, dict)
    g.free()
