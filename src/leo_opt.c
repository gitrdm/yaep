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
  /* Suppress unused parameter warnings */
  (void)ctx;
  (void)completed_sit;
  (void)origin_set;
  (void)origin_set_num;
  (void)current_set_num;
  (void)waiting_vect;
  (void)lookahead_term_num;
  
  /* P4-001: Infrastructure only - no actual Leo optimization yet
   * Return 0 to fallback to standard Earley completion */
  return 0;
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
