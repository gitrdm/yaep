"""Utilities to convert YAEP parse trees into Python-native structures and Graphviz DOT.

This module provides two convenience functions:

- to_dict(parse_tree: yaep_python.ParseTree) -> dict | None
    Convert a YAEP parse tree (a `ParseTree` wrapper or a raw `yaep_tree_node *`
    pointer) into a nested Python dictionary. The returned structure is easy to
    inspect, serialize (JSON) and manipulate from Python code.

- to_dot(parse_tree: yaep_python.ParseTree) -> str
    Produce a Graphviz DOT source string representing the parse tree. The DOT
    output is intended for quick visual inspection; it is not a fully featured
    renderer but is sufficient to render a PNG/SVG with the `dot` tool.

Dict shape and translation/terminals semantics
----------------------------------------------
YAEP builds translations for grammar alternatives according to optional
translation annotations (the parts that start with `#` in a grammar
description). Only alternatives with an explicit translation produce a
non-NIL translation node. Typical translation kinds you will encounter here
are:

- TERM nodes: translations for terminals. In the Python dict they look like:
        {'type': 'TERM', 'code': <int>, 'char': <str>?}

    - `code` is the integer token code reported by YAEP (for character
        literals this is the integer character code, e.g. 97 for 'a').
    - `char` is present when `code` maps to a printable character; it is the
        single-character Python string for convenience and is not guaranteed to
        be present for non-printable or out-of-range codes.

- ANODE nodes: abstract nodes created by grammar translations such as
    `#plus(0,2)`. They have the shape:
        {'type': 'ANODE', 'name': <str or None>, 'cost': <int>, 'children': [ ... ]}

    - `name` is the annotation identifier (e.g. "plus"). If YAEP created an
        anonymous or unnamed node this field may be None.
    - `cost` is the translation cost attached by the grammar (default 1 when
        omitted in the grammar). The cost is preserved in the dict for debugging
        and disambiguation scenarios.
    - `children` is a list of child node dicts (TERM, ANODE, ALT, etc.).

- ALT nodes: used when YAEP produces an alternatives chain. They look like:
        {'type': 'ALT', 'alts': [ <node dict>, ... ]}

Notes about presentation and to_dot()
------------------------------------
- `to_dict()` is deterministic and preserves all translation structure YAEP
    produced. It does not attempt to collapse or simplify nodes; that is left
    to callers who want a higher-level AST.

- `to_dot()` produces a readable Graphviz representation and attempts to
    display printable characters for TERM nodes (for example "'a' (code=97)")
    while still including the numeric code to avoid ambiguity. Use `to_dict()`
    if you need programmatic access to codes and characters (e.g. for tests).

Security and ownership note
---------------------------
The functions in this module expect the YAEP parse tree to remain alive
while they traverse it. When you receive a `ParseTree` wrapper from the
high-level API prefer using it as a context manager (or call its `.free()`
method) only after you have converted it to Python structures with
`to_dict()`/`to_dot()`; otherwise traversal will touch freed memory.

"""
from typing import Any, Dict, List, Optional
from . import _cffi
from . import ast as _ast


def _node_to_dict(node_ptr: Any) -> Optional[Dict]:
    if node_ptr == _cffi._ffi.NULL or node_ptr is None:
        return None

    t = int(node_ptr.type)
    # YAEP enum mapping: 0 NIL, 1 ERROR, 2 TERM, 3 ANODE, 4 ALT
    if t == 0:  # NIL
        return {'type': 'NIL'}
    if t == 1:  # ERROR
        return {'type': 'ERROR'}
    if t == 2:  # TERM
        term = node_ptr.val.term
        code = int(term.code)
        node = {'type': 'TERM', 'code': code}
        try:
            ch = chr(code)
            if ch.isprintable():
                node['char'] = ch
        except Exception:
            pass
        return node
    if t == 3:  # ANODE
        an = node_ptr.val.anode
        name = None
        if an.name != _cffi._ffi.NULL:
            name = _cffi._ffi.string(an.name).decode('utf-8', errors='replace')
        children = []
        if an.children != _cffi._ffi.NULL:
            i = 0
            while True:
                child = an.children[i]
                if child == _cffi._ffi.NULL:
                    break
                children.append(_node_to_dict(child))
                i += 1
        return {'type': 'ANODE', 'name': name, 'cost': int(an.cost), 'children': children}
    if t == 4:  # ALT (alternatives chain)
        # iterate alternatives linked list
        alts = []
        cur = node_ptr.val.alt.node
        while cur != _cffi._ffi.NULL:
            alts.append(_node_to_dict(cur))
            # move to next alternative
            next_ptr = node_ptr.val.alt.next
            if next_ptr == _cffi._ffi.NULL:
                break
            node_ptr = next_ptr
            cur = node_ptr.val.alt.node
        return {'type': 'ALT', 'alts': alts}

    return {'type': 'UNKNOWN', 'raw_type': t}


