# YAEP Grammar Definition Language

Welcome to the YAEP (Yet Another Earley Parser) grammar definition language!  
YAEP is a Unicode-capable, scannerless Earley parser. Its grammar syntax is inspired by YACC/Bison, but with a streamlined, modernized approach and features tailored for both humans and machines.

---

## Table of Contents

1. [Overview](#overview)
2. [Grammar Layout](#grammar-layout)
3. [Identifiers & Case Sensitivity](#identifiers--case-sensitivity)
4. [Terminals](#terminals)
5. [Rules & Productions](#rules--productions)
6. [Explicit Enumeration](#explicit-enumeration)
7. [Character Literals & Unicode](#character-literals--unicode)
8. [Translation Annotations](#translation-annotations)
9. [Best Practices for LHS](#best-practices-for-lhs)
10. [Comments](#comments)
11. [Complete Example](#complete-example)
12. [Key Differences from Classic YACC/Bison](#key-differences-from-classic-yaccbison)
13. [References](#references)

---

## Overview

YAEP grammars are written in a YACC-like style, but with some key simplifications and modern features:

- Unicode support throughout (identifiers, literals)
- Inline terminal and rule definitions
- Simple, clear annotation of translation rules and costs
- No scanner required (“scannerless parsing”)

---

## Grammar Layout

A YAEP grammar consists of:

- **Zero or more terminal declarations** (at the top, optional)
- **Production rules** (with optional translation and cost annotations)
- **Comments** (C-style, anywhere whitespace is allowed)

The order is flexible; you can place terminals and rules as needed.

---

## Identifiers & Case Sensitivity

- **Identifiers** (names for terminals and non-terminals) are case-sensitive.
- YAEP accepts any Unicode letter or underscore as the initial character, and then letters, digits, or underscores.
- There are **no capitalization conventions enforced** (e.g., `expr`, `EXPR`, and `Expr` are all unique and valid).

---

## Terminals

- **Terminals** are declared at the top of the grammar file, optionally with numeric codes:
  ```
  PLUS = 43
  STAR = 42
  ```
- The special `TERM` keyword can end the list of terminals.
- Terminals may also be introduced implicitly by their use in rules or as character literals (see below).
- **No `%token`, `%left`, `%right`, or associativity/precedence directives.**

---

## Rules & Productions

Define language structure using productions in the style of:

```
nonterminal : RHS [ | RHS ... ] [translation] ;
```

- The **LHS** (left-hand side) is any identifier (preferably a non-terminal).
- Each **RHS** (right-hand side) is a sequence of identifiers and/or character literals.
- You can use `|` to separate alternatives and `;` to terminate a rule (semicolon is optional after each rule).

---

## Explicit Enumeration

YAEP **requires explicit enumeration** of alternatives if you want to match a set of characters or symbols. There is no shorthand for ranges or groups.

For example:

```yacc
digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;

lowercase_letter : 'a' | 'b' | 'c' | ... | 'z' ;

uppercase_letter : 'A' | 'B' | ... | 'Z' ;
```

You must list each alternative individually.

---

## Character Literals & Unicode

- Any single Unicode character can be written as a literal in single quotes (`'+'`, `'λ'`, `'Ж'`).
- Character literals are always **terminals**.
- Identifiers and literals are fully Unicode-capable.

---

## Translation Annotations

YAEP allows you to annotate rules for translation/AST construction and to specify parse costs:

- `#` starts a translation annotation.
- `# IDENT` attaches an abstract node label to the rule.
- You can add a cost (`# IDENT 3`) or arguments (`# IDENT (1 2)`).
- `# NUMBER`, `# -`, or `#` alone have special translation meanings.

**Example:**

```yacc
expr : expr PLUS term # Add (1 3)
     | term #
     ;
```

---

## Best Practices for LHS

- **YAEP will accept any identifier as a rule’s left-hand side (LHS)**—even if it was declared as a terminal.
- **Best Practice:**  
  Only use non-terminals as the LHS of rules.  
  Avoid using an identifier as both terminal and non-terminal, as this leads to ambiguous or broken grammars.

| Symbol Type     | Allowed on LHS? | What Parser Does          | Best Practice      |
|-----------------|-----------------|---------------------------|--------------------|
| Non-terminal    | Yes             | Treats as non-terminal    | Yes                |
| Terminal        | Yes (discouraged)| Treats as non-terminal   | **No** (avoid this)|
| Char literal    | No (by syntax)  | Not accepted              | Never              |

---

## Comments

Use C-style block comments anywhere whitespace is allowed:

```c
/* This is a comment */
```

---

## Complete Example

```yacc
/* Simple arithmetic */

PLUS = 43
STAR = 42
TERM

expr : expr PLUS term # Add (1 3)
     | term #
     ;

term : term STAR factor # Mul (1 3)
     | factor #
     ;

factor : '(' expr ')' # Pass (2)
       | NUMBER #
       ;

digit : '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;

lowercase_letter : 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j'
                 | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't'
                 | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' ;

uppercase_letter : 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J'
                 | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T'
                 | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' ;
```

---

## Key Differences from Classic YACC/Bison

- **No scanner required:** YAEP is truly scannerless; no `%token` or external token definitions.
- **No precedence/associativity:** No support for `%left`, `%right`, `%nonassoc`, or precedence resolution.
- **No code embedding:** No `{ C code }` blocks in grammar files.
- **No header/prologue/epilogue:** No `%{ ... %}` or code before/after the grammar.
- **No `%start`, `%type`, `%union`, or value type declarations.**
- **No grammar file sections:** You can freely interleave terminals and rules.
- **Must enumerate alternatives:** There is no shorthand for groups of symbols.
- **Unicode everywhere:** Identifiers and literals are fully Unicode-capable.
- **No parser customizations:** No `%error-verbose`, `%import`, `%include`, or related features.
- **LHS is flexible:** Any identifier can be an LHS, but only non-terminals should be used.

---

## References

- [`src/sgramm.y` (YAEP grammar parser)](https://github.com/gitrdm/yaep/blob/9f9d299debb47ab9d35088f90e27da292cfa891a/src/sgramm.y)
- [YAEP Documentation](https://github.com/gitrdm/yaep/tree/master/doc)

---