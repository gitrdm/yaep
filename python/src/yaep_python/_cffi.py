import os
from cffi import FFI

_ffi = FFI()
# Minimal declarations for the functions we need
_ffi.cdef("""
typedef struct grammar grammar;
grammar *yaep_create_grammar(void);
int yaep_parse_grammar(grammar *g, int strict_p, const char *description);
void yaep_free_grammar(grammar *g);
int yaep_error_code(struct grammar *g);
const char *yaep_error_message(struct grammar *g);
int yaep_set_lookahead_level(struct grammar *grammar, int level);
int yaep_set_debug_level(struct grammar *grammar, int level);
int yaep_set_one_parse_flag(struct grammar *grammar, int flag);
int yaep_set_cost_flag(struct grammar *grammar, int flag);
int yaep_set_error_recovery_flag(struct grammar *grammar, int flag);
int yaep_set_recovery_match(struct grammar *grammar, int n_toks);

/* Parse API with callbacks and tree structures (simplified declarations). */
enum yaep_tree_node_type { YAEP_NIL, YAEP_ERROR, YAEP_TERM, YAEP_ANODE, YAEP_ALT };

struct yaep_term {
    int code;
    void *attr;
};

struct yaep_anode {
    const char *name;
    int cost;
    struct yaep_tree_node **children;
};

struct yaep_alt {
    struct yaep_tree_node *node;
    struct yaep_tree_node *next;
};

struct yaep_tree_node {
    enum yaep_tree_node_type type;
    union {
        struct { int used; } nil;
        struct { int used; } error;
        struct yaep_term term;
        struct yaep_anode anode;
        struct yaep_alt alt;
    } val;
};

int yaep_parse(struct grammar *grammar,
                             int (*read_token) (void **attr),
                             void (*syntax_error) (int, void *, int, void *, int, void *),
                             void *(*parse_alloc) (int nmemb),
                             void (*parse_free) (void *mem),
                             struct yaep_tree_node **root,
                             int *ambiguous_p);

void yaep_free_tree(struct yaep_tree_node * root, void (*parse_free) (void *), void (*termcb) (struct yaep_term * term));

int yaep_read_grammar(struct grammar *g, int strict_p,
                      const char *(*read_terminal) (int *code),
                      const char *(*read_rule) (const char ***rhs,
                                                const char **abs_node,
                                                int *anode_cost,
                                                int **transl));
""")

def _find_lib():
    # Allow override for testing
    override = os.environ.get('YAEP_LIB_PATH')
    if override:
        if os.path.exists(override):
            return override
        raise RuntimeError(f'YAEP_LIB_PATH set but file not found: {override}')

    # repo root is three levels up from this file: python/src/yaep_python -> repo root
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
    build_dir = os.path.join(repo_root, 'build')
    if not os.path.isdir(build_dir):
        # also accept an out-of-tree build named 'build' in repo root
        raise RuntimeError(f'build directory not found at expected location: {build_dir}')

    for subdir, dirs, files in os.walk(build_dir):
        for f in files:
            if f.startswith('libyaep') and (f.endswith('.so') or f.endswith('.dylib') or f.endswith('.dll')):
                return os.path.join(subdir, f)

    raise RuntimeError(f'libyaep not found under build/ (looked in {build_dir})')

_lib_path = _find_lib()
_lib = _ffi.dlopen(_lib_path)

class YaepLib:
    def __init__(self):
        self._lib = _lib

def create_grammar():
    return _lib.yaep_create_grammar()

def parse_grammar(g, strict_p, description: str):
    b = description.encode('utf-8')
    c = _ffi.new("char[]", b)
    return _lib.yaep_parse_grammar(g, int(strict_p), c)


def parse_grammar_bytes(g, strict_p, buf: bytes):
    # Pass raw bytes directly to YAEP without any re-encoding.
    if not isinstance(buf, (bytes, bytearray)):
        raise TypeError('buf must be bytes')
    c = _ffi.new("char[]", buf)
    return _lib.yaep_parse_grammar(g, int(strict_p), c)


