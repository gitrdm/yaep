import ctypes
import os

# Locate shared lib built in the build tree
build_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build'))
lib_path = None
for root, dirs, files in os.walk(build_dir):
    for f in files:
        if f.startswith('libyaep') and (f.endswith('.so') or f.endswith('.dylib') or f.endswith('.dll')):
            lib_path = os.path.join(root, f)
            break
    if lib_path:
        break

if not lib_path:
    raise SystemExit('Shared library libyaep not found in build directory')

print('Using library:', lib_path)
lib = ctypes.CDLL(lib_path)

# Define opaque grammar pointer
class Grammar(ctypes.Structure):
    pass

lib.yaep_create_grammar.restype = ctypes.POINTER(Grammar)
lib.yaep_free_grammar.argtypes = [ctypes.POINTER(Grammar)]
lib.yaep_parse_grammar.argtypes = [ctypes.POINTER(Grammar), ctypes.c_int, ctypes.c_char_p]
lib.yaep_parse_grammar.restype = ctypes.c_int

# Smoke test: create grammar, parse trivial grammar description, free grammar
g = lib.yaep_create_grammar()
if not g:
    raise SystemExit('yaep_create_grammar returned NULL')

desc = b"S : 'a' ;\n"
ret = lib.yaep_parse_grammar(g, 1, desc)
print('yaep_parse_grammar returned', ret)

lib.yaep_free_grammar(g)
print('OK')