def to_dict(parse_tree: Any) -> Optional[Dict]:
    """Convert a ParseTree (or its root pointer) to a nested Python dict.

    parse_tree: either a ParseTree instance or a raw yaep_tree_node * pointer.
    Returns None if the tree is empty.
    """
    root_ptr = None
    if hasattr(parse_tree, '_root_ptr'):
        root_ptr = getattr(parse_tree, '_root_ptr')
    else:
        root_ptr = parse_tree

    if root_ptr == _cffi._ffi.NULL or root_ptr is None:
        return None

    return _node_to_dict(root_ptr)


def to_ast(parse_tree: Any) -> Optional[_ast.ASTNode]:
    """Convert a YAEP parse tree to the simple Python dataclasses in
    `yaep_python.ast`. Returns None for an empty/NIL tree."""

    root_ptr = None
    if hasattr(parse_tree, '_root_ptr'):
        root_ptr = getattr(parse_tree, '_root_ptr')
    else:
        root_ptr = parse_tree

    if root_ptr == _cffi._ffi.NULL or root_ptr is None:
        return None

    def node_to_ast(node_ptr):
        if node_ptr == _cffi._ffi.NULL or node_ptr is None:
            return None
        t = int(node_ptr.type)
        if t == 2:  # TERM
            term = node_ptr.val.term
            code = int(term.code)
            ch = None
            try:
                c = chr(code)
                if c.isprintable():
                    ch = c
            except Exception:
                pass
            return _ast.Term(code=code, char=ch)
        if t == 3:  # ANODE
            an = node_ptr.val.anode
            name = None
            if an.name != _cffi._ffi.NULL:
                name = _cffi._ffi.string(an.name).decode('utf-8', errors='replace')
            children = []
            if an.children != _cffi._ffi.NULL:
                i = 0
                while True:
                    child = an.children[i]
                    if child == _cffi._ffi.NULL:
                        break
                    children.append(node_to_ast(child))
                    i += 1
            return _ast.Anode(name=name, cost=int(an.cost), children=children)
        # For ALT and others, fall back to None for now
        return None

    return node_to_ast(root_ptr)


def to_dot(parse_tree: Any) -> str:
    """Produce a Graphviz dot string for the parse tree.

    The function creates simple node labels indicating type and term code or anode name.
    """
    import graphviz

    d = to_dict(parse_tree)
    if d is None:
        return ''

    dot = graphviz.Digraph(comment='YAEP parse tree')

    counter = {'id': 0}

    def add_node(node):
        nid = f'n{counter["id"]}'
        counter['id'] += 1
        t = node.get('type')
        label = t
        if t == 'TERM':
            code = node.get('code')
            try:
                # Prefer showing a printable character when possible.
                ch = chr(int(code))
                if ch.isprintable():
                    # Use repr to get a quoted, escaped representation like '\'a\''
                    label += f"\\n{repr(ch)} (code={code})"
                else:
                    label += f"\\ncode={code}"
            except Exception:
                label += f"\\ncode={code}"
        if t == 'ANODE':
            label += f"\\n{node.get('name')}"
        dot.node(nid, label)
        return nid

    def walk(node, parent_id=None):
        nid = add_node(node)
        if parent_id is not None:
            dot.edge(parent_id, nid)
        if node['type'] == 'ANODE':
            for c in node.get('children', []):
                if c is not None:
                    walk(c, nid)
        if node['type'] == 'ALT':
            for a in node.get('alts', []):
                if a is not None:
                    walk(a, nid)

    walk(d, None)
    return dot.source
