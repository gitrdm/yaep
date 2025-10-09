/**
 * @file test_mem_pool.c
 * @brief Unit tests for memory pool allocator
 *
 * Tests all mem_pool.h functions thoroughly to ensure correctness.
 *
 * @author YAEP Project
 * @date October 9, 2025
 */

#include "../src/mem_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  /* For uintptr_t */
#include <assert.h>

/* Test result tracking */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s...", #name); \
        fflush(stdout); \
        tests_run++; \
        if (name()) { \
            printf(" PASSED\n"); \
            tests_passed++; \
        } else { \
            printf(" FAILED\n"); \
        } \
    } while (0)

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "\nAssertion failed at %s:%d: %s\n", \
                    __FILE__, __LINE__, #cond); \
            return 0; \
        } \
    } while (0)

/**
 * Test: Create and destroy pool
 */
static int
test_pool_create_destroy(void)
{
    mem_pool_t *pool = NULL;
    
    /* Create pool */
    pool = pool_create(NULL, 64, 128);
    ASSERT(pool != NULL);
    
    /* Destroy pool */
    pool_destroy(pool);
    
    /* Destroy NULL pool (should be safe) */
    pool_destroy(NULL);
    
    return 1;
}

/**
 * Test: Invalid parameters to pool_create
 */
static int
test_pool_create_invalid(void)
{
    mem_pool_t *pool = NULL;
    
    /* Zero item size */
    pool = pool_create(NULL, 0, 128);
    ASSERT(pool == NULL);
    
    /* Zero items per block */
    pool = pool_create(NULL, 64, 0);
    ASSERT(pool == NULL);
    
    return 1;
}

/**
 * Test: Basic allocation
 */
