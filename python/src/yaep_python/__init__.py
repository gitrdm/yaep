from . import _cffi
from typing import Iterable, Optional, Tuple, Any

# YAEP public error codes (a few are mirrored here for convenience)
YAEP_INVALID_UTF8 = 18


class YaepError(Exception):
	"""Base class for YAEP-related exceptions."""


class YaepInvalidUTF8Error(YaepError):
	"""Raised when YAEP reports invalid UTF-8 in input bytes."""
	pass


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
		rc = int(_cffi.parse_grammar_bytes(self._g, strict, buf))
		if rc == YAEP_INVALID_UTF8:
			# Raise a richer Python exception with YAEP's diagnostic message
			raise YaepInvalidUTF8Error(self.error_message() or "YAEP_INVALID_UTF8")
		return rc

	def parse(self, tokens: Iterable[int]) -> Tuple[int, Optional[ParseTree]]:
		rc, root_ptr, ambiguous, syntax_err = _cffi.parse_with_tokens(self._g, tokens)
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

__all__ = ["Grammar", "ParseTree", "ParseNode"]
