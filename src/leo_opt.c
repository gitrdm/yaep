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
 * @file leo_opt.c
 * @brief Leo's Right-Recursion Optimization - Implementation
 *
 * PHASE: P4 - Leo's Right-Recursion Optimization
 */

#include "leo_opt.h"
#include "yaep_internal.h"  /* For core_symb_vect, sit, rule definitions */
#include "allocate.h"
#include "objstack.h"
#include "hashtab.h"
#include "yaep_macros.h"
#include <assert.h>
#include <stdint.h>

static unsigned int
leo_hash_entry (hash_table_entry_t entry)
{
  const struct leo_item *item = YAEP_STATIC_CAST(const struct leo_item *, entry);
  uintptr_t sym = YAEP_STATIC_CAST(uintptr_t, item->symbol);
  unsigned int hash = YAEP_STATIC_CAST(unsigned int, item->origin);
  hash ^= YAEP_STATIC_CAST(unsigned int, sym >> 3);
  /* Mix bits to reduce clustering */
  hash *= 16777619U;
  return hash;
}

static int
leo_eq_entry (hash_table_entry_t lhs, hash_table_entry_t rhs)
{
  const struct leo_item *a = YAEP_STATIC_CAST(const struct leo_item *, lhs);
  const struct leo_item *b = YAEP_STATIC_CAST(const struct leo_item *, rhs);
  return a->origin == b->origin && a->symbol == b->symbol;
}

/**
 * leo_init - Initialize Leo optimization infrastructure
 */
void
leo_init(struct leo_context *ctx, YaepAllocator *alloc)
{
  assert(ctx != NULL);
  assert(alloc != NULL);
  
  /* Store allocator reference */
  ctx->alloc = alloc;
  ctx->enabled = 1;  /* Enabled by default */
  
  /* Initialize statistics */
  ctx->n_leo_items = 0;
  ctx->n_leo_completions = 0;
  
  /* Create object stack for Leo item allocation */
#ifndef __cplusplus
  /* C mode: Use macro that calls function */
  OS_CREATE(ctx->leo_items_os, alloc, 0);
#else
  /* C++ mode: Use constructor */
  ctx->leo_items_os = new os(alloc, 0);
#endif
  
  /* Create hash table for Leo items */
#ifndef __cplusplus
  /* C mode: Function returns hash_table_t value */
  ctx->leo_item_table = create_hash_table(alloc, 1024, leo_hash_entry, leo_eq_entry);
#else
  /* C++ mode: Constructor returns pointer */
  ctx->leo_item_table = new hash_table(alloc, 1024, leo_hash_entry, leo_eq_entry);
#endif
  
  /* Mark as initialized */
  ctx->initialized = 1;
}

/**
 * leo_clear - Clear Leo state for new parse
 */
void
leo_clear(struct leo_context *ctx)
{
  if (!ctx || !ctx->initialized)
    return;
  
  /* Reset statistics */
  ctx->n_leo_items = 0;
  ctx->n_leo_completions = 0;
  
  /* Clear object stack (bulk deallocation) */
#ifndef __cplusplus
  /* C mode: Use macro */
  OS_EMPTY(ctx->leo_items_os);
#else
  /* C++ mode: Call method */
  ctx->leo_items_os->empty();
#endif
  
  /* Clear hash table */
#ifndef __cplusplus
  /* C mode: Function */
  empty_hash_table(ctx->leo_item_table);
#else
  /* C++ mode: Method */
  ctx->leo_item_table->empty();
#endif
}

/**
 * leo_finish - Cleanup Leo resources
 */
void
leo_finish(struct leo_context *ctx)
{
  if (!ctx || !ctx->initialized)
    return;
  
  /* Destroy object stack */
#ifndef __cplusplus
  /* C mode: Use macro */
  OS_DELETE(ctx->leo_items_os);
#else
  /* C++ mode: Delete pointer */
  delete ctx->leo_items_os;
  ctx->leo_items_os = NULL;
#endif
  
  /* Destroy hash table */
#ifndef __cplusplus
  /* C mode: Function */
  delete_hash_table(ctx->leo_item_table);
#else
  /* C++ mode: Delete pointer */
  delete ctx->leo_item_table;
  ctx->leo_item_table = NULL;
#endif
  
  /* Mark as uninitialized */
  ctx->initialized = 0;
  ctx->alloc = NULL;
}