static int
test_pool_alloc_basic(void)
{
    mem_pool_t *pool = NULL;
    void *item1 = NULL;
    void *item2 = NULL;
    void *item3 = NULL;
    
    pool = pool_create(NULL, 64, 128);
    ASSERT(pool != NULL);
    
    /* Allocate items */
    item1 = pool_alloc(pool);
    ASSERT(item1 != NULL);
    
    item2 = pool_alloc(pool);
    ASSERT(item2 != NULL);
    
    item3 = pool_alloc(pool);
    ASSERT(item3 != NULL);
    
    /* Items should be distinct */
    ASSERT(item1 != item2);
    ASSERT(item2 != item3);
    ASSERT(item1 != item3);
    
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Allocate many items (trigger block allocation)
 */
static int
test_pool_alloc_many(void)
{
    mem_pool_t *pool = NULL;
    void **items = NULL;
    size_t num_items = 1000;
    size_t i = 0;
    
    pool = pool_create(NULL, 64, 128);
    ASSERT(pool != NULL);
    
    /* Allocate array to track items */
    items = (void**)malloc(num_items * sizeof(void*));
    ASSERT(items != NULL);
    
    /* Allocate many items */
    for (i = 0; i < num_items; i++) {
        items[i] = pool_alloc(pool);
        ASSERT(items[i] != NULL);
    }
    
    /* Verify all items are distinct */
    for (i = 0; i < num_items; i++) {
        for (size_t j = i + 1; j < num_items; j++) {
            ASSERT(items[i] != items[j]);
        }
    }
    
    free(items);
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Free and reuse items
 */
static int
test_pool_free_reuse(void)
{
    mem_pool_t *pool = NULL;
    void *item1 = NULL;
    void *item2 = NULL;
    void *item3 = NULL;
    void *item4 = NULL;
    
    pool = pool_create(NULL, 64, 128);
    ASSERT(pool != NULL);
    
    /* Allocate and free */
    item1 = pool_alloc(pool);
    ASSERT(item1 != NULL);
    
    item2 = pool_alloc(pool);
    ASSERT(item2 != NULL);
    
    pool_free(pool, item1);
    
    /* Next allocation should reuse item1 */
    item3 = pool_alloc(pool);
    ASSERT(item3 != NULL);
    ASSERT(item3 == item1);  /* Reused! */
    
    /* Next allocation gets new item */
    item4 = pool_alloc(pool);
    ASSERT(item4 != NULL);
    ASSERT(item4 != item1);
    ASSERT(item4 != item2);
    ASSERT(item4 != item3);
    
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Free patterns (LIFO reuse)
 */
static int
test_pool_free_patterns(void)
{
    mem_pool_t *pool = NULL;
    void *items[10];
    int i = 0;
    
    pool = pool_create(NULL, 64, 128);
    ASSERT(pool != NULL);
    
    /* Allocate 10 items */
    for (i = 0; i < 10; i++) {
        items[i] = pool_alloc(pool);
        ASSERT(items[i] != NULL);
    }
    
    /* Free items 5-9 */
    for (i = 5; i < 10; i++) {
        pool_free(pool, items[i]);
    }
    
    /* Next allocations should reuse 9, 8, 7, 6, 5 (LIFO order) */
    for (i = 9; i >= 5; i--) {
        void *reused = pool_alloc(pool);
        ASSERT(reused == items[i]);
    }
    
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Free NULL item (should be safe)
 */
static int
test_pool_free_null(void)
{
    mem_pool_t *pool = NULL;
    
    pool = pool_create(NULL, 64, 128);
    ASSERT(pool != NULL);
    
    /* Free NULL should be safe */
    pool_free(pool, NULL);
    
    /* Free with NULL pool should be safe */
    pool_free(NULL, (void*)0x12345);
    
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Pool statistics
 */
static int
test_pool_stats(void)
{
    mem_pool_t *pool = NULL;
    void *item1 = NULL;
    void *item2 = NULL;
    void *item3 = NULL;
    size_t total_allocated = 0;
    size_t total_freed = 0;
    size_t blocks_allocated = 0;
    size_t free_list_length = 0;
    
    pool = pool_create(NULL, 64, 128);
    ASSERT(pool != NULL);
    
    /* Check initial stats */
    pool_stats(pool, &total_allocated, &total_freed, &blocks_allocated, &free_list_length);
    ASSERT(total_allocated == 0);
    ASSERT(total_freed == 0);
    ASSERT(blocks_allocated == 0);  /* No blocks yet (lazy allocation) */
    ASSERT(free_list_length == 0);
    
    /* Allocate items */
    item1 = pool_alloc(pool);
    ASSERT(item1 != NULL);
    item2 = pool_alloc(pool);
    ASSERT(item2 != NULL);
    item3 = pool_alloc(pool);
    ASSERT(item3 != NULL);
    
    pool_stats(pool, &total_allocated, &total_freed, &blocks_allocated, &free_list_length);
    ASSERT(total_allocated == 3);
    ASSERT(total_freed == 0);
    ASSERT(blocks_allocated == 1);  /* First block allocated */
    ASSERT(free_list_length == 0);
    
    /* Free one item */
    pool_free(pool, item2);
    
    pool_stats(pool, &total_allocated, &total_freed, &blocks_allocated, &free_list_length);
    ASSERT(total_allocated == 3);
    ASSERT(total_freed == 1);
    ASSERT(blocks_allocated == 1);
    ASSERT(free_list_length == 1);
    
    /* Reuse freed item */
    void *reused = pool_alloc(pool);
    ASSERT(reused == item2);
    
    pool_stats(pool, &total_allocated, &total_freed, &blocks_allocated, &free_list_length);
    ASSERT(total_allocated == 4);  /* Allocation count increments */
    ASSERT(total_freed == 1);
    ASSERT(blocks_allocated == 1);
    ASSERT(free_list_length == 0);  /* Free list now empty */
    
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Small item size (should be rounded up)
 */
static int
test_pool_small_items(void)
{
    mem_pool_t *pool = NULL;
    void *item = NULL;
    
    /* Item size smaller than pointer (should be rounded up) */
    pool = pool_create(NULL, 1, 128);
    ASSERT(pool != NULL);
    
    item = pool_alloc(pool);
    ASSERT(item != NULL);
    
    pool_free(pool, item);
    
    /* Should be able to reuse */
    void *reused = pool_alloc(pool);
    ASSERT(reused == item);
    
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Large number of blocks
 */
static int
test_pool_many_blocks(void)
{
    mem_pool_t *pool = NULL;
    size_t items_per_block = 10;
    size_t num_blocks = 100;
    size_t total_items = items_per_block * num_blocks;
    size_t i = 0;
    void **items = NULL;
    size_t blocks_allocated = 0;
    
    pool = pool_create(NULL, 64, items_per_block);
    ASSERT(pool != NULL);
    
    items = (void**)malloc(total_items * sizeof(void*));
    ASSERT(items != NULL);
    
    /* Allocate enough to trigger many blocks */
    for (i = 0; i < total_items; i++) {
        items[i] = pool_alloc(pool);
        ASSERT(items[i] != NULL);
    }
    
    /* Verify block count */
    pool_stats(pool, NULL, NULL, &blocks_allocated, NULL);
    ASSERT(blocks_allocated == num_blocks);
    
    free(items);
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Stress test (allocate/free patterns)
 */
static int
test_pool_stress(void)
{
    mem_pool_t *pool = NULL;
    void *items[1000];
    int i = 0;
    
    pool = pool_create(NULL, 128, 256);
    ASSERT(pool != NULL);
    
    /* Pattern: Allocate 1000, free odd, reallocate */
    for (i = 0; i < 1000; i++) {
        items[i] = pool_alloc(pool);
        ASSERT(items[i] != NULL);
    }
    
    for (i = 1; i < 1000; i += 2) {
        pool_free(pool, items[i]);
    }
    
    for (i = 0; i < 500; i++) {
        void *item = pool_alloc(pool);
        ASSERT(item != NULL);
    }
    
    pool_destroy(pool);
    return 1;
}

/**
 * Test: Aligned allocation
 */
static int
test_pool_alignment(void)
{
    mem_pool_t *pool = NULL;
    void *item = NULL;
    uintptr_t addr = 0;
    
    pool = pool_create(NULL, 64, 128);
    ASSERT(pool != NULL);
    
    /* Allocate item and check alignment */
    item = pool_alloc(pool);
    ASSERT(item != NULL);
    
    addr = (uintptr_t)item;
    ASSERT(addr % sizeof(void*) == 0);  /* Must be pointer-aligned */
    
    pool_destroy(pool);
    return 1;
}

/**
 * Main test runner
 */
int
main(void)
{
    printf("=== Memory Pool Unit Tests ===\n\n");
    
    TEST(test_pool_create_destroy);
    TEST(test_pool_create_invalid);
    TEST(test_pool_alloc_basic);
    TEST(test_pool_alloc_many);
    TEST(test_pool_free_reuse);
    TEST(test_pool_free_patterns);
    TEST(test_pool_free_null);
    TEST(test_pool_stats);
    TEST(test_pool_small_items);
    TEST(test_pool_many_blocks);
    TEST(test_pool_stress);
    TEST(test_pool_alignment);
    
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("\n✅ ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("\n❌ SOME TESTS FAILED\n");
        return 1;
    }
}
