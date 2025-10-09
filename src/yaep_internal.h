/**
 * @file yaep_internal.h
 * @brief Internal YAEP structures for optimization modules
 *
 * This header exposes internal YAEP data structures needed by optimization
 * modules like Leo. These definitions are NOT part of the public API and
 * should only be used by YAEP internal implementation files.
 *
 * USAGE:
 *   Include this header ONLY in YAEP implementation files (.c files in src/)
 *   that need access to internal structures.
 *
 * MAINTENANCE:
 *   Keep this synchronized with definitions in yaep.c. When yaep.c structures
 *   change, update this header accordingly.
 */

#ifndef YAEP_INTERNAL_H
#define YAEP_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations from yaep.c */
struct symb;

/**
 * Set core structure - the core content of an Earley set
 *
 * Contains the situations (items) in a set, without origin information.
 * Multiple sets can share the same core if they have the same items.
 */
struct set_core
{
  int num;                 /* Unique number of set core */
  unsigned int hash;       /* Hash of the set core */
  struct symb *term;       /* Terminal scanned to reach this core */
  int n_sits;              /* Total number of situations */
  int n_start_sits;        /* Number of start situations */
  struct sit **sits;       /* Array of situations */
  int n_all_dists;         /* Number of situations with distance tracking */
  int *parent_indexes;     /* Parent indexes for deriving distances */
};

/**
 * Earley Set - all parse states at a token position
 *
 * Represents all possible parse states after consuming a specific
 * number of input tokens. Set i contains all items after consuming i tokens.
 */
struct set
{
  struct set_core *core;   /* Core containing the situations */
  unsigned int dists_hash; /* Hash of the distance array */
  int *dists;              /* Origin positions for start situations */
};

/**
 * Rule structure - represents a grammar production
 *
 * Represents a production rule: LHS → RHS
 * For example: Expr → Expr '+' Term
 */
struct rule
{
  int num;                /* Rule order number */
  int rhs_len;            /* Length of right-hand side */
  struct rule *next;      /* Next rule in grammar */
  struct rule *lhs_next;  /* Next rule with same LHS */
  struct symb *lhs;       /* Left-hand side nonterminal */
  struct symb **rhs;      /* Right-hand side symbols */
  /* ... other fields omitted for brevity ... */
};

/**
 * Vector structure for situation indexes
 */
struct vect
{
  int intern;   /* Internal index into VLO array (matches yaep.c layout) */
  int len;      /* Number of elements */
  int *els;     /* Array of situation indexes */
};

/**
 * Core symbol vector - tracks situations with a specific symbol after dot
 *
 * This structure is used for fast lookup during scanning and completion.
 * For a given (set_core, symbol) pair, it stores:
 * - Which situations have this symbol after the dot (transitions)
 * - Which situations can produce this symbol (reduces)
 */
struct core_symb_vect
{
  struct set_core *set_core;  /* The set core this belongs to */
  struct symb *symb;           /* The symbol after dot */
  
  /* Situations with given symbol after dot */
  struct vect transitions;
  
#ifdef TRANSITIVE_TRANSITION
  /* Transitive closure: includes situations reachable through reductions */
  struct vect transitive_transitions;
#endif
  
  /* Situations that can reduce to produce this symbol (for completion) */
  struct vect reduces;
};

/**
 * Situation (Earley item) structure
 *
 * Represents a position in a grammar rule during parsing:
 *   A → α . β   where α is before dot, β is after dot
 */
struct sit
{
  struct rule *rule;     /* The production rule */
  short pos;             /* Position of dot in RHS (0 to rhs_len) */
  char empty_tail_p;     /* TRUE if symbols after dot can derive empty */
  int sit_number;        /* Unique situation number */
  int context;           /* Lookahead context number */
  
#ifdef TRANSITIVE_TRANSITION
  int sit_check;         /* Marker to avoid reprocessing */
#endif
  
  void *lookahead;       /* Lookahead terminal set (term_set_el_t*) */
};

#ifdef __cplusplus
}
#endif

#endif /* YAEP_INTERNAL_H */