/**
 * leo_lookup - Look up a Leo item in the hash table
 *
 * Search for a Leo item with the given (set_num, symbol) key.
 *
 * PARAMETERS:
 *   ctx      - Leo optimization context
 *   set_num  - Set number where completion occurred
 *   symbol   - Completed nonterminal symbol
 *
 * RETURNS:
 *   Pointer to leo_item if found, NULL otherwise
 */
static struct leo_item *
leo_lookup(struct leo_context *ctx, int set_num, struct symb *symbol)
{
  struct leo_item key;
  hash_table_entry_t *slot;

  if (!ctx || !ctx->initialized || !symbol)
    return NULL;

  key.origin = set_num;
  key.symbol = symbol;
  key.sit = NULL;
  key.next = NULL;

#ifndef __cplusplus
  slot = find_hash_table_entry(ctx->leo_item_table,
                               YAEP_STATIC_CAST(hash_table_entry_t, &key),
                               0);
  if (slot == NULL || *slot == NULL)
    return NULL;
  return YAEP_STATIC_CAST(struct leo_item *, *slot);
#else
  slot = ctx->leo_item_table->find_entry(&key, 0);
  if (slot == NULL || *slot == NULL)
    return NULL;
  return static_cast<struct leo_item *>(*slot);
#endif
}

/**
 * leo_insert - Insert a new Leo item into hash table
 *
 * Creates a new Leo item representing a deterministic completion.
 * The item stores the waiting situation that should be advanced
 * when this (set, symbol) completion is seen again.
 *
 * PARAMETERS:
 *   ctx          - Leo optimization context
 *   set_num      - Set number where completion occurred
 *   symbol       - Completed nonterminal symbol
 *   waiting_sit  - Situation waiting for this symbol
 *   lookahead    - Lookahead terminal number (unused, kept for future)
 *
 * RETURNS:
 *   Pointer to newly created leo_item, or NULL on allocation failure
 */
static struct leo_item *
leo_insert(struct leo_context *ctx, int set_num, struct symb *symbol,
           struct sit *waiting_sit, int lookahead)
{
  struct leo_item *new_item;
  hash_table_entry_t *slot;
  struct leo_item key;
  
  (void)lookahead; /* Unused for now, kept for future use */
  
  if (!ctx || !ctx->initialized || !symbol || !waiting_sit)
    return NULL;
  
  /* Allocate new Leo item from object stack */
#ifndef __cplusplus
  /* C mode: Use macro that returns void* */
  OS_TOP_EXPAND(ctx->leo_items_os, sizeof(struct leo_item));
  new_item = (struct leo_item *)OS_TOP_BEGIN(ctx->leo_items_os);
  OS_TOP_FINISH(ctx->leo_items_os);
#else
  /* C++ mode: Call method and get pointer */
  ctx->leo_items_os->top_expand(sizeof(struct leo_item));
  new_item = static_cast<struct leo_item *>(ctx->leo_items_os->top_begin());
  ctx->leo_items_os->top_finish();
#endif
  
  if (!new_item)
    return NULL; /* Allocation failed */
  
  /* Initialize the new Leo item */
  new_item->origin = set_num;
  new_item->symbol = symbol;
  new_item->sit = waiting_sit;
  new_item->next = NULL;

  key.origin = set_num;
  key.symbol = symbol;
  key.sit = NULL;
  key.next = NULL;

#ifndef __cplusplus
  slot = find_hash_table_entry(ctx->leo_item_table,
                               YAEP_STATIC_CAST(hash_table_entry_t, &key),
                               1);
  if (slot == NULL)
    return NULL;
  *slot = YAEP_STATIC_CAST(hash_table_entry_t, new_item);
#else
  slot = ctx->leo_item_table->find_entry(&key, 1);
  if (slot == NULL)
    return NULL;
  *slot = static_cast<hash_table_entry_t>(new_item);
#endif
  
  /* Update statistics */
  ctx->n_leo_items++;
  
  return new_item;
}


