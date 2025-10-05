from yaep_python import Grammar
from yaep_python.tree_utils import to_ast


def test_test07_ast_shape():
    g = Grammar()
    desc = (
        "\n"
        "TERM;\n"
        "E : T         # 0\n"
        "  | E '+' T   # plus (0 2)\n"
        "  ;\n"
        "T : F         # 0\n"
        "  | T '*' F   # mult (0 2)\n"
        "  ;\n"
        "F : 'a'       # 0\n"
        "  | '(' E ')' # 1\n"
        "  ;\n"
    )
    rc = g.parse_description(desc, strict=True)
    assert rc == 0

    input_text = "a+a*(a*a+a)"
    rc2, tree, syntax_err = g.parse(iter([ord(c) for c in input_text]))
    assert rc2 == 0
    assert syntax_err is None
    ast = to_ast(tree)
    # Top-level should be an Anode named 'plus'
    assert ast is not None
    assert getattr(ast, 'name', None) == 'plus'
    # The top-level plus should have two children (left operand and right operand)
    assert len(getattr(ast, 'children', [])) == 2
    g.free()
