# YAEP (Yet Another Earley Parser) - C Interface

This document describes YAEP (Yet Another Earley Parser) written in C/C++.

---

## YAEP

YAEP is an abbreviation of Yet Another Earley Parser. The package `YAEP` implements an Earley parser. The Earley parser implementation has the following features:

* **Fast and memory-efficient**: This is the fastest implementation of Earley parser which I know. The main design goal is to achieve speed and memory requirements which are necessary to use it in prototype compilers and language processors. It parses 30K lines of C program per second on 500 MHz Pentium III and allocates about 5MB memory for 10K line C program.

* **Syntax directed translation**: Simple syntax directed translation is performed, so an abstract tree is already the output of YAEP.

* **Ambiguous grammar support**: It can parse input described by an ambiguous grammar. In this case the parse result can be an abstract tree or all possible abstract trees. Moreover it produces the compact representation of all possible parse trees by using DAG instead of real trees. This feature can be used to parse natural language sentences.

* **Cost-based parsing**: It can parse input described by an ambiguous grammar according to the abstract node costs. In this case the parse result can be a minimal cost abstract tree or all possible minimal cost abstract trees. This feature can be used for code selection tasks in compilers.

* **Error recovery**: It can make syntax error recovery. Moreover its error recovery algorithms find error recovery with minimal number of ignored tokens. This permits implementing parsers with very good error recovery and reporting.

* **Fast startup**: There is no practically delay between processing grammar and start of parsing.

* **Flexible interface**: The input grammar can be given by YACC-like description or by providing functions returning terminals and rules.

* **Good debugging features**: It can print huge amount of information about grammar, parsing, error recovery, translation. You can even output the result translation in form for a graphic visualization program.

The interface part of the parser is file `yaep.h`. The implementation part is file `yaep.c`. The interface contains the following external definitions and macros:

---

## API Reference

### Data Types

#### `struct grammar`

Describes a grammar. Knowledge of structure (implementation) of this type is not visible and not needed for using the parser.

#### `enum yaep_tree_node_type`

Describes all possible nodes of abstract tree representing the translation. The following enumeration constants are defined:

* **`YAEP_NIL`** - the corresponding node represents empty translations.
* **`YAEP_ERROR`** - the corresponding node represents translation of special terminal `error` (see error recovery).
* **`YAEP_TERM`** - the corresponding node represents translation of a terminal.
* **`YAEP_ANODE`** - the corresponding node represents an abstract node.
* **`YAEP_ALT`** - the corresponding node represents an alternative of the translation. Such nodes are created only when there are two or more possible translations. It means that the grammar is ambiguous.

#### `struct yaep_tree_node`

Represents node of the translation. The nodes refer to each other forming DAG (directed acyclic graph) in general case. The main reason of generating DAG is that some input fragments may have the same translation, when there are several parsings of input (which is possible only for ambiguous grammars). But DAG may be created even for unambiguous grammar because some nodes (empty and error nodes) exist only in one exemplar. When such nodes are not created, the translation nodes form a tree.

This structure has the following members:

* **`type`** (type `enum yaep_tree_node_type`) - representing type of the translation node.
* **`val`** (union) - Depending on the translation node type, one of the union members `nil`, `error`, `term`, `anode`, and `alt` of the structure types described below is used to represent the translation node.

#### `struct yaep_nil`

Represents empty node. It has no members. Actually the translation is DAG (not tree) in general case. The empty and error nodes are present only in one exemplar.

#### `struct yaep_error`

Represents translation of special terminal `error`. It has no members. The error node exists only in one exemplar.

#### `struct yaep_term`

Represents translation of terminals. It has the following two members:

* **`code`** (int) - representing code of the corresponding terminal.
* **`attr`** (void *) - is reference for the attribute of the corresponding terminal.

#### `struct yaep_anode`

Represents abstract node. It has the following members:

* **`name`** (const char *) - representing name of anode as it given in the corresponding rule translation.
* **`cost`** (int) - representing cost of the node plus costs of all children if the cost flag is set up. Otherwise, the value is cost of the abstract node itself.
* **`children`** (struct yaep_tree_node **) - is array of nodes representing the translations of the symbols given in the rule with the abstract node.

#### `struct yaep_alt`

Represents an alternative of the translation. It has the following two members:

* **`node`** (struct yaep_tree_node *) - representing alternative translation.
* **`next`** (struct yaep_tree_node *) - is reference for the next alternative of translation.

---

### Error Codes

#### `YAEP_NIL_TRANSLATION_NUMBER`

Reserved to be designation of empty node for translation.

#### `YAEP_NO_MEMORY`

Error code of the parser. The parser functions return the code when parser cannot allocate enough memory for its work.

#### `YAEP_UNDEFINED_OR_BAD_GRAMMAR`

Error code of the parser. The parser functions return the code when we call parsing without defining grammar or call parsing for bad defined grammar.

#### `YAEP_DESCRIPTION_SYNTAX_ERROR_CODE`

Error code of the parser. The code is returned when the grammar is defined by description and there is syntax error in the description.

#### `YAEP_FIXED_NAME_USAGE`

Error code of the parser. The code is returned when the grammar uses reserved names for terminals and nonterminals. There are two reserved names `$S` (for axiom) and `$eof` for end of file (input end marker). The parser adds these symbols and rules with these symbols to the grammar given by user. So user should not use these names in his grammar.

#### `YAEP_REPEATED_TERM_DECL`

Error code of the parser. The code is returned when the grammar contains several declarations of terminals with the same name.

#### `YAEP_NEGATIVE_TERM_CODE`

Error code of the parser. The code is returned when the grammar terminal is described with negative code.

#### `YAEP_REPEATED_TERM_CODE`

Error code of the parser. The code is returned when the two or more grammar terminals are described with the same code.

#### `YAEP_NO_RULES`

Error code of the parser. The code is returned when the grammar given by user has no rules.

#### `YAEP_TERM_IN_RULE_LHS`

Error code of the parser. The code is returned when grammar rule given by user contains terminal in left hand side of the rule.

#### `YAEP_INCORRECT_TRANSLATION`

Error code of the parser. The code is returned when grammar rule translation is not correct. The single reason for this is translation of the rule consists of translations of more one symbols in the right hand side of the rule without forming an abstract tree node.

#### `YAEP_NEGATIVE_COST`

Error code of the parser. The code is returned when abstract node has a negative cost.

#### `YAEP_INCORRECT_SYMBOL_NUMBER`

Error code of the parser. The code is returned when grammar rule translation contains incorrect symbol number which should be nonnegative number less than rule right hand side length.

#### `YAEP_UNACCESSIBLE_NONTERM`

Error code of the parser. The code is returned when there is grammar nonterminal which cannot be derived from axiom.

#### `YAEP_NONTERM_DERIVATION`

Error code of the parser. The code is returned when there is grammar nonterminal which cannot derive a terminal string.

#### `YAEP_LOOP_NONTERM`

Error code of the parser. The code is returned when there is grammar nonterminal which can derive only itself. The parser does not work with such grammars.

#### `YAEP_INVALID_TOKEN_CODE`

Error code of the parser. The code is returned when the parser got input token whose code is different from all grammar terminal codes.

---

### Functions

#### `yaep_create_grammar`

```c
struct grammar *yaep_create_grammar(void)
```

Should be called the first. It actually creates a YAEP parser with undefined grammar. You can use two or more parsers simultaneously. The function returns `NULL` if there is no memory.

---

#### `yaep_error_code`

```c
int yaep_error_code(struct grammar *g)
```

Returns the last occurred error code (see the possible error codes above) for given parser. If the function returns zero, no error was found so far.

---

#### `yaep_error_message`

```c
const char *yaep_error_message(struct grammar *g)
```

Returns detail message about last occurred error. The message always corresponds to the last error code returned by the previous function.

---

#### `yaep_read_grammar`

```c
int yaep_read_grammar(struct grammar *g, int strict_p,
                      const char *(*read_terminal)(int *code),
                      const char *(*read_rule)(const char ***rhs,
                                               const char **abs_node,
                                               int *anode_cost,
                                               int **transl))
```

One of two functions which tunes the parser to given grammar. The grammar is read with the aid of functions given as parameters.

**Parameters:**

* **`read_terminal`** - Function for reading terminals. This function is called before function `read_rule`. The function should return the name and the code of the next terminal. If all terminals have been read, the function returns NULL. The terminal code should be nonnegative.