def parse_with_tokens(grammar_ptr, token_iterable):
    """Call yaep_parse with a Python iterable of token integers.

    Returns (root_ptr, ambiguous, syntax_error_info)
    """
    it = iter(token_iterable)

    @_ffi.callback("int(void **)" )
    def read_token(attr_ptr):
        try:
            tok = next(it)
        except StopIteration:
            return -1
        # attr is not used; set to NULL
        attr_ptr[0] = _ffi.NULL
        return int(tok)

    syntax_err = {'called': False, 'info': None}

    @_ffi.callback("void(int, void *, int, void *, int, void *)")
    def syntax_error(err_tok_num, err_tok_attr, start_ignored_tok_num, start_ignored_tok_attr, start_recovered_tok_num, start_recovered_tok_attr):
        syntax_err['called'] = True
        syntax_err['info'] = (int(err_tok_num), start_ignored_tok_num, start_recovered_tok_num)

    root_ptr = _ffi.new("struct yaep_tree_node **")
    ambiguous_p = _ffi.new("int *")

    rc = _lib.yaep_parse(grammar_ptr, read_token, syntax_error, _ffi.NULL, _ffi.NULL, root_ptr, ambiguous_p)

    # Return root pointer (may be NULL), ambiguous flag, and syntax error info
    return int(rc), root_ptr[0], int(ambiguous_p[0]), syntax_err


def free_tree(root_ptr, parse_free=_ffi.NULL, termcb=None):
    """Free a parse tree, with optional parse_free and termcb."""
    if root_ptr == _ffi.NULL or root_ptr is None:
        return
    c_termcb = _ffi.NULL
    if termcb:
        @_ffi.callback("void(struct yaep_term *)")
        def c_termcb(term_ptr):
            termcb(term_ptr)
    _lib.yaep_free_tree(root_ptr, parse_free, c_termcb)

def free_grammar(g):
    _lib.yaep_free_grammar(g)

def error_code(g):
    return int(_lib.yaep_error_code(g))

def error_message(g):
    p = _lib.yaep_error_message(g)
    if p == _ffi.NULL:
        return None
    return _ffi.string(p).decode('utf-8')

def set_lookahead_level(g, level):
    return int(_lib.yaep_set_lookahead_level(g, int(level)))


def set_debug_level(g, level):
    return int(_lib.yaep_set_debug_level(g, int(level)))


def set_one_parse_flag(g, flag):
    return int(_lib.yaep_set_one_parse_flag(g, int(flag)))


def set_cost_flag(g, flag):
    return int(_lib.yaep_set_cost_flag(g, int(flag)))


def set_error_recovery_flag(g, flag):
    return int(_lib.yaep_set_error_recovery_flag(g, int(flag)))


def set_recovery_match(g, n_toks):
    return int(_lib.yaep_set_recovery_match(g, int(n_toks)))

