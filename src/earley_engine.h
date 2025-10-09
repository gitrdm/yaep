/*
   YAEP (Yet Another Earley Parser) - Earley Engine Module

   Copyright (c) 1997-2025  Vladimir Makarov <vmakarov@gcc.gnu.org>

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/**
 * @file earley_engine.h
 * @brief Earley parsing engine - Core Earley algorithm implementation
 *
 * This module contains the core Earley parsing algorithm, extracted from
 * the monolithic yaep.c implementation for better modularity and to enable
 * future optimizations (Leo's right-recursion, SPPF, etc.).
 *
 * ARCHITECTURE:
 *   The Earley engine is responsible for the core parsing algorithm:
 *   - Building parse sets (the parse list)
 *   - Managing Earley items and sets
 *   - Executing the prediction, scanning, and completion steps
 *
 * DESIGN RATIONALE:
 *   This modularization phase (Phase P2) is behavior-preserving. No
 *   algorithmic changes are made - only code organization. Future
 *   optimization phases (P3-P7) will build upon this clean foundation.
 *
 * REFERENCES:
 *   - Earley (1970) "An efficient context-free parsing algorithm"
 *   - Aycock & Horspool (2002) "Practical Earley Parsing"
 *
 * PHASE: P2 - Modularization (Behavior-Preserving Refactoring)
 */

#ifndef __YAEP_EARLEY_ENGINE__
#define __YAEP_EARLEY_ENGINE__

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations to avoid circular dependencies.
 * The actual grammar structure is defined in yaep.c and will be
 * passed as an opaque pointer during this phase. */
struct grammar;
struct set;
struct core_symb_vect;

/**
 * Opaque handle to the Earley parsing engine
 *
 * This structure contains the state needed for Earley parsing.
 * During Phase P2, this will initially be a thin wrapper, with
 * implementation gradually moved from yaep.c.
 *
 * MEMORY: Allocated by earley_engine_create(), freed by earley_engine_destroy()
 * LIFECYCLE: Create before parse, destroy after parse complete
 */
typedef struct earley_engine earley_engine_t;

/**
 * Create an Earley parsing engine instance
 *
 * Allocates and initializes a new Earley engine for parsing operations.
 * The engine is associated with a specific grammar and will use that
 * grammar's allocator for internal memory management.
 *
 * ALGORITHM: Currently minimal - just allocation. Future phases will
 *            add initialization of optimization data structures.
 *
 * @param grammar Grammar to use for parsing (must not be NULL)
 *
 * @return Pointer to new engine instance, or NULL on allocation failure
 *
 * @note The engine does not take ownership of the grammar.
 * @note Call earley_engine_destroy() when done to avoid memory leaks.
 *
 * EXAMPLE:
 *   struct grammar *g = yaep_create_grammar();
 *   earley_engine_t *engine = earley_engine_create(g);
 *   // ... use engine ...
 *   earley_engine_destroy(engine);
 *
 * TESTING: See test/test_earley_engine.c (to be created in P2)
 */
earley_engine_t *earley_engine_create(struct grammar *grammar);

/**
 * Destroy an Earley parsing engine instance
 *
 * Frees all memory associated with the engine, including any internal
 * data structures used for parsing.
 *
 * @param engine Engine to destroy (may be NULL, in which case no action)
 *
 * @note After this call, the engine pointer is invalid and must not be used.
 * @note This does NOT free the associated grammar.
 *
 * TESTING: Verified with valgrind to ensure no leaks
 */
void earley_engine_destroy(earley_engine_t *engine);

/**
 * Reset the Earley engine for a new parse
 *
 * Clears all parse-specific state, allowing the engine to be reused
 * for parsing a new input with the same grammar.
 *
 * @param engine Engine to reset (must not be NULL)
 *
 * @note More efficient than destroy + create for multiple parses
 * @note The grammar remains associated with the engine
 *
 * TESTING: Verify multiple parses with same engine produce correct results
 */
void earley_engine_reset(earley_engine_t *engine);

/**
 * Build the parse list (main Earley parsing algorithm)
 *
 * This is the core Earley algorithm: builds the parse list (sequence of
 * Earley sets) by processing tokens one at a time. Each token triggers
 * prediction, scanning, and completion steps.
 *
 * ALGORITHM:
 *   For each token in input:
 *     1. Look up transition from current set via token symbol
 *     2. If transition exists, build new set (scanning step)
 *     3. Prediction and completion happen during set building
 *     4. Advance to next set
 *   Error recovery performed if no valid transition found.
 *
 * COMPLEXITY: O(n³) for arbitrary CFGs, where n = input length
 *             (Can be improved to O(n²) with Leo optimization in P4)
 *
 * @param engine Earley engine instance (must not be NULL)
 *
 * @return 0 on success, non-zero error code on failure
 *
 * @note This function is currently a wrapper that will be implemented
 *       during Phase P2 by extracting code from yaep.c:build_pl()
 *
 * OPTIMIZATION: Future phases (P4-P6) will add Leo items, SPPF, etc.
 *
 * TESTING: Must produce identical results to original build_pl()
 */
