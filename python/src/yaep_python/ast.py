from dataclasses import dataclass
from typing import List, Optional, Union


@dataclass
class Node:
    """Base class for AST nodes converted from YAEP parse trees."""


@dataclass
class Term(Node):
    code: int
    char: Optional[str] = None


@dataclass
class Anode(Node):
    name: Optional[str]
    cost: int
    children: List[Node]


ASTNode = Union[Term, Anode]
