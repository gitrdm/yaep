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

/* Print JSON string with escaping */
static void
print_json_string(FILE *fp, const char *str)
{
    if (!str) {
        fprintf(fp, "null");
        return;
    }
    
    fputc('"', fp);
    while (*str) {
        switch (*str) {
            case '"':  fprintf(fp, "\\\""); break;
            case '\\': fprintf(fp, "\\\\"); break;
            case '\b': fprintf(fp, "\\b"); break;
            case '\f': fprintf(fp, "\\f"); break;
            case '\n': fprintf(fp, "\\n"); break;
            case '\r': fprintf(fp, "\\r"); break;
            case '\t': fprintf(fp, "\\t"); break;
            default:
                if ((unsigned char)*str < 0x20) {
                    fprintf(fp, "\\u%04x", (unsigned char)*str);
                } else {
                    fputc(*str, fp);
                }
                break;
        }
        str++;
    }
    fputc('"', fp);
}

/* Print parse tree as JSON */
static void
print_tree_json(FILE *fp, struct yaep_tree_node *node, int depth)
{
    if (node == NULL) {
        fprintf(fp, "null");
        return;
    }
    
    /* Print indentation */
    const char *indent = "";
    const char *indent1 = "  ";
    const char *indent2 = "    ";
    if (depth > 0) {
        indent = "\n";
        for (int i = 0; i < depth; i++) {
            fprintf(fp, "  ");
        }
    }
    
    fprintf(fp, "{%s", indent);
    for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
    
    /* Print node type */
    fprintf(fp, "\"type\": ");
    switch (node->type) {
        case YAEP_NIL:
            fprintf(fp, "\"NIL\",\n");
            for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
            fprintf(fp, "\"value\": null");
            break;
            
        case YAEP_ERROR:
            fprintf(fp, "\"ERROR\",\n");
            for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
            fprintf(fp, "\"used\": %d", node->val.error.used);
            break;
            
        case YAEP_TERM:
            fprintf(fp, "\"TERM\",\n");
            for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
            fprintf(fp, "\"code\": %d,\n", node->val.term.code);
            for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
            fprintf(fp, "\"char\": ");
            if (node->val.term.code >= 32 && node->val.term.code < 127) {
                fprintf(fp, "\"%c\"", (char)node->val.term.code);
            } else {
                fprintf(fp, "\"\\\\x%02x\"", node->val.term.code);
            }
            break;
            
        case YAEP_ANODE:
            fprintf(fp, "\"ANODE\",\n");
            for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
            fprintf(fp, "\"name\": ");
            print_json_string(fp, node->val.anode.name);
            fprintf(fp, ",\n");
            for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
            fprintf(fp, "\"cost\": %d", node->val.anode.cost);
            
            /* Print children if present */
            if (node->val.anode.children && node->val.anode.children[0]) {
                fprintf(fp, ",\n");
                for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
                fprintf(fp, "\"children\": [");
                
                int first = 1;
                for (int i = 0; node->val.anode.children[i] != NULL; i++) {
                    if (!first) {
                        fprintf(fp, ",");
                    }
                    fprintf(fp, "\n");
                    for (int j = 0; j < depth + 2; j++) fprintf(fp, "  ");
                    print_tree_json(fp, node->val.anode.children[i], depth + 2);
                    first = 0;
                }
                fprintf(fp, "\n");
                for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
                fprintf(fp, "]");
            }
            break;
            
        case YAEP_ALT:
            fprintf(fp, "\"ALT\",\n");
            for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
            fprintf(fp, "\"alternatives\": [\n");
            
            /* Print all alternatives */
            struct yaep_tree_node *alt = node;
            int first_alt = 1;
            while (alt && alt->type == YAEP_ALT) {
                if (!first_alt) {
                    fprintf(fp, ",\n");
                }
                for (int i = 0; i < depth + 2; i++) fprintf(fp, "  ");
                print_tree_json(fp, alt->val.alt.node, depth + 2);
                alt = alt->val.alt.next;
                first_alt = 0;
            }
            fprintf(fp, "\n");
            for (int i = 0; i < depth + 1; i++) fprintf(fp, "  ");
            fprintf(fp, "]");
            break;
            
        default:
            fprintf(fp, "\"UNKNOWN\"");
            break;
    }
    
    fprintf(fp, "\n");
    for (int i = 0; i < depth; i++) fprintf(fp, "  ");
    fprintf(fp, "}");
}

/* Save tree to JSON file */
static int
save_tree_json(const char *filename, struct yaep_tree_node *root)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file '%s' for writing\n", filename);
        return -1;
    }
    
    fprintf(fp, "{\n  \"parse_tree\": ");
    print_tree_json(fp, root, 1);
    fprintf(fp, "\n}\n");
    
    fclose(fp);
    return 0;
}

int
main(int argc, char *argv[])
{
    const char *grammar_file = "meta-grammar.yaep";
    const char *input_file = NULL;
    const char *output_file = NULL;
    
    if (argc > 1) {
        input_file = argv[1];
    } else {
        fprintf(stderr, "Usage: %s <ebnf-file> [output.json]\n", argv[0]);
        fprintf(stderr, "Example: %s test_simple.ebnf test_simple.json\n", argv[0]);
        fprintf(stderr, "         %s ../meta-grammar.ebnf meta-grammar.json\n", argv[0]);
        return 1;
    }
    
    if (argc > 2) {
        output_file = argv[2];
    } else {
        /* Default output filename: replace .ebnf with .json */
        static char default_output[256];
        const char *base = input_file;
        const char *dot = strrchr(base, '.');
        const char *slash = strrchr(base, '/');
        
        if (slash) base = slash + 1;
        
        if (dot && dot > base) {
            int len = dot - base;
            if (len > 250) len = 250;
            snprintf(default_output, sizeof(default_output), "%.*s.json", len, base);
        } else {
            snprintf(default_output, sizeof(default_output), "%s.json", base);
        }
        output_file = default_output;
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
    
    /* Print parse tree to console */
    printf("\nParse tree structure:\n");
    print_tree_json(stdout, root, 0);
    printf("\n");
    
    /* Save parse tree to JSON file */
    printf("\nSaving parse tree to '%s'...\n", output_file);
    if (save_tree_json(output_file, root) == 0) {
        printf("JSON file saved successfully!\n");
    } else {
        fprintf(stderr, "Error: failed to save JSON file\n");
    }
    
    /* Cleanup */
    if (root) {
        yaep_free_tree(root, parse_free, NULL);
    }
    free(input_text);
    yaep_free_grammar(g);
    
    return 0;
}
