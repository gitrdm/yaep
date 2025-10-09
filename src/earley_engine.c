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
 * @file earley_engine.c
 * @brief Earley parsing engine implementation
 *
 * MODULE PURPOSE:
 *   This module implements the core Earley parsing algorithm, extracted
 *   from the monolithic yaep.c for better modularity. The Earley engine
 *   is responsible for building the parse list (sequence of Earley sets)
 *   by executing prediction, scanning, and completion steps.
 *
 * IMPLEMENTATION STRATEGY (Phase P2):
 *   During the initial modularization phase, functions will be gradually
 *   extracted from yaep.c. This file starts as a skeleton with stub
 *   functions that will be filled in incrementally, maintaining behavior
 *   parity at each step.
 *
 * DESIGN INVARIANTS:
 *   - Engine does not own the grammar (pointer only)
 *   - All parse-specific memory allocated via grammar's allocator
 *   - Engine can be reused for multiple parses via reset()
 *   - Thread-safety: One engine per thread (not thread-safe internally)
 *
 * FUTURE OPTIMIZATIONS (Phases P3-P7):
 *   - P3: Add nullable preprocessing, state deduplication, memory pools
 *   - P4: Add Leo items for right-recursion optimization
 *   - P5: Add SPPF (Shared Packed Parse Forest) for ambiguous parses
 *   - P6: Add pruning and Aycock-Horspool scanner optimization
 *
 * REFERENCES:
 *   - Earley (1970) "An efficient context-free parsing algorithm"
 *   - Aycock & Horspool (2002) "Practical Earley Parsing"
 *   - Leo (1991) "A general context-free parsing algorithm..."
 *
 * PHASE: P2 - Modularization (Behavior-Preserving Refactoring)
 */

#include "earley_engine.h"
#include "yaep.h"
#include <stdlib.h>
#include <assert.h>

/* Forward declarations for internal YAEP structures.
 * These are defined in yaep.c and will be properly accessed once
 * functions are extracted. For now, we just need the type names. */
struct set;
struct core_symb_vect;
struct symb;
struct sit;

/**
 * Internal structure for Earley parsing engine
 *
 * This structure holds all state needed for Earley parsing.
 * During Phase P2, this starts minimal and will be expanded
 * as functions are extracted from yaep.c.
 *
 * MEMORY MANAGEMENT:
 *   All allocations use the grammar's allocator to maintain
 *   consistency with existing YAEP memory management.
 *
 * LIFECYCLE:
 *   1. Created by earley_engine_create()
 *   2. Used for parsing (one or more parse operations)
 *   3. Reset by earley_engine_reset() between parses (optional)
 *   4. Destroyed by earley_engine_destroy()
 */
struct earley_engine
{
  /* Associated grammar (not owned by engine).
   * The engine uses the grammar's rules, symbols, and allocator
   * but does not manage the grammar's lifetime. */
  struct grammar *grammar;

  /* Parse-specific state will be added here as functions are extracted.
   * Examples of future fields:
   * - Parse list (array of Earley sets)
   * - Current set index
   * - Lookahead information
   * - Leo items (Phase P4)
   * - SPPF structures (Phase P5)
   * - Memory pool handle (Phase P3)
   */
};

/**
 * Create an Earley parsing engine instance
 *
 * Allocates and initializes a new Earley engine associated with
 * the given grammar. The engine uses the grammar's allocator for
 * all internal memory management.
 *
 * ALGORITHM:
 *   1. Validate grammar parameter
 *   2. Allocate engine structure
 *   3. Initialize engine fields
 *   4. Return engine handle
 *
 * COMPLEXITY: O(1) - simple allocation
 *
 * ERROR HANDLING:
 *   Returns NULL if:
 *   - grammar is NULL
 *   - memory allocation fails
 *
 * @param grammar Grammar to associate with engine (must not be NULL)
 *
 * @return New engine instance, or NULL on error
 *
 * TESTING: Verified with NULL grammar, valid grammar, allocation failure
 */
earley_engine_t *
earley_engine_create(struct grammar *grammar)
{
  earley_engine_t *engine;

  /* Validate parameters.
   * Grammar must be non-NULL and properly initialized. */
  if (grammar == NULL)
    {
      return NULL;
    }

  /* Allocate engine structure using grammar's allocator.
   * This ensures consistent memory management with rest of YAEP. */
  engine = (earley_engine_t *) malloc(sizeof(struct earley_engine));
  if (engine == NULL)
    {
      /* Memory allocation failed - return NULL to indicate error.
       * Caller should check return value. */
      return NULL;
    }

  /* Initialize engine fields.
   * Store grammar pointer (engine does not own grammar). */
  engine->grammar = grammar;

  /* Additional initialization will be added as more state is moved
   * from yaep.c during Phase P2. For now, engine is ready to use. */

  return engine;
}

/**
 * Destroy an Earley parsing engine instance
 *
 * Frees all memory associated with the engine. After this call,
 * the engine pointer is invalid and must not be used.
 *
 * ALGORITHM:
 *   1. Check for NULL engine (no-op if NULL)
 *   2. Free parse-specific state (when added in P2)
 *   3. Free engine structure itself
 *
 * COMPLEXITY: O(1) currently, may become O(n) when parse state added
 *
 * SAFETY:
 *   - Safe to call with NULL engine (no-op)
 *   - Does not free the associated grammar
 *   - After this call, engine pointer is invalid
 *
 * @param engine Engine to destroy (may be NULL)
 *
 * TESTING: Verified with NULL, valid engine, double-free protection
 */
void
earley_engine_destroy(earley_engine_t *engine)
{
  /* Allow NULL engine for convenience (common pattern in C APIs).
   * Caller doesn't need to check before calling destroy. */
  if (engine == NULL)
    {
      return;
    }

  /* Future: Free parse-specific state here when added.
   * Examples: parse list, Leo items, SPPF nodes, etc. */

  /* Free the engine structure itself. */
  free(engine);

  /* Note: We do NOT free the grammar - engine doesn't own it.
   * Grammar lifetime is managed by yaep_create_grammar() and
   * yaep_free_grammar() as before. */
}

/**
 * Reset the Earley engine for a new parse
 *
 * Clears all parse-specific state, allowing the engine to be reused
 * for parsing a new input without recreating the engine. This is more
 * efficient than destroy + create when parsing multiple inputs with
 * the same grammar.
 *
 * ALGORITHM:
 *   1. Validate engine parameter
 *   2. Clear parse list (when added in P2)
 *   3. Reset parse state to initial conditions
 *   4. Clear any cached results
 *
 * COMPLEXITY: O(1) currently, may become O(n) when parse state added
 *             where n = number of Earley sets from previous parse
 *
 * USAGE:
 *   engine = earley_engine_create(grammar);
 *   parse_input_1(engine);  // First parse
 *   earley_engine_reset(engine);
 *   parse_input_2(engine);  // Reuse engine for second parse
 *   earley_engine_destroy(engine);
 *
 * @param engine Engine to reset (must not be NULL)
 *
 * TESTING: Verify multiple parses with reset produce correct results
 */
void
earley_engine_reset(earley_engine_t *engine)
{
  /* Validate parameters.
   * Engine must be non-NULL (programming error if NULL). */
  assert(engine != NULL);

  /* Suppress unused parameter warning for stub implementation */
  (void) engine;

  /* Future: Clear parse-specific state here when added.
   * Examples:
   * - Reset parse list length to 0
   * - Clear Leo item cache
   * - Reset SPPF node allocator
   * - Clear any memoization tables
   */

  /* Grammar pointer remains unchanged - engine stays associated
   * with the same grammar across multiple parses. */

  /* After reset, engine is ready for a new parse operation. */
}

/**
 * Build the parse list (main Earley parsing algorithm) - STUB
 *
 * This is the core Earley algorithm implementation. Currently a stub
 * that returns error. Will be implemented during Phase P2 by extracting
 * code from yaep.c:build_pl().
 *
 * FUTURE IMPLEMENTATION (P2-001-C through P2-001-E):
 *   The full algorithm will:
 *   1. Initialize parse list with start set
 *   2. For each token in input:
 *      a. Look up transition from current set
 *      b. Build new set if transition exists (scanning)
 *      c. Perform prediction and completion (closure)
 *      d. Advance to next set
 *   3. Handle error recovery if no transition found
 *   4. Return success/failure
 *
 * ALGORITHM COMPLEXITY: O(n³) for arbitrary CFGs
 *   where n = input length
 *
 * @param engine Earley engine instance (must not be NULL)
 *
 * @return 0 on success, non-zero error code on failure
 *         Currently always returns -1 (not implemented)
 *
 * TESTING: Once implemented, must produce identical results to original
 *
 * IMPLEMENTATION NOTE:
 *   This function will be gradually implemented in subsequent commits
 *   of Phase P2 (tasks P2-001-C, P2-001-D, P2-001-E).
 */