* **`read_rule`** - Function called to read the next rule. This function is called after function `read_terminal`. The function should return the name of nonterminal in the left hand side of the rule and array of names of symbols in the right hand side of the rule (the array end marker should be `NULL`). If all rules have been read, the function returns `NULL`. All symbols with name which was not provided by function `read_terminal` are considered to be nonterminals.

  The function also returns translation given by abstract node name and its fields which will be translation of symbols (with indexes given in array given by parameter `transl`) in the right hand side of the rule. All indexes in `transl` should be different (so the translation of a symbol cannot be represented twice). The end marker of the array should be a negative value.
  
  There is a reserved value of the translation symbol number denoting empty node. It is value defined by macro `YAEP_NIL_TRANSLATION_NUMBER`. If parameter `transl` is `NULL` or contains only the end marker, translations of the rule will be empty node.
  
  If `abs_node` is `NULL`, abstract node is not created. In this case `transl` should be null or contain at most one element. This means that the translation of the rule will be correspondingly empty node or the translation of the symbol in the right hand side given by the single array element.
  
  The cost of the abstract node if given is passed through parameter `anode_cost`. If `abs_node` is not `NULL`, the cost should be greater or equal to zero. Otherwise the cost is ignored.

  There is reserved terminal `error` which is used to mark start point of error recovery.

* **`strict_p`** - Nonzero value means more strict checking the grammar. In this case, all nonterminals will be checked on ability to derive a terminal string instead of only checking axiom for this.

**Returns:** Zero if it is all ok. Otherwise, the function returns the error code occurred.

---

#### `yaep_parse_grammar`

```c
int yaep_parse_grammar(struct grammar *g, int strict_p,
                       const char *description)
```

Another function which tunes the parser to given grammar. The grammar is given by string `description`. The description is similar to YACC one.

**Grammar Description Syntax:**

```
file : file terms [';']
     | file rule
     | terms [';']
     | rule

terms : terms IDENTIFIER ['=' NUMBER]
      | TERM

rule : IDENTIFIER ':' rhs [';']

rhs : rhs '|' sequence [translation]
    | sequence [translation]

sequence :
         | sequence IDENTIFIER
         | sequence C_CHARACTER_CONSTANT

translation : '#'
            | '#' NUMBER
            | '#' '-'
            | '#' IDENTIFIER [NUMBER] '(' numbers ')'

numbers :
        | numbers NUMBER
        | numbers '-'
```

**Description Format:**

The description consists of terminal declaration and rules sections.

**Terminal Declaration Section:**
* Describes name of terminals and their codes
* Terminal code is optional. If omitted, the terminal code will be the next free code starting with 256
* You can declare terminal several times (the single condition: its code should be the same)
* Character constant present in the rules is a terminal described by default. Its code is always code of the character constant.

**Rules:**
* Syntax is the same as YACC rule syntax
* The single difference is an optional translation construction starting with `#` right after each alternative
* The translation part could be:
  - A single number which means that the translation of the alternative will be the translation of the symbol with given number (symbol numbers in alternative start with 0)
  - Empty or `-` which mean empty node
  - Abstract node with given name, optional cost, and with fields whose values are the translations of the alternative symbols with numbers given in parentheses after the abstract node name
  - You can use `-` in abstract node to show that empty node should be used in this place
  - If the cost is absent it is believed to be one
  - The cost of terminal, error node, and empty node is always zero

**Reserved Terminal:**
* There is reserved terminal `error` which is used to mark start point of error recovery.

**Parameters:**
* **`strict_p`** - Nonzero value means more strict checking the grammar

**Returns:** Zero if successful, otherwise the error code.

---

#### `yaep_set_lookahead_level`

```c
int yaep_set_lookahead_level(struct grammar *grammar, int level)
```

Sets up level of usage of look ahead in parser work.

* Value `0` means no usage of lookaheads at all
* Value `1` (default): Lookahead with static (independent on input tokens) context sets in parser situation - gives the best results with the point of view of space and speed
* Value `2`: Lookahead with dynamic (dependent on input tokens) context sets in parser situations - does slightly worse
* Higher values: Treated same as 2

If the level value is negative, zero is used instead. If the value is greater than two, two is used.

**Returns:** The previously set up level.

---

#### `yaep_set_debug_level`

```c
int yaep_set_debug_level(struct grammar *grammar, int level)
```

Sets up level of debugging information output to `stderr`. The more level, the more information is output. The default value is 0 (no output).

