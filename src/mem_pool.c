/**
 * @file mem_pool.c
 * @brief Memory pool allocator implementation
 *
 * IMPLEMENTATION NOTES:
 * - Free list uses intrusive linking (first pointer-size bytes of freed items)
 * - Blocks are allocated lazily (on-demand)
 * - No bounds checking on pool_free() for performance
 * - Block list is singly-linked for O(1) insertion
 *
 * @author YAEP Project
 * @date October 9, 2025
 */

#include "mem_pool.h"
#include "allocate.h"
#include <assert.h>
#include <stdlib.h>  /* For malloc, free */
#include <string.h>  /* For memset */
#include <stdint.h>  /* For uintptr_t */

/**
 * mem_block - Internal structure for memory blocks
 *
 * Each block contains items_per_block items of item_size bytes each.
 * Blocks are linked together for cleanup during pool_destroy().
 */
struct mem_block {
    struct mem_block *next;  /**< Next block in list (NULL if last) */
    /* Items follow immediately after this header */
};

/**
 * mem_pool - Memory pool implementation
 *
 * LAYOUT:
 * - Pool metadata (this struct)
 * - Block list: Linked list of allocated blocks
 * - Free list: Linked list of freed items (intrusive)
 * - Current block: Active block for bump-pointer allocation
 *
 * ALLOCATION STRATEGY:
 * 1. Try free list first (if non-empty)
 * 2. Try bump pointer in current block
 * 3. Allocate new block and use bump pointer
 *
 * MEMORY OWNERSHIP:
 * - Pool owns all blocks (freed in pool_destroy)
 * - Allocator owns pool structure itself
 */
struct mem_pool {
    /* Allocator for blocks and pool structure */
    YaepAllocator *alloc;
    
    /* Configuration (set at creation, never changed) */
    size_t item_size;         /**< Size of each item (aligned) */
    size_t items_per_block;   /**< Items per block */
    
    /* Block management */
    struct mem_block *blocks;       /**< Linked list of all blocks */
    struct mem_block *current_block; /**< Current block for bump allocation */
    size_t current_offset;          /**< Offset in current block (in items) */
    
    /* Free list (for reuse) */
    void *free_list;  /**< Intrusive singly-linked list of freed items */
    
    /* Statistics (for debugging/profiling) */
    size_t total_allocated;    /**< Total pool_alloc() calls */
    size_t total_freed;        /**< Total pool_free() calls */
    size_t blocks_allocated;   /**< Total blocks allocated */
};

/**
 * Free list node - Intrusive linking in freed items
 *
 * We reuse the first sizeof(void*) bytes of freed items to store the next pointer.
 * This requires item_size >= sizeof(void*), enforced in pool_create().
 */
struct free_node {
    struct free_node *next;
};

/**
 * Align size up to pointer alignment
 *
 * Ensures all items are properly aligned for any type.
 *
 * @param size Size to align
 * @return Aligned size (multiple of sizeof(void*))
 */
static size_t
align_size(size_t size)
{
    const size_t alignment = sizeof(void*);
    return (size + alignment - 1) & ~(alignment - 1);
}

/**
 * pool_create - Create memory pool
 *
 * Implementation follows design in mem_pool.h header documentation.
 */
mem_pool_t *
pool_create(YaepAllocator *alloc, size_t item_size, size_t items_per_block)
{
    mem_pool_t *pool = NULL;
    
    /* Validate parameters */
    if (item_size == 0 || items_per_block == 0)
        return NULL;
    
    /* Align item size for proper pointer storage */
    item_size = align_size(item_size);
    
    /* Item must be large enough to hold a free list pointer */
    if (item_size < sizeof(void*))
        item_size = sizeof(void*);
    
    /* Allocate pool structure */
    if (alloc != NULL)
        pool = (mem_pool_t*)yaep_malloc(alloc, sizeof(mem_pool_t));
    else
        pool = (mem_pool_t*)malloc(sizeof(mem_pool_t));
    
    if (pool == NULL)
        return NULL;
    
    /* Initialize pool */
    pool->alloc = alloc;
    pool->item_size = item_size;
    pool->items_per_block = items_per_block;
    pool->blocks = NULL;
    pool->current_block = NULL;
    pool->current_offset = 0;
    pool->free_list = NULL;
    pool->total_allocated = 0;
    pool->total_freed = 0;
    pool->blocks_allocated = 0;
    
    return pool;
}

/**
 * allocate_block - Allocate a new block of items
 *
 * @param pool Pool to allocate block for
 * @return New block on success, NULL on failure
 */
static struct mem_block *
allocate_block(mem_pool_t *pool)
{
    struct mem_block *block = NULL;
    size_t block_size;
    
    /* Calculate block size: header + items */
    block_size = sizeof(struct mem_block) + (pool->item_size * pool->items_per_block);
    
    /* Allocate block */
    if (pool->alloc != NULL)
        block = (struct mem_block*)yaep_malloc(pool->alloc, block_size);
    else
        block = (struct mem_block*)malloc(block_size);
    
    if (block == NULL)
        return NULL;
    
    /* Link block into block list */
    block->next = pool->blocks;
    pool->blocks = block;
    pool->blocks_allocated++;
    
    return block;
}

/**
 * pool_alloc - Allocate item from pool
 *
 * Implementation follows design in mem_pool.h header documentation.
 */
void *
pool_alloc(mem_pool_t *pool)
{
    void *item = NULL;
    
    if (pool == NULL)
        return NULL;
    
    /* Try free list first (fast path for reuse) */
    if (pool->free_list != NULL) {
        struct free_node *node = (struct free_node*)pool->free_list;
        pool->free_list = node->next;
        pool->total_allocated++;
        return (void*)node;
    }
    
    /* Check if current block has space */
    if (pool->current_block == NULL || pool->current_offset >= pool->items_per_block) {
        /* Allocate new block */
        pool->current_block = allocate_block(pool);
        if (pool->current_block == NULL)
            return NULL;  /* Allocation failed */
        pool->current_offset = 0;
    }
    
    /* Bump-pointer allocation from current block */
    item = (char*)(pool->current_block + 1) + (pool->current_offset * pool->item_size);
    pool->current_offset++;
    pool->total_allocated++;
    
    return item;
}

/**
 * pool_free - Return item to pool
 *
 * Implementation follows design in mem_pool.h header documentation.
 */
void
pool_free(mem_pool_t *pool, void *item)
{
    struct free_node *node = NULL;
    
    if (pool == NULL || item == NULL)
        return;
    
    /* Add item to free list (intrusive linking) */
    node = (struct free_node*)item;
    node->next = (struct free_node*)pool->free_list;
    pool->free_list = node;
    pool->total_freed++;
}

/**
 * pool_destroy - Destroy pool and free all blocks
 *
 * Implementation follows design in mem_pool.h header documentation.
 */
void
pool_destroy(mem_pool_t *pool)
{
    struct mem_block *block = NULL;
    struct mem_block *next = NULL;
    
    if (pool == NULL)
        return;
    
    /* Free all blocks */
    block = pool->blocks;
    while (block != NULL) {
        next = block->next;
        
        if (pool->alloc != NULL)
            yaep_free(pool->alloc, block);
        else
            free(block);
        
        block = next;
    }
    
    /* Free pool structure */
    if (pool->alloc != NULL)
        yaep_free(pool->alloc, pool);
    else
        free(pool);
}

/**
 * pool_stats - Get pool statistics
 *
 * Implementation follows design in mem_pool.h header documentation.
 */
void
pool_stats(mem_pool_t *pool,
           size_t *total_allocated_out,
           size_t *total_freed_out,
           size_t *blocks_allocated_out,
           size_t *free_list_length_out)
{
    size_t free_list_length = 0;
    struct free_node *node = NULL;
    
    if (pool == NULL)
        return;
    
    /* Count free list length if requested */
    if (free_list_length_out != NULL) {
        node = (struct free_node*)pool->free_list;
        while (node != NULL) {
            free_list_length++;
            node = node->next;
        }
    }
    
    /* Return requested statistics */
    if (total_allocated_out != NULL)
        *total_allocated_out = pool->total_allocated;
    if (total_freed_out != NULL)
        *total_freed_out = pool->total_freed;
    if (blocks_allocated_out != NULL)
        *blocks_allocated_out = pool->blocks_allocated;
    if (free_list_length_out != NULL)
        *free_list_length_out = free_list_length;
}