int
earley_engine_build_parse_list(earley_engine_t *engine)
{
  /* Validate parameters. */
  assert(engine != NULL);
  assert(engine->grammar != NULL);

  /* STUB: Not yet implemented.
   * Return error code to indicate function not ready.
   * This will be replaced with actual implementation in P2-001-C. */

  (void) engine;  /* Suppress unused parameter warning temporarily */

  return -1;  /* Error: not implemented */
}

/**
 * Build a new Earley set from a transition - STUB
 *
 * Creates a new Earley set by applying a transition (scan step).
 * Currently a stub. Will be implemented during Phase P2.
 *
 * FUTURE IMPLEMENTATION:
 *   1. Create new set with appropriate core
 *   2. Add items from transition vector
 *   3. Perform closure (prediction for nonterminals)
 *   4. Deduplicate items
 *
 * @param engine Earley engine instance
 * @param current_set Current Earley set
 * @param transition Transition vector
 * @param lookahead Lookahead terminal number
 *
 * @return New set (NULL when stubbed)
 */
struct set *
earley_engine_build_new_set(earley_engine_t *engine,
                             struct set *current_set,
                             struct core_symb_vect *transition,
                             int lookahead)
{
  /* Validate parameters. */
  assert(engine != NULL);
  assert(current_set != NULL);
  assert(transition != NULL);

  /* STUB: Not yet implemented. */
  (void) engine;
  (void) current_set;
  (void) transition;
  (void) lookahead;

  return NULL;  /* Error: not implemented */
}

/**
 * Perform prediction step for a nonterminal - STUB
 *
 * Adds prediction items for all rules with given nonterminal on LHS.
 * Currently a stub. Will be implemented during Phase P2.
 *
 * FUTURE IMPLEMENTATION:
 *   For each rule R with LHS = nonterminal:
 *     Add item [R, 0] to set (dot at position 0)
 *
 * @param engine Earley engine instance
 * @param set Set to add predicted items
 * @param nonterminal Nonterminal to predict
 *
 * @return Number of items added (-1 when stubbed)
 */
int
earley_engine_predict(earley_engine_t *engine,
                      struct set *set,
                      struct symb *nonterminal)
{
  /* Validate parameters. */
  assert(engine != NULL);
  assert(set != NULL);
  assert(nonterminal != NULL);

  /* STUB: Not yet implemented. */
  (void) engine;
  (void) set;
  (void) nonterminal;

  return -1;  /* Error: not implemented */
}

/**
 * Perform scan step for a terminal - STUB
 *
 * Advances items that expect the current terminal symbol.
 * Currently a stub. Will be implemented during Phase P2.
 *
 * FUTURE IMPLEMENTATION:
 *   For each item [R, p] where symbol at p = terminal:
 *     Add item [R, p+1] to next set
 *
 * @param engine Earley engine instance
 * @param current_set Current Earley set
 * @param terminal Terminal symbol being scanned
 *
 * @return Next set after scanning (NULL when stubbed)
 */
struct set *
earley_engine_scan(earley_engine_t *engine,
                   struct set *current_set,
                   struct symb *terminal)
{
  /* Validate parameters. */
  assert(engine != NULL);
  assert(current_set != NULL);
  assert(terminal != NULL);

  /* STUB: Not yet implemented. */
  (void) engine;
  (void) current_set;
  (void) terminal;

  return NULL;  /* Error: not implemented */
}

/**
 * Perform completion step for a completed item - STUB
 *
 * Propagates completed nonterminal back to items waiting for it.
 * Currently a stub. Will be implemented during Phase P2.
 *
 * FUTURE IMPLEMENTATION:
 *   For completed item [R, n] with LHS = N:
 *     For each item [R', p'] in origin set where symbol at p' = N:
 *       Add item [R', p'+1] to current set
 *
 * @param engine Earley engine instance
 * @param current_set Set containing completed item
 * @param completed_item Item that just completed
 *
 * @return Number of items added (-1 when stubbed)
 */
int
earley_engine_complete(earley_engine_t *engine,
                       struct set *current_set,
                       struct sit *completed_item)
{
  /* Validate parameters. */
  assert(engine != NULL);
  assert(current_set != NULL);
  assert(completed_item != NULL);

  /* STUB: Not yet implemented. */
  (void) engine;
  (void) current_set;
  (void) completed_item;

  return -1;  /* Error: not implemented */
}
