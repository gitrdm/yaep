/*
 * Example: Using the meta-grammar.yaep parser
 * 
 * This program demonstrates how to parse EBNF grammar files using the
 * YAEP meta-grammar parser.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yaep.h"

/* Global state for character reading */
static const char *input_ptr = NULL;
static const char *input_start = NULL;

/* Simple token reading function for character-level parsing */
static int
read_char_token(void **attr)
{
    /* Initialize on first call */
    if (input_start == NULL) {
        fprintf(stderr, "Error: input not set\n");
        return -1;
    }
    
    if (*input_ptr == '\0') {
        return -1; /* EOF */
    }
    
    /* Return the current character as token value */
    int ch = (unsigned char)*input_ptr;
    *attr = (void *)(long)ch;
    input_ptr++;
    return ch;
}

/* Set the input string for parsing */
static void
set_input(const char *input)
{
    input_ptr = input;
    input_start = input;
}

/* Syntax error handler */
static void
syntax_error(int err_tok_num, void *err_tok_attr,
             int start_ignored_tok_num, void *start_ignored_tok_attr,
             int start_recovered_tok_num, void *start_recovered_tok_attr)
{
    /* Silence unused-parameter warnings */
    (void) err_tok_attr;
    (void) start_ignored_tok_attr;
    (void) start_recovered_tok_attr;
    (void) start_ignored_tok_num;
    (void) start_recovered_tok_num;
    
    fprintf(stderr, "Syntax error at token %d\n", err_tok_num);
}

/* Memory allocation for parse tree */
static void *
parse_alloc(int size)
{
    return malloc(size);
}

/* Memory deallocation for parse tree */
static void
parse_free(void *mem)
{
    free(mem);
}

/* Read file contents into a string */
static char *
read_file(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return NULL;
    }
    
    /* Get file size */
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    /* Allocate and read */
    char *content = malloc(size + 1);
    if (!content) {
        fclose(fp);
        return NULL;
    }
    
    size_t read_size = fread(content, 1, size, fp);
    content[read_size] = '\0';
    fclose(fp);
    
    return content;
}

/* Print parse tree (simplified) */
static void
print_tree(struct yaep_tree_node *node, int depth)
{
    if (node == NULL) return;
    
    /* Print indentation */
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    /* Print node info */
    printf("Node: %p\n", (void*)node);
    
    /* Recursively print children if needed */
    /* (YAEP tree structure depends on translation annotations) */
}

int
main(int argc, char *argv[])
{
    const char *grammar_file = "meta-grammar.yaep";
    const char *input_file = NULL;
    
    if (argc > 1) {
        input_file = argv[1];
    } else {
        fprintf(stderr, "Usage: %s <ebnf-file>\n", argv[0]);
        fprintf(stderr, "Example: %s ../meta-grammar.ebnf\n", argv[0]);
        return 1;
    }
    
    /* Create grammar */
    struct grammar *g = yaep_create_grammar();
    if (!g) {
        fprintf(stderr, "Error: cannot create grammar\n");
        return 1;
    }
    
    /* Read grammar specification */
    char *grammar_text = read_file(grammar_file);
    if (!grammar_text) {
        yaep_free_grammar(g);
        return 1;
    }
    
    printf("Loading grammar from '%s'...\n", grammar_file);
    if (yaep_parse_grammar(g, 1, grammar_text) != 0) {
        fprintf(stderr, "Grammar error: %s\n", yaep_error_message(g));
        free(grammar_text);
        yaep_free_grammar(g);
        return 1;
    }
    free(grammar_text);
    printf("Grammar loaded successfully.\n");
    
    /* Read input EBNF file */
    char *input_text = read_file(input_file);
    if (!input_text) {
        yaep_free_grammar(g);
        return 1;
    }
    
    printf("Parsing '%s'...\n", input_file);
    
    /* Set up for parsing */
    set_input(input_text);
    
    struct yaep_tree_node *root = NULL;
    int ambiguous = 0;
    
    /* Parse the input */
    int result = yaep_parse(g, read_char_token, syntax_error,
                           parse_alloc, parse_free, &root, &ambiguous);
    
    if (result != 0) {
        fprintf(stderr, "Parse error: %s\n", yaep_error_message(g));
        free(input_text);
        yaep_free_grammar(g);
        return 1;
    }
    
    printf("Parse successful!\n");
    if (ambiguous) {
        printf("Warning: grammar is ambiguous\n");
    }
    
    /* Print parse tree */
    printf("\nParse tree:\n");
    print_tree(root, 0);
    
    /* Cleanup */
    if (root) {
        yaep_free_tree(root, parse_free, NULL);
    }
    free(input_text);
    yaep_free_grammar(g);
    
    return 0;
}