/**
 * leo_try_completion - Attempt Leo optimization for completed item
 *
 * PURPOSE:
 *   Checks if Leo optimization applies (deterministic completion with exactly
 *   ONE waiting item). If deterministic, marks it for future optimization.
 *
 * ALGORITHM:
 *   1. Check if exactly ONE item is waiting for the completed nonterminal
 *   2. If so, this is a deterministic completion
 *   3. For now, just count it (full optimization in P4-004)
 *   4. Return 0 to proceed with standard completion
 *
 * PARAMETERS:
 *   ctx              - Leo optimization context
 *   completed_sit    - Completed situation [A → α ., i]
 *   origin_set       - Set where completion started (set i)
 *   origin_set_num   - Index of origin set
 *   current_set_num  - Index of current set
 *   waiting_vect     - Items waiting for completed nonterminal
 *   lookahead_term_num - Current lookahead terminal (or -1)
 *
 * RETURNS:
 *   0 (always fallback to standard completion for now)
 */
int
leo_try_completion(struct leo_context *ctx,
                   struct sit *completed_sit,
                   struct set *origin_set,
                   int origin_set_num,
                   int current_set_num,
                   struct core_symb_vect *waiting_vect,
                   int lookahead_term_num)
{
  int n_waiting;
  struct leo_item *existing, *new_item;
  struct symb *completed_symbol;
  int *waiting_sit_indices;
  struct sit *waiting_sit;
  
  /* Suppress unused parameter - reserved for future use */
  (void)origin_set_num;
  
  /* Check if Leo optimization is enabled and initialized */
  if (!ctx || !ctx->initialized || !ctx->enabled)
    return 0;
  
  /* Sanity check */
  if (!waiting_vect || !completed_sit)
    return 0;
  
  /* LEO CONDITION: Must have exactly ONE waiting item (determinism)
   * 
   * If multiple items are waiting for this nonterminal, the parse is
   * non-deterministic and Leo optimization does not apply. */
#ifdef TRANSITIVE_TRANSITION
  n_waiting = waiting_vect->transitive_transitions.len;
#ifndef __cplusplus
  waiting_sit_indices = (int *)waiting_vect->transitive_transitions.els;
#else
  waiting_sit_indices = const_cast<int *>(waiting_vect->transitive_transitions.els);
#endif
#else
  n_waiting = waiting_vect->transitions.len;
#ifndef __cplusplus
  waiting_sit_indices = (int *)waiting_vect->transitions.els;
#else
  waiting_sit_indices = const_cast<int *>(waiting_vect->transitions.els);
#endif
#endif
  
  /* Not deterministic? Fall back to standard completion */
  if (n_waiting != 1)
    return 0;
  
  /* DETERMINISTIC CASE: Exactly one waiting item - count it! */
  ctx->n_leo_completions++;
  
  /* Get the completed nonterminal symbol */
  completed_symbol = completed_sit->rule->lhs;
  
  /* Check if a Leo item already exists for this (set, symbol) pair.
   * This handles recursive Leo completions (A → B., B → C., etc.) */
  existing = leo_lookup(ctx, current_set_num, completed_symbol);
  if (existing)
    {
      /* Leo item already exists from a previous completion.
       * The standard completion would propagate this item to the
       * waiting item's position. Instead, we skip it - the existing
       * Leo item already encapsulates the completion chain. */
      return 1; /* Skip standard completion */
    }
  
  /* Get the single waiting situation */
#ifndef __cplusplus
  waiting_sit = (struct sit *)origin_set->core->sits[waiting_sit_indices[0]];
#else
  waiting_sit = const_cast<struct sit *>(origin_set->core->sits[waiting_sit_indices[0]]);
#endif
  
  /* Create a new Leo item: [B → α A . β, i, lookahead]
   * This represents: "If we see A completed in set i, we can
   * immediately jump to this waiting item's next position." */
  new_item = leo_insert(ctx, current_set_num, completed_symbol,
                        waiting_sit, lookahead_term_num);
  
  if (!new_item)
    {
      /* Memory allocation failed. Fall back to standard completion
       * to maintain correctness (slower, but correct). */
      return 0;
    }
  
  /* SUCCESS: Leo item created. Skip standard completion.
   * The Leo item will be used in future completions to avoid
   * redundant work. This reduces the parsing complexity from
   * O(n³) to O(n²) for right-recursive grammars. */
  return 1;
}

/**
 * leo_get_stats - Get Leo optimization statistics
 */
void
leo_get_stats(struct leo_context *ctx, int *n_items, int *n_completions)
{
  if (!ctx || !ctx->initialized)
    {
      if (n_items) *n_items = 0;
      if (n_completions) *n_completions = 0;
      return;
    }
  
  if (n_items) *n_items = ctx->n_leo_items;
  if (n_completions) *n_completions = ctx->n_leo_completions;
}
