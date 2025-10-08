/**
 * @file fuzz_yaep.c
 * @brief LibFuzzer harness for YAEP C17 modernization testing
 *
 * This fuzzer targets the grammar parsing and token processing paths
 * that previously exhibited crashes and memory leaks with setjmp/longjmp.
 *
 * Test Objectives:
 * 1. Verify no segfaults from NULL pointer dereferences (previous bug)
 * 2. Verify no memory leaks from longjmp bypassing cleanup
 * 3. Verify proper error handling with malformed input
 * 4. Stress test thread-local error context
 *
 * Build:
 *   clang -fsanitize=fuzzer,address -g -O1 fuzz_yaep.c -I../src \
 *         -L../build/src -lyaep -o fuzz_yaep
 *
 * Run:
 *   ASAN_OPTIONS=detect_leaks=1:allocator_may_return_null=1 \
 *   ./fuzz_yaep corpus/ -max_len=4096 -timeout=1
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* YAEP public API */
#include "yaep.h"

/* Global state for test callbacks (safe for single-threaded fuzzer) */
static void *g_tok_attr = NULL;
static int g_tok_count = 0;

/**
 * @brief Simple token reader for testing
 */
static int read_test_token(void **attr) {
    /* Return EOF after a few tokens to avoid timeout */
    if (g_tok_count++ > 10) {
        return -1;  /* EOF */
    }
    
    *attr = &g_tok_attr;
    /* Return a random token code (bounded) */
    return g_tok_count % 256;
}

/**
 * @brief No-op error handler for testing
 */
static void test_error(int err_tok_num, void *err_tok_attr,
                      int start_ignored, void *start_ignored_attr,
                      int start_recovered, void *start_recovered_attr) {
    /* No-op error handler */
    (void)err_tok_num;
    (void)err_tok_attr;
    (void)start_ignored;
    (void)start_ignored_attr;
    (void)start_recovered;
    (void)start_recovered_attr;
}

/**
 * @brief Fuzzer entry point
 *
 * Tests grammar parsing with arbitrary input, targeting previously
 * crash-prone code paths:
 * - Grammar description parsing (sgramm.y/yyparse)
 * - Token validation (tok_add with NULL symbols)
 * - Error recovery paths
 *
 * @param data Fuzz input data
 * @param size Length of fuzz input
 * @return 0 (always, crashes are detected by sanitizers)
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    struct grammar *g = NULL;
    int result;
    
    /* Ignore too-small inputs */
    if (size < 10) {
        return 0;
    }
    
    /* Limit input size to prevent timeout */
    if (size > 8192) {
        size = 8192;
    }
    
    /* Create null-terminated grammar string from fuzz input */
    char *grammar_desc = (char *)malloc(size + 1);
    if (!grammar_desc) {
        return 0;
    }
    memcpy(grammar_desc, data, size);
    grammar_desc[size] = '\0';
    
    /* Test 1: Grammar creation (previously crashed with allocation errors) */
    g = yaep_create_grammar();
    if (!g) {
        free(grammar_desc);
        return 0;
    }
    
    /* Test 2: Grammar parsing (previously crashed in yyparse/validation)
     * This is the main target - the yyerror and validation paths that
     * used to longjmp and cause memory leaks/crashes.
     */
    result = yaep_parse_grammar(g, 0, grammar_desc);
    
    /* If grammar parsing succeeded, try a simple parse to test token handling */
    if (result == 0) {
        struct yaep_tree_node *root = NULL;
        int ambiguous = 0;
        
        /* Reset token state */
        g_tok_count = 0;
        
        /* Test 3: Parsing (token handling path that had NULL deref bug) */
        result = yaep_parse(g, read_test_token, test_error,
                           NULL, NULL, &root, &ambiguous);
        
        /* Clean up parse tree if created */
        if (root) {
            yaep_free_tree(root, NULL, NULL);
        }
    }
    
    /* Test 4: Error message retrieval (thread-local error context) */
    const char *err_msg = yaep_error_message(g);
    (void)err_msg;  /* Use it to avoid optimization */
    
    /* Clean up - this tests proper cleanup paths */
    yaep_free_grammar(g);
    free(grammar_desc);
    
    /* If we got here without crashing, the C17 modernization worked! */
    return 0;
}

/* Optional: Custom initialization for fuzzer */
__attribute__((constructor))
static void fuzz_init(void) {
    /* Could initialize any global state if needed */
}
