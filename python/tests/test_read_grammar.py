from yaep_python import Grammar
from yaep_python.tree_utils import to_ast


def test_read_grammar_from_lists():
    g = Grammar()
    # Simple grammar: E -> 'a' (with translation)
    terminals = [('a', 97)]  # 'a' with code 97
    rules = [
        {'lhs': 'E', 'rhs': ['a'], 'abs_node': 'test', 'anode_cost': 1, 'transl': []}
    ]
    rc = g.read_grammar_from_lists(terminals, rules, strict=True)
    assert rc == 0

    # Parse input 'a'
    rc2, tree, syntax_err = g.parse([97])  # ord('a')
    assert rc2 == 0
    assert syntax_err is None
    ast = to_ast(tree)
    assert ast is not None
    assert ast.name == 'test'
    assert len(ast.children) == 0
    g.free()