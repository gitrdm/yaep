/**
 * @file mem_pool.h
 * @brief Memory pool allocator for high-frequency allocations
 *
 * PURPOSE:
 * Reduces malloc/free overhead for frequently allocated objects during parsing.
 * Earley parsing creates thousands of short-lived situations and sets, making
 * malloc overhead a bottleneck. Memory pools pre-allocate blocks and reuse them.
 *
 * DESIGN:
 * - Fixed-size allocator: All items in a pool have the same size
 * - Block allocation: Pools grow by allocating blocks of items
 * - Free list: Freed items are linked in a free list for O(1) reuse
 * - Custom allocator support: Uses YaepAllocator for underlying allocation
 *
 * USAGE EXAMPLE:
 * ```c
 * mem_pool_t *pool = pool_create(alloc, sizeof(struct sit), 256);
 * struct sit *item1 = pool_alloc(pool);
 * struct sit *item2 = pool_alloc(pool);
 * pool_free(pool, item1);
 * struct sit *item3 = pool_alloc(pool);  // Reuses item1's memory
 * pool_destroy(pool);
 * ```
 *
 * PERFORMANCE:
 * - Allocation: O(1) amortized (free list or bump pointer)
 * - Deallocation: O(1) (add to free list)
 * - Memory overhead: ~8 bytes per block + free list pointers
 *
 * THREAD SAFETY:
 * - NOT thread-safe: Each thread should have its own pool
 * - Pools can be shared if externally synchronized
 *
 * INTEGRATION (P3-005):
 * - Replace malloc() in sit_alloc(), set_alloc() with pool_alloc()
 * - Replace free() with pool_free()
 * - Create pools at parse start, destroy at parse end
 *
 * REFERENCES:
 * - "The Art of Computer Programming" Vol 1, Knuth (memory management)
 * - tcmalloc design: Fast path for thread-local allocation
 * - APR memory pools: Apache Portable Runtime design patterns
 *
 * @author YAEP Project
 * @date October 9, 2025
 */

#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#include "allocate.h"  /* For YaepAllocator */
#include <stddef.h>    /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Memory pool structure - Opaque handle
 *
 * Internal details hidden in mem_pool.c to allow implementation changes.
 * Clients only use the typedef pointer.
 */
typedef struct mem_pool mem_pool_t;

/**
 * pool_create - Create a new memory pool
 *
 * PURPOSE:
 * Allocates and initializes a memory pool for fixed-size items.
 *
 * ALGORITHM:
 * 1. Allocate pool structure
 * 2. Initialize free list (empty initially)
 * 3. Pre-allocate first block of items (optional optimization)
 * 4. Return pool handle
 *
 * COMPLEXITY:
 * - Time: O(items_per_block) if pre-allocating, O(1) otherwise
 * - Space: O(items_per_block * item_size) for first block
 *
 * @param alloc Custom allocator for pool metadata and blocks (NULL = malloc)
 * @param item_size Size of each item in bytes (must be > 0, typically sizeof(struct))
 * @param items_per_block Number of items per block (recommended: 128-1024)
 *                        Trade-off: Larger = fewer malloc calls but more wasted space
 *
 * @return Pool handle on success, NULL on allocation failure
 *
 * @note item_size is rounded up to ensure proper alignment for pointers
 * @note items_per_block affects memory fragmentation vs allocation overhead
 *
 * TYPICAL VALUES:
 * - item_size: sizeof(struct sit) ~= 64 bytes, sizeof(struct set) ~= 128 bytes
 * - items_per_block: 256 (16KB blocks) for small items, 128 (16KB) for large items
 *
 * TESTING:
 * - See test/test_mem_pool.c: test_pool_create_destroy()
 */
mem_pool_t *pool_create(YaepAllocator *alloc, size_t item_size, size_t items_per_block);

