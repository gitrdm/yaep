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


def free_tree(root_ptr):
    """Free a parse tree previously returned by yaep_parse.

    This calls yaep_free_tree(root, NULL, NULL) which is correct when
    the parser was invoked with default PARSE_ALLOC/PARSE_FREE (NULL).
    """
    if root_ptr == _ffi.NULL or root_ptr is None:
        return
    _lib.yaep_free_tree(root_ptr, _ffi.NULL, _ffi.NULL)

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