int earley_engine_build_parse_list(earley_engine_t *engine);

/**
 * Build a new Earley set from a transition
 *
 * Creates a new Earley set by applying a transition (scan step) from
 * the current set. This involves copying items that can advance via
 * the given symbol and performing closure (prediction).
 *
 * ALGORITHM:
 *   1. Create new set with appropriate core
 *   2. Add items from transition vector (scan step)
 *   3. Perform closure: predict new items for nonterminals at dot
 *   4. Deduplicate items
 *
 * @param engine Earley engine instance (must not be NULL)
 * @param current_set Current Earley set (must not be NULL)
 * @param transition Transition vector for this symbol (must not be NULL)
 * @param lookahead Lookahead terminal number (-1 if no lookahead)
 *
 * @return Pointer to newly created set, or NULL on error
 *
 * @note The set is owned by the engine and will be freed on engine_destroy()
 *
 * TESTING: Verify set structure and item count matches original
 */
struct set *earley_engine_build_new_set(earley_engine_t *engine,
                                         struct set *current_set,
                                         struct core_symb_vect *transition,
                                         int lookahead);

/**
 * Perform prediction step for a nonterminal
 *
 * When the dot is before a nonterminal N in an item, adds prediction items
 * for all rules with N on the left-hand side.
 *
 * ALGORITHM:
 *   For each rule R with LHS = N:
 *     Add item [R, 0] to current set (dot at position 0)
 *
 * COMPLEXITY: O(R) where R = number of rules for nonterminal N
 *
 * @param engine Earley engine instance (must not be NULL)
 * @param set Set to add predicted items to (must not be NULL)
 * @param nonterminal Nonterminal to predict (must not be NULL)
 *
 * @return Number of items added, or -1 on error
 *
 * @note Deduplication ensures items aren't added twice
 *
 * OPTIMIZATION: P3 will add nullable preprocessing to skip empty predictions
 *
 * TESTING: Verify predicted items match expected rules
 */
int earley_engine_predict(earley_engine_t *engine,
                          struct set *set,
                          struct symb *nonterminal);

/**
 * Perform scan step for a terminal
 *
 * Advances items that expect the current terminal symbol.
 * Creates transition to next set with advanced items.
 *
 * ALGORITHM:
 *   For each item [R, p] where symbol at p = current terminal:
 *     Add item [R, p+1] to next set
 *
 * @param engine Earley engine instance (must not be NULL)
 * @param current_set Current Earley set (must not be NULL)
 * @param terminal Terminal symbol being scanned (must not be NULL)
 *
 * @return Pointer to next set after scanning, or NULL if no items match
 *
 * @note If no items can scan this terminal, returns NULL (parse error)
 *
 * OPTIMIZATION: P6 will add Aycock-Horspool grouping for faster scanning
 *
 * TESTING: Verify scanned items correctly advanced
 */
struct set *earley_engine_scan(earley_engine_t *engine,
                               struct set *current_set,
                               struct symb *terminal);

/**
 * Perform completion step for a completed item
 *
 * When an item [R, n] has dot at end (completed), propagates the completed
 * nonterminal back to items waiting for it.
 *
 * ALGORITHM:
 *   For completed item [R, n] with LHS = N:
 *     For each item [R', p'] in origin set where symbol at p' = N:
 *       Add item [R', p'+1] to current set
 *
 * COMPLEXITY: O(I) where I = number of items in origin set
 *
 * @param engine Earley engine instance (must not be NULL)
 * @param current_set Set containing completed item (must not be NULL)
 * @param completed_item Item that just completed (must not be NULL)
 *
 * @return Number of items added, or -1 on error
 *
 * @note Completion can trigger more predictions (closure)
 *
 * OPTIMIZATION: P4 will add Leo items to eliminate redundant completions
 *
 * TESTING: Verify completion propagation matches expected items
 */
int earley_engine_complete(earley_engine_t *engine,
                           struct set *current_set,
                           struct sit *completed_item);

#ifdef __cplusplus
}
#endif

#endif /* __YAEP_EARLEY_ENGINE__ */
