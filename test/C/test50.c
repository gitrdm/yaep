/*
   YAEP (Yet Another Earley Parser)

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

#include <stdio.h>
#include <stdlib.h>

#include "yaep.h"

static const char *description =
  "\n"
  "TERM;\n"
  "S : A\n"
  "  ;\n"
  "A : 'b'\n"
  "  ;\n";

static const char *input = "b";

static int
read_token (void **attr)
{
  static size_t pos = 0;

  *attr = NULL;
  if (input[pos] == '\0')
    return -1;

  return (int)input[pos++];
}

static void
syntax_error (int err_tok_num, void *err_tok_attr,
              int start_ignored_tok_num, void *start_ignored_tok_attr,
              int start_recovered_tok_num, void *start_recovered_tok_attr)
{
  (void)err_tok_num;
  (void)err_tok_attr;
  (void)start_ignored_tok_num;
  (void)start_ignored_tok_attr;
  (void)start_recovered_tok_num;
  (void)start_recovered_tok_attr;
}

static void *
parse_alloc (int size)
{
  return malloc ((size_t)size);
}

static void
parse_free (void *mem)
{
  free (mem);
}

int
main (void)
{
  struct grammar *grammar;
  struct yaep_tree_node *root = NULL;
  int ambiguous_p = 0;
  int rc;
  int n_leo_items = 0;
  int n_leo_completions = 0;

  grammar = yaep_create_grammar ();
  if (grammar == NULL)
    {
      fprintf (stderr, "Leo stats: FAIL (unable to allocate grammar)\n");
      return 1;
    }

  rc = yaep_parse_grammar (grammar, 1, description);
  if (rc != 0)
    {
      fprintf (stderr, "Leo stats: FAIL (grammar parse error: %s)\n",
               yaep_error_message (grammar));
      yaep_free_grammar (grammar);
      return 1;
    }

  rc = yaep_parse (grammar, read_token, syntax_error,
                   parse_alloc, parse_free, &root, &ambiguous_p);
  if (rc != 0)
    {
      fprintf (stderr, "Leo stats: FAIL (parse error: %s)\n",
               yaep_error_message (grammar));
      yaep_free_grammar (grammar);
      return 1;
    }

  if (yaep_get_leo_stats (grammar, &n_leo_items, &n_leo_completions) != 0)
    {
      fprintf (stderr, "Leo stats: FAIL (stats unavailable)\n");
      yaep_free_tree (root, parse_free, NULL);
      yaep_free_grammar (grammar);
      return 1;
    }

  if (n_leo_items <= 0 || n_leo_completions <= 0)
    {
      fprintf (stderr,
               "Leo stats: FAIL (items=%d, completions=%d)\n",
               n_leo_items, n_leo_completions);
      yaep_free_tree (root, parse_free, NULL);
      yaep_free_grammar (grammar);
      return 1;
    }

  printf ("Leo stats: PASS (items=%d, completions=%d)\n",
          n_leo_items, n_leo_completions);

  yaep_free_tree (root, parse_free, NULL);
  yaep_free_grammar (grammar);

  return 0;
}