/**
 * pool_alloc - Allocate one item from the pool
 *
 * PURPOSE:
 * Fast O(1) allocation from memory pool, reusing freed items or allocating new blocks.
 *
 * ALGORITHM:
 * 1. If free list is not empty:
 *    a. Pop item from free list
 *    b. Return item
 * 2. If current block has space:
 *    a. Bump pointer to next item
 *    b. Return item
 * 3. Allocate new block:
 *    a. Call alloc->malloc(items_per_block * item_size)
 *    b. Link block into block list
 *    c. Initialize bump pointer to block start
 *    d. Return first item from new block
 *
 * COMPLEXITY:
 * - Time: O(1) amortized (new block allocation is rare)
 * - Space: O(1) per call (blocks allocated lazily)
 *
 * @param pool Pool handle from pool_create()
 *
 * @return Pointer to allocated item, NULL on allocation failure
 *
 * @note Returned memory is NOT zeroed (client must initialize)
 * @note Alignment is guaranteed to be suitable for any type
 *
 * ERROR HANDLING:
 * - Returns NULL if pool is NULL
 * - Returns NULL if underlying allocator fails
 * - Caller must check for NULL and handle gracefully
 *
 * TESTING:
 * - See test/test_mem_pool.c: test_pool_alloc_basic(), test_pool_alloc_many()
 */
void *pool_alloc(mem_pool_t *pool);

/**
 * pool_free - Return an item to the pool
 *
 * PURPOSE:
 * Fast O(1) deallocation by adding item to free list for reuse.
 *
 * ALGORITHM:
 * 1. Typecast item to free list node
 * 2. Set node->next = pool->free_list
 * 3. Set pool->free_list = node
 *
 * COMPLEXITY:
 * - Time: O(1) exact
 * - Space: O(1) (no additional memory allocated)
 *
 * @param pool Pool handle from pool_create()
 * @param item Pointer to item previously allocated from this pool
 *
 * @note item MUST have been allocated from this pool (undefined behavior otherwise)
 * @note item is NOT zeroed or validated (client responsibility)
 * @note Freeing NULL is a no-op (safe)
 * @note Double-free is NOT detected (client must avoid)
 *
 * RATIONALE:
 * Free list reuse avoids calling underlying free(), which is slow for small objects.
 * Items are never returned to the OS until pool_destroy(), trading memory for speed.
 *
 * TESTING:
 * - See test/test_mem_pool.c: test_pool_free_reuse(), test_pool_free_patterns()
 */
void pool_free(mem_pool_t *pool, void *item);

/**
 * pool_destroy - Destroy memory pool and free all blocks
 *
 * PURPOSE:
 * Releases all memory allocated by the pool. Must be called when pool is no longer needed.
 *
 * ALGORITHM:
 * 1. Iterate through block list
 * 2. Free each block via alloc->free()
 * 3. Free pool structure
 *
 * COMPLEXITY:
 * - Time: O(num_blocks) where num_blocks = total_allocated / items_per_block
 * - Space: O(1) (frees memory)
 *
 * @param pool Pool handle from pool_create()
 *
 * @note All items allocated from pool become INVALID after this call
 * @note Client must ensure no references to pool items exist
 * @note Destroying NULL pool is a no-op (safe)
 *
 * LIFECYCLE:
 * - Create: pool_create() at start of parse
 * - Use: pool_alloc() / pool_free() during parse
 * - Destroy: pool_destroy() at end of parse (success or error)
 *
 * TESTING:
 * - See test/test_mem_pool.c: test_pool_create_destroy(), valgrind leak tests
 */
void pool_destroy(mem_pool_t *pool);

/**
 * pool_stats - Get pool statistics (for debugging/profiling)
 *
 * PURPOSE:
 * Returns pool usage statistics for performance analysis and debugging.
 *
 * @param pool Pool handle
 * @param total_allocated_out [out] Total items allocated (alloc calls)
 * @param total_freed_out [out] Total items freed (free calls)
 * @param blocks_allocated_out [out] Total blocks allocated
 * @param free_list_length_out [out] Current free list length
 *
 * @note Pass NULL for any output parameter to skip that statistic
 * @note Useful for tracking pool efficiency and memory usage
 *
 * TESTING:
 * - See test/test_mem_pool.c: test_pool_stats()
 */
void pool_stats(mem_pool_t *pool,
                size_t *total_allocated_out,
                size_t *total_freed_out,
                size_t *blocks_allocated_out,
                size_t *free_list_length_out);

#ifdef __cplusplus
}
#endif

#endif /* __MEM_POOL_H__ */
