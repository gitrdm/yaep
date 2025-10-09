/*
   YAEP (Yet Another Earley Parser) - Leo Optimization Module

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
 * @file leo_opt.h
 * @brief Leo's Right-Recursion Optimization for Earley Parsing
 *
 * ALGORITHM:
 *   Implements Leo (1991) optimization that reduces complexity from O(n³) 
 *   to O(n²) for right-recursive grammars, and from O(n²) to O(n) for 
 *   LR(k) grammars.
 *
 * PRINCIPLE:
 *   When completing a nonterminal with exactly ONE parent item waiting,
 *   we can create a "Leo item" representing the entire deterministic 
 *   reduction chain, eliminating intermediate items.
 *
 * EXAMPLE:
 *   Grammar: S → S 'a' | 'b'
 *   Input: b a a a
 *   
 *   Without Leo: Creates O(n²) items (quadratic growth)
 *   With Leo:    Creates O(n) items (linear growth)
 *
 * REFERENCE:
 *   Leo, Joop (1991). "A general context-free parsing algorithm running
 *   in linear time on every LR(k) grammar without using lookahead"
 *   Theoretical Computer Science 82(1): 165-176.
 *
 * PHASE: P4 - Leo's Right-Recursion Optimization
 */

#ifndef LEO_OPT_H
#define LEO_OPT_H

#include "yaep.h"
#include "hashtab.h"
#include "objstack.h"

/* Forward declarations for internal YAEP types */
struct sit;
struct symb;
struct set;
struct core_symb_vect;
struct grammar;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Leo item - Represents topmost item in deterministic reduction path
 *
 * PURPOSE:
 *   Eliminates redundant intermediate items in right-recursive derivations.
 *   A Leo item caches the "topmost" situation after a deterministic 
 *   completion chain.
 *
 * STORAGE:
 *   Indexed by (set_number, nonterminal_symbol) in a hash table.
 *   Only created when exactly ONE item is waiting for the completed nonterminal.
 */
struct leo_item {
  struct sit *sit;         /**< Topmost situation in reduction chain */
  int origin;              /**< Origin set index for this derivation */
  struct symb *symbol;     /**< Completed nonterminal symbol (hash key) */
  struct leo_item *next;   /**< Hash table collision chain */
};

/**
 * Leo optimization context - Encapsulates all Leo state
 *
 * PURPOSE:
 *   Groups all Leo-related data structures for clean initialization,
 *   cleanup, and enable/disable toggling.
 */
struct leo_context {
  int enabled;                  /**< Is Leo optimization enabled? */
  int initialized;              /**< Has leo_init() been called? */
  
  /* Statistics */
  int n_leo_items;              /**< Total Leo items created */
  int n_leo_completions;        /**< Completions that used Leo items */
  
  /* Storage */
#ifndef __cplusplus
  os_t leo_items_os;            /**< Object stack for Leo item allocation */
#else
  os_t *leo_items_os;           /**< Object stack (pointer in C++ mode) */
#endif
  hash_table_t leo_item_table;  /**< Hash table: (set_num, symbol) → leo_item */
  
  /* Allocator reference (not owned) */
  YaepAllocator *alloc;         /**< Allocator for Leo structures */
};

/**
 * leo_init - Initialize Leo optimization infrastructure
 *
 * @param ctx Leo context to initialize
 * @param alloc Allocator to use for Leo data structures
 *
 * Must be called once per grammar initialization.
 * Sets ctx->initialized = 1 if successful.
 */
extern void leo_init(struct leo_context *ctx, YaepAllocator *alloc);

/**
 * leo_clear - Clear Leo state for new parse
 *
 * @param ctx Leo context to clear
 *
 * Resets Leo optimization state at the start of each parse.
 * Leo items are parse-specific, so we clear the table for each new input.
 */
extern void leo_clear(struct leo_context *ctx);

/**
 * leo_finish - Cleanup Leo resources
 *
 * @param ctx Leo context to destroy
 *
 * Destroys Leo optimization data structures.
 * Called when grammar is freed.
 * Sets ctx->initialized = 0.
 */
extern void leo_finish(struct leo_context *ctx);

/**
 * leo_try_completion - Attempt Leo optimization for completion
 *
 * PURPOSE:
 *   Checks if Leo conditions are met for a completed nonterminal.
 *   If so, creates/uses Leo item instead of standard Earley completion.
 *
 * ALGORITHM:
 *   1. Check if exactly ONE item waiting for completed nonterminal
 *   2. If so, check if this creates deterministic reduction path
 *   3. If conditions met, create/update Leo item
 *   4. Use Leo item to skip intermediate items (jump to topmost)
 *
 * @param ctx Leo context
 * @param completed_sit The just-completed situation
 * @param origin_set The set where completion originated
 * @param origin_set_num The number of the origin set  
 * @param current_set_num The number of the current set
 * @param waiting_vect Transition vector of items waiting for this nonterminal
 * @param lookahead_term_num Current lookahead terminal (-1 if none)
 *
 * @return 1 if Leo handled the completion, 0 for fallback to standard
 *
 * SIDE EFFECTS:
 *   - May create new Leo item in ctx->leo_item_table
 *   - May add situations to current Earley set (via callback)
 *   - Updates ctx->n_leo_items and ctx->n_leo_completions
 */
extern int leo_try_completion(struct leo_context *ctx,
                               struct sit *completed_sit,
                               struct set *origin_set,
                               int origin_set_num,
                               int current_set_num,
                               struct core_symb_vect *waiting_vect,
                               int lookahead_term_num);

/**
 * leo_get_stats - Get Leo optimization statistics
 *
 * @param ctx Leo context
 * @param n_items Output: number of Leo items created
 * @param n_completions Output: number of Leo completions performed
 */
extern void leo_get_stats(struct leo_context *ctx, 
                          int *n_items, 
                          int *n_completions);

#ifdef __cplusplus
}
#endif

#endif /* LEO_OPT_H */