**Debug Information Includes:**
* Statistics
* Result translation tree
* Grammar
* Parser sets
* Parser sets with all situations
* Situations with contexts

**Special:** Setting up negative debug level results in output of translation for program `dot` of graphic visualization package `graphviz`.

**Returns:** The previously set up debug level.

---

#### `yaep_set_one_parse_flag`

```c
int yaep_set_one_parse_flag(struct grammar *grammar, int flag)
```

Sets up building only one translation tree (parameter value 0) or all parse trees for ambiguous grammar for which several parsings are possible.

* For unambiguous grammar the flag does not affect the result
* The default value is 1

**Returns:** The previously used flag value.

---

#### `yaep_set_cost_flag`

```c
int yaep_set_cost_flag(struct grammar *grammar, int flag)
```

Sets up building only translation tree (trees if we set up `one_parse_flag` to 0) with minimal cost.

* For unambiguous grammar the flag does not affect the result
* The default value is 0

**Returns:** The previously used flag value.

---

#### `yaep_set_error_recovery_flag`

```c
int yaep_set_error_recovery_flag(struct grammar *grammar, int flag)
```

Sets up internal flag whose nonzero value means making error recovery if syntax error occurred. Otherwise, syntax error results in finishing parsing (although function `syntax_error` in function `yaep_parse` will be called once).

* The default value is 1

**Returns:** The previously used flag value.

---

#### `yaep_set_recovery_match`

```c
int yaep_set_recovery_match(struct grammar *grammar, int n_toks)
```

Sets up recovery parameter which means how much subsequent tokens should be successfully shifted to finish error recovery.

* The default value is 3

**Returns:** The previously used flag value.

---

#### `yaep_parse`

```c
int yaep_parse(struct grammar *grammar,
               int (*read_token)(void **attr),
               void (*syntax_error)(int err_tok_num, void *err_tok_attr,
                                   int start_ignored_tok_num,
                                   void *start_ignored_tok_attr,
                                   int start_recovered_tok_num,
                                   void *start_recovered_tok_attr),
               void *(*parse_alloc)(int nmemb),
               void (*parse_free)(void *mem),
               struct yaep_tree_node **root,
               int *ambiguous_p)
```

Major parser function. It parses input according to the grammar.

**Parameters:**

* **`read_token`** - Function that provides input tokens. It returns code of the next input token and its attribute. If the function returns negative value, we've read all tokens.

* **`syntax_error`** - Function called when syntactic error has been found. It may print an error message about syntax error which occurred on token with number `err_tok_num` and attribute `err_tok_attr`. The following four parameters describe made error recovery which ignored tokens starting with token given by 3rd and 4th parameters. The first token which was not ignored is described by the last parameters. If the number of ignored tokens is zero, all parameters describe the same token. If the error recovery is switched off, the third and the fifth parameters will be negative and the fourth and the sixth parameters will be `NULL`.

* **`parse_alloc`** - Function used by YAEP to allocate memory for parse tree representation (translation). After calling `yaep_free_grammar` we free all memory allocated for the parser. At this point it is convenient to free all memory but parse tree. Therefore we require the following function. So the caller will be responsible to allocate and free memory for parse tree representation (translation). But the caller should not free the memory until `yaep_free_grammar` is called for the parser. The function may be called even during reading the grammar not only during the parsing.

* **`parse_free`** - Function used by the parser to free memory allocated by `parse_alloc`. If it is `NULL`, the memory is not freed.

* **`root`** - Pointer to where root of the parse tree will be returned. The tree represents the translation. Value will be `NULL` only if syntax error occurred and error recovery was switched off.

* **`ambiguous_p`** - Pointer to flag that will be set if we found that the grammar is ambiguous (it works even if we asked only one parse tree without alternatives).

**Returns:** The error code (which can also be returned by `yaep_error_code`). If the code is zero, the function will also return root of the parse tree through parameter `root`.

---

#### `yaep_free_grammar`

```c
void yaep_free_grammar(struct grammar *grammar)
```

Frees all memory allocated for the parser. This function should be called the last for given parser.

---

## See Also

* `yaep.h` - Complete API header file
* Test files in `test/C/` for usage examples
* `Internals.txt` - Internal implementation details

---

## Original Author

This parser was originally created by **Vladimir Makarov** (<vmakarov@gcc.gnu.org>) in October 2015.