def read_grammar_from_lists(grammar_ptr, strict_p, terminals, rules):
    """Call yaep_read_grammar with Python lists for terminals and rules.

    terminals: list of (name: str, code: int?) tuples. If code is None, YAEP auto-assigns.
    rules: list of dicts with keys:
        'lhs': str (nonterminal)
        'rhs': list[str] (symbols)
    Returns the YAEP return code (int).
    """
    # State to keep C arrays alive during callbacks
    # CRITICAL: We must keep all C string allocations alive until yaep_read_grammar
    # completes, otherwise YAEP will access freed memory (use-after-free bug).
    # The 'keepalive' list ensures strings persist for the entire call.
    state = {
        'terminals': terminals,
        'rules': rules,
        'term_idx': 0,
        'rule_idx': 0,
        'keepalive': []  # Keep C strings alive
    }

    @_ffi.callback("const char *(int *)")
    def read_terminal(code_ptr):
        if state['term_idx'] >= len(state['terminals']):
            return _ffi.NULL
        name, code = state['terminals'][state['term_idx']]
        state['term_idx'] += 1
        if code is not None:
            code_ptr[0] = int(code)
        else:
            code_ptr[0] = -1  # Let YAEP assign
        
        # Keep the C string alive by storing it in keepalive list
        name_c = _ffi.new("char[]", name.encode('utf-8'))
        state['keepalive'].append(name_c)
        return name_c

    @_ffi.callback("const char *(const char ***, const char **, int *, int **)")  
    def read_rule(rhs_ptr, abs_node_ptr, anode_cost_ptr, transl_ptr):
        if state['rule_idx'] >= len(state['rules']):
            return _ffi.NULL
        rule = state['rules'][state['rule_idx']]
        state['rule_idx'] += 1

        # RHS: array of strings - keep all C strings alive
        rhs = rule['rhs']
        rhs_c = [_ffi.new("char[]", s.encode('utf-8')) for s in rhs]
        state['keepalive'].extend(rhs_c)  # Keep alive
        rhs_c.append(_ffi.NULL)  # End marker
        rhs_array = _ffi.new("const char *[]", rhs_c)
        state['keepalive'].append(rhs_array)  # Keep alive
        rhs_ptr[0] = rhs_array

        # abs_node - keep C string alive
        abs_node = rule.get('abs_node')
        if abs_node:
            abs_node_c = _ffi.new("char[]", abs_node.encode('utf-8'))
            state['keepalive'].append(abs_node_c)  # Keep alive
            abs_node_ptr[0] = _ffi.cast("const char *", abs_node_c)
        else:
            abs_node_ptr[0] = _ffi.NULL

        # anode_cost
        cost = rule.get('anode_cost', 1)  # Default 1
        anode_cost_ptr[0] = int(cost)

        # transl: array of ints, end with negative
        transl = rule.get('transl')
        if transl is None:
            transl_ptr[0] = _ffi.NULL
        else:
            transl_c = [int(x) for x in transl] + [-1]  # End marker
            transl_array = _ffi.new("int[]", transl_c)
            state['keepalive'].append(transl_array)  # Keep alive
            transl_ptr[0] = transl_array

        # LHS - keep C string alive
        lhs_c = _ffi.new("char[]", rule['lhs'].encode('utf-8'))
        state['keepalive'].append(lhs_c)  # Keep alive
        return lhs_c

    return int(_lib.yaep_read_grammar(grammar_ptr, int(strict_p), read_terminal, read_rule))

def parse_full(grammar_ptr, read_token, syntax_error, parse_alloc, parse_free):
    """Call yaep_parse with full Python callbacks."""
    @_ffi.callback("int(void **)")
    def c_read_token(attr_ptr):
        tok, attr = read_token()
        attr_ptr[0] = attr if attr is not None else _ffi.NULL
        return int(tok)

    c_syntax_error = _ffi.NULL
    if syntax_error:
        @_ffi.callback("void(int, void *, int, void *, int, void *)")
        def c_syntax_error(err_tok_num, err_tok_attr, start_ignored_tok_num, start_ignored_tok_attr, start_recovered_tok_num, start_recovered_tok_attr):
            syntax_error(err_tok_num, err_tok_attr, start_ignored_tok_num, start_ignored_tok_attr, start_recovered_tok_num, start_recovered_tok_attr)

    c_parse_alloc = _ffi.NULL
    if parse_alloc:
        @_ffi.callback("void *(int)")
        def c_parse_alloc(nmemb):
            return parse_alloc(nmemb)

    c_parse_free = _ffi.NULL
    if parse_free:
        @_ffi.callback("void(void *)")
        def c_parse_free(mem):
            parse_free(mem)

    root_ptr = _ffi.new("struct yaep_tree_node **")
    ambiguous_p = _ffi.new("int *")
    rc = _lib.yaep_parse(grammar_ptr, c_read_token, c_syntax_error, c_parse_alloc, c_parse_free, root_ptr, ambiguous_p)
    return int(rc), root_ptr[0], int(ambiguous_p[0])

