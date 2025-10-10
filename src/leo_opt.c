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
#include "allocate.h"
#include "objstack.h"
#include "hashtab.h"
#include "yaep_macros.h"
#include <assert.h>
#include <stdio.h>

/* Cross-language cast helper: use C++ reinterpret_cast where available to
 * avoid -Wold-style-cast when this C file is compiled/linked into C++ TUs. */
#ifdef __cplusplus
# define LEO_REINTERPRET_CAST(type, expr) reinterpret_cast<type>(expr)
#else
# define LEO_REINTERPRET_CAST(type, expr) ((type)(expr))
#endif

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
  ctx->leo_item_table = create_hash_table(alloc, 1000, NULL, NULL);
#else
  /* C++ mode: Constructor returns pointer */
  ctx->leo_item_table = new hash_table(alloc, 1000, NULL, NULL);
#endif
  
  /* Mark as initialized */
  ctx->initialized = 1;
  ctx->debug_enabled = 0;
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

void
leo_set_debug_enabled(struct leo_context *ctx, int enabled)
{
  if (!ctx || !ctx->initialized)
    return;
  ctx->debug_enabled = enabled ? 1 : 0;
}

/**
 * leo_try_completion - Attempt Leo optimization (STUB for P4-001)
 *
 * IMPLEMENTATION STATUS:
 *   P4-001: Data structures only - always returns 0 (fallback to standard)
 *   P4-002: Will implement hash table operations
 *   P4-003: Will implement detection logic
 *   P4-004: Will implement actual Leo completion
 *
 * This stub allows the code to compile and pass all tests while we build
 * out the Leo infrastructure incrementally.
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
  /* Silence unused-parameter warnings for parameters that are not yet
   * used by this conservative implementation. */
  (void) origin_set_num;
  (void) current_set_num;
  (void) lookahead_term_num;

  /* Conservative P4-002 implementation: handle single-waiter case only.
   * If exactly one item in origin_set is waiting for this completed
   * nonterminal, create the advanced situation and add it to the
   * in-progress new set, bypassing the full completion loop.
   *
   * Notes:
   * - This implementation is intentionally minimal and relies only on
   *   the accessor functions exported from yaep.c (see leo_opt.h).
   * - If any step fails, return 0 so caller falls back to standard
   *   Earley completion. */

  int n_waiters, waiter_idx;
  struct sit *parent_sit;
  struct sit *new_sit;
  int parent_dist;

  if (!ctx || !ctx->initialized || !ctx->enabled)
    return 0;

  if (!waiting_vect || !completed_sit || !origin_set)
    return 0;

  /* Optional runtime debug printing. Guarded by ctx->debug_enabled so
   * the calls are cheap when disabled. */
  if (ctx && ctx->debug_enabled)
    fprintf(stderr, "[LEO] try_completion: origin_set=%p origin_set_num=%d current_set_num=%d\n",
            LEO_REINTERPRET_CAST(const void *, origin_set), origin_set_num, current_set_num);

  /* Only handle the simple case where exactly one item waits for the
   * completed nonterminal. */
  n_waiters = yaep_core_symb_vect_transition_len(waiting_vect);
  if (n_waiters != 1)
    {
#ifdef LEO_DEBUG
      fprintf(stderr, "[LEO] n_waiters=%d not equal 1, skipping\n", n_waiters);
#endif
      return 0;
    }

  /* Get the waiting situation index (index into origin_set->core->sits). */
  waiter_idx = yaep_core_symb_vect_transition_el(waiting_vect, 0);
  if (waiter_idx < 0)
    {
#ifdef LEO_DEBUG
      fprintf(stderr, "[LEO] invalid waiter_idx=%d\n", waiter_idx);
#endif
      return 0;
    }

  /* Get the parent sit pointer from the origin set core. */
  parent_sit = yaep_prev_set_core_sit_at(origin_set, waiter_idx);
  if (!parent_sit)
    {
#ifdef LEO_DEBUG
      fprintf(stderr, "[LEO] parent_sit is NULL for waiter_idx=%d\n", waiter_idx);
#endif
      return 0;
    }

  /* Create the advanced situation: parent_sit's rule, pos+1, same context.
   * Use yaep_sit_create to leverage YAEP's deduplication. Use the
   * lightweight accessors exposed by yaep.c to avoid depending on
   * internal struct layouts. */
  {
    struct rule *r = yaep_sit_rule(parent_sit);
    int pos = yaep_sit_pos(parent_sit);
    int sit_ctx = yaep_sit_context(parent_sit);

    if (!r || pos < 0 || sit_ctx < 0)
      {
#ifdef LEO_DEBUG
        fprintf(stderr, "[LEO] invalid parent_sit fields r=%p pos=%d ctx=%d\n",
                (void *) r, pos, sit_ctx);
#endif
        return 0;
      }

    new_sit = yaep_sit_create(r, pos + 1, sit_ctx);
  }
  if (!new_sit)
    {
#ifdef LEO_DEBUG
      fprintf(stderr, "[LEO] yaep_sit_create returned NULL\n");
#endif
      return 0;
    }

  /* Compute distance for parent tracking and add to the in-progress set. */
  parent_dist = yaep_compute_parent_dist(origin_set, waiter_idx);
  /* A non-positive parent distance is not safe to use here. The
   * YAEP accessor may return 0 to indicate "unknown/default" for
   * certain core indexes; inserting a start sit with distance 0
   * leads to place = pl_curr + 1 - 0 which can index a not-yet-
   * allocated parser list entry (and cause a crash). Treat <= 0
   * as failure and fall back to standard completion. */
  if (parent_dist <= 0)
    {
#ifdef LEO_DEBUG
      fprintf(stderr, "[LEO] parent_dist=%d not usable\n", parent_dist);
#endif
      return 0;
    }

  if (!yaep_set_new_add_start_sit_wrapper(new_sit, parent_dist))
    {
#ifdef LEO_DEBUG
      fprintf(stderr, "[LEO] yaep_set_new_add_start_sit_wrapper failed\n");
#endif
      return 0;
    }

  /* Update stats */
  ctx->n_leo_items++;
  ctx->n_leo_completions++;

  /* Indicate we handled the completion */
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
