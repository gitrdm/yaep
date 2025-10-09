/*
 * YAEP Benchmark Harness (Phase P0 Baseline)
 * -----------------------------------------
 * A minimal, stable micro-benchmark used only for capturing a performance
 * baseline BEFORE any refactors or optimizations (P0 gate requirement).
 *
 * Design Constraints (P0):
 *  - MUST NOT mutate parser implementation sources.
 *  - MUST use only stable public API in yaep.h.
 *  - MUST emit deterministic JSON (single line) so CI can diff future runs.
 *  - MUST be fast (< 1s default) but allow --iterations for smoothing.
 *  - NO premature complexity: single tiny arithmetic grammar & token stream.
 *
 * Future Phases (not implemented here):
 *  - Add multiple grammars (ambiguous, large, real-world subset)
 *  - Corpus-driven file input, more metrics (malloc counts, SPPF stats, etc.)
 *  - Extended JSON schema versioning and compatibility checks
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

#include "yaep.h"

/* Simple expression grammar (numbers + plus) to exercise parse loop.
 * Grammar (BNF-ish):
 *   Expr -> Expr PLUS Num | Num
 *   Num  -> NUM
 * Implemented via yaep_read_grammar callbacks for deterministic control. */

struct term_def { const char *name; int code; };
static const struct term_def term_table[] = {
    { "PLUS", 1 },
    { "NUM",  2 },
    { NULL, 0 }
};

struct rule_def { const char *lhs; const char **rhs; };
static const char *rhs0[] = { "Expr", "PLUS", "Num", NULL };
static const char *rhs1[] = { "Num", NULL };
static const char *rhs2[] = { "NUM", NULL };
static const struct rule_def rule_table[] = {
    { "Expr", rhs0 },
    { "Expr", rhs1 },
    { "Num",  rhs2 },
    { NULL, NULL }
};

static int term_index = 0;
static int rule_index = 0;

static const char *bench_read_terminal(int *code) {
    const struct term_def *t = &term_table[term_index];
    if (!t->name) return NULL;
    *code = t->code;
    term_index++;
    return t->name;
}

static const char *bench_read_rule(const char ***rhs, const char **abs_node, int *anode_cost, int **transl) {
    (void)anode_cost; /* no abstract node costs */
    const struct rule_def *r = &rule_table[rule_index];
    if (!r->lhs) return NULL;
    *rhs = r->rhs;
    *abs_node = NULL; /* no abstract node */
    *transl = NULL;   /* nil translation */
    rule_index++;
    return r->lhs;
}

static int build_expr_grammar(struct grammar *g) {
    term_index = 0; rule_index = 0; /* reset iterators */
    return yaep_read_grammar(g, 1, bench_read_terminal, bench_read_rule);
}

struct token_stream {
    int remaining; /* how many tokens (NUM PLUS NUM PLUS ...) left */
    int toggle;    /* 0 => NUM, 1 => PLUS (except last) */
};

static struct token_stream *g_ts = NULL; /* current active stream */

static int bench_read_token(void **attr) {
    (void)attr;
    if (!g_ts || g_ts->remaining <= 0) return -1;
    int code;
    if (g_ts->toggle == 0) { code = 2; g_ts->toggle = 1; }
    else {
        if (g_ts->remaining == 1) code = 2; else { code = 1; g_ts->toggle = 0; }
    }
    g_ts->remaining--;
    return code;
}

static void syntax_error(int err_tok_num, void *err_tok_attr,
                         int start_ignored_tok_num, void *start_ignored_tok_attr,
                         int start_recovered_tok_num, void *start_recovered_tok_attr) {
    (void)err_tok_num; (void)err_tok_attr; (void)start_ignored_tok_num; (void)start_ignored_tok_attr;
    (void)start_recovered_tok_num; (void)start_recovered_tok_attr;
    /* Suppress output in baseline to avoid noise */
}

/* We rely on YAEP internal allocation for parse tree (NULL alloc/free). */

/* Minimal monotonic timer (nanoseconds) */
static uint64_t nsec_now(void) {
#if defined(CLOCK_MONOTONIC)
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
#else
    return (uint64_t)clock() * (1000000000ull / (uint64_t)CLOCKS_PER_SEC);
#endif
}

static void usage(const char *argv0) {
    fprintf(stderr, "Usage: %s [--iterations N] [--tokens N] [--json path] [--baseline]\n", argv0);
}

int main(int argc, char **argv) {
    int iterations = 5;
    int n_tokens = 2000; /* generates ~1000 NUM + ~1000 PLUS pattern */
    const char *json_path = NULL;
    int baseline_mode = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--iterations") == 0 && i+1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--tokens") == 0 && i+1 < argc) {
            n_tokens = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--json") == 0 && i+1 < argc) {
            json_path = argv[++i];
        } else if (strcmp(argv[i], "--baseline") == 0) {
            baseline_mode = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if (iterations <= 0) iterations = 1;
    if (n_tokens < 3) n_tokens = 3; /* minimal pattern */

    struct grammar *g = yaep_create_grammar();
    if (!g) { fprintf(stderr, "Failed to create grammar\n"); return 2; }
    if (build_expr_grammar(g) != 0) {
        fprintf(stderr, "Grammar build error: %s\n", yaep_error_message(g));
        yaep_free_grammar(g); return 3;
    }

    /* Ensure deterministic flags */
    yaep_set_one_parse_flag(g, 1);
    yaep_set_cost_flag(g, 0);
    yaep_set_error_recovery_flag(g, 0);

    uint64_t best = UINT64_MAX, total = 0;
    int parses_ok = 0;

    for (int it = 0; it < iterations; ++it) {
        struct token_stream ts = { .remaining = n_tokens, .toggle = 0 };
        g_ts = &ts;

        struct yaep_tree_node *root = NULL; int ambiguous = 0;
        uint64_t start = nsec_now();
    int rc = yaep_parse(g, bench_read_token, syntax_error, NULL, NULL, &root, &ambiguous);
        uint64_t end = nsec_now();
        uint64_t dur = end - start;
        if (rc != 0) {
            fprintf(stderr, "parse error rc=%d err=%s\n", rc, yaep_error_message(g));
            yaep_free_tree(root, NULL, NULL);
            yaep_free_grammar(g);
            return 4;
        }
        if (ambiguous) {
            /* Should not be ambiguous for this grammar */
            fprintf(stderr, "unexpected ambiguity\n");
            yaep_free_tree(root, NULL, NULL);
            yaep_free_grammar(g);
            return 5;
        }
        yaep_free_tree(root, NULL, NULL);
        if (dur < best) best = dur;
        total += dur;
        parses_ok++;
    }

    double avg_ns = (double)total / (double)parses_ok;
    double best_ns = (double)best;
    /* (Optional future metrics: tokens/sec, etc.) */

    /* JSON output */
    FILE *out = stdout;
    if (json_path) {
        out = fopen(json_path, "w");
        if (!out) {
            fprintf(stderr, "Failed to open JSON path %s: %s\n", json_path, strerror(errno));
            yaep_free_grammar(g);
            return 6;
        }
    }
    fprintf(out,
            "{\"schema_version\":1,\"mode\":\"%s\",\"iterations\":%d,\"tokens\":%d,\"best_ns\":%.0f,\"avg_ns\":%.0f,\"avg_ns_per_token\":%.2f}\n",
            baseline_mode ? "baseline" : "ad-hoc",
            parses_ok, n_tokens, best_ns, avg_ns, avg_ns / (double)n_tokens);
    if (out != stdout) fclose(out);

    yaep_free_grammar(g);
    return 0;
}
