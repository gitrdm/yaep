from . import _cffi
from typing import Iterable, Optional, Tuple, Any, List, Dict, Callable

# NOTE: The current YAEP C library does not have UTF-8 validation support.
# UTF-8 validation was previously implemented but was removed due to segfaults
# and memory leaks. Error codes only go up to YAEP_INVALID_TOKEN_CODE (17).
# The YAEP_INVALID_UTF8 error code (18) and YaepInvalidUTF8Error exception
# have been removed to match the actual C library capabilities.


class YaepError(Exception):
	"""Base class for YAEP-related exceptions."""


class ParseNode:
	"""Lightweight wrapper for a yaep_tree_node pointer."""
	def __init__(self, ptr: Any):
		self._ptr = ptr

	@property
	def type(self) -> int:
		return int(self._ptr.type)

	def is_term(self) -> bool:
		return self.type == 2  # YAEP_TERM

	def term_code(self) -> Optional[int]:
		if self.is_term():
			return int(self._ptr.val.term.code)
		return None


class ParseTree:
	def __init__(self, root_ptr: Any, ambiguous: bool):
		self._root_ptr = root_ptr
		self.root = ParseNode(root_ptr) if root_ptr != _cffi._ffi.NULL else None
		self.ambiguous = bool(ambiguous)

	def free(self) -> None:
		if hasattr(self, '_root_ptr') and self._root_ptr and self._root_ptr != _cffi._ffi.NULL:
			_cffi.free_tree(self._root_ptr)
			self._root_ptr = None

	def free_with_termcb(self, termcb: Optional[Callable[[Any], None]] = None) -> None:
		"""Free the parse tree with optional term callback for freeing term attributes.

		termcb: callable that takes a struct yaep_term * cdata, for freeing attr.
		"""
		if hasattr(self, '_root_ptr') and self._root_ptr and self._root_ptr != _cffi._ffi.NULL:
			_cffi.free_tree(self._root_ptr, termcb=termcb)
			self._root_ptr = None

	def __enter__(self) -> 'ParseTree':
		return self

	def __exit__(self, exc_type, exc, tb) -> None:
		self.free()

	def __del__(self):
		try:
			self.free()
		except Exception:
			pass


class Grammar:
	"""Thin wrapper around YAEP grammar pointer with deterministic cleanup."""
	def __init__(self):
		self._g = _cffi.create_grammar()
		if self._g == 0 or self._g is None:
			raise MemoryError('yaep_create_grammar failed')

	def __enter__(self) -> 'Grammar':
		return self

	def __exit__(self, exc_type, exc, tb) -> None:
		self.free()

	def parse_description(self, description: str, strict: bool = True) -> int:
		return int(_cffi.parse_grammar(self._g, strict, description))

	def parse_description_bytes(self, buf: bytes, strict: bool = True) -> int:
		"""Parse a grammar description from raw bytes.
		
		Note: The current YAEP C library does not perform UTF-8 validation.
		Invalid byte sequences will be passed through to the parser which may
		treat them as regular characters. Previous versions of YAEP had UTF-8
		validation but it was removed due to bugs.
		"""
		return int(_cffi.parse_grammar_bytes(self._g, strict, buf))

	def parse(self, tokens: Iterable[int]) -> Tuple[int, Optional[ParseTree], Optional[Tuple[int, int, int]]]:
		"""Parse a stream of integer token codes.

		Returns (rc, ParseTree|None, syntax_error_info|None).
		syntax_error_info is (err_tok_num, start_ignored_tok_num, start_recovered_tok_num) if syntax error occurred, else None.
		"""
		rc, root_ptr, ambiguous, syntax_err = _cffi.parse_with_tokens(self._g, tokens)
		tree = ParseTree(root_ptr, ambiguous) if root_ptr != _cffi._ffi.NULL else None
		syntax_info = syntax_err['info'] if syntax_err['called'] else None
		return int(rc), tree, syntax_info

	def full_parse(self, read_token: Callable[[], Tuple[int, Optional[Any]]], syntax_error: Optional[Callable[[int, Any, int, Any, int, Any], None]] = None, parse_alloc: Optional[Callable[[int], Any]] = None, parse_free: Optional[Callable[[Any], None]] = None) -> Tuple[int, Optional[ParseTree]]:
		"""Parse with full YAEP callbacks.

		read_token: () -> (token_code: int, attr: cdata or None)
		syntax_error: (err_tok_num, err_tok_attr, start_ignored_tok_num, start_ignored_tok_attr, start_recovered_tok_num, start_recovered_tok_attr) -> None
		parse_alloc: (nmemb: int) -> cdata
		parse_free: (mem: cdata) -> None

		Returns (rc: int, tree: ParseTree or None)
		"""
		rc, root_ptr, ambiguous = _cffi.parse_full(self._g, read_token, syntax_error, parse_alloc, parse_free)
		tree = ParseTree(root_ptr, ambiguous) if root_ptr != _cffi._ffi.NULL else None
		return int(rc), tree

	def error_code(self) -> int:
		return int(_cffi.error_code(self._g))

	def error_message(self) -> Optional[str]:
		return _cffi.error_message(self._g)

	def free(self) -> None:
		if self._g:
			_cffi.free_grammar(self._g)
			self._g = None

	# convenience setters mirroring public API
	def set_lookahead_level(self, level: int) -> int:
		return int(_cffi.set_lookahead_level(self._g, level))

	def set_debug_level(self, level: int) -> int:
		return int(_cffi.set_debug_level(self._g, level))

	def set_one_parse_flag(self, flag: int) -> int:
		return int(_cffi.set_one_parse_flag(self._g, flag))

	def set_cost_flag(self, flag: int) -> int:
		return int(_cffi.set_cost_flag(self._g, flag))

	def set_error_recovery_flag(self, flag: int) -> int:
		return int(_cffi.set_error_recovery_flag(self._g, flag))

	def set_recovery_match(self, n_toks: int) -> int:
		return int(_cffi.set_recovery_match(self._g, n_toks))

	def read_grammar_from_lists(self, terminals: List[Tuple[str, Optional[int]]], rules: List[Dict], strict: bool = True) -> int:
		"""Read grammar from Python lists of terminals and rules.

		terminals: [(name, code?), ...] where code is int or None (auto-assign).
		rules: [{'lhs': str, 'rhs': [str], 'abs_node': str?, 'anode_cost': int?, 'transl': [int]?}, ...]
		Returns YAEP return code.
		"""
		return int(_cffi.read_grammar_from_lists(self._g, strict, terminals, rules))

__all__ = ["Grammar", "ParseTree", "ParseNode"]
