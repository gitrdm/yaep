#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "yaep.h"

/* This test exercises a simple right-recursive grammar which should
 * trigger the Leo single-waiter optimization in conservative mode.
 * Grammar:
 *   S : S 'a' | 'b'
 * Input: b a a
 */

static const char *input = "baa";

static const char *description =
"\n"
"TERM;\n"
"S : S 'a' # 0\n"
"  | 'b'   # 0\n"
"  ;\n";

int main(int argc, char **argv)
{
  struct grammar *g;
  struct yaep_tree_node *root;
  int ambiguous_p;

  if ((g = yaep_create_grammar()) == NULL)
    {
      fprintf(stderr, "yaep_create_grammar: No memory\n");
      exit(1);
    }

  /* Parse description */
  if (yaep_parse_grammar(g, 1, description) != 0)
    {
      fprintf(stderr, "%s\n", yaep_error_message(g));
      exit(1);
    }

  /* Enable Leo debug only when explicitly requested via argv[2] != 0.
     This avoids printing debug lines during normal test runs. */
  if (argc > 2 && atoi(argv[2]) != 0)
    yaep_set_leo_debug(g, 1);

  if (yaep_parse(g, test_read_token, test_syntax_error,
                 test_parse_alloc, test_parse_free, &root, &ambiguous_p))
    {
      fprintf(stderr, "yaep parse: %s\n", yaep_error_message(g));
      exit(1);
    }

  /* Ensure test emits a single newline so the CTest expected output (an
     empty line) matches. Do not print other debug information during
     standard runs. */
  printf("\n");
  yaep_free_grammar(g);
  return 0;
}
