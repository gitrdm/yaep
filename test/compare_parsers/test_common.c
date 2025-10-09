/*
   YAEP (Yet Another Earley Parser)

   Copyright (c) 1997-2018  Vladimir Makarov <vmakarov@gcc.gnu.org>

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

#define YAEP_TEST_ANSIC_H_ /* FIXME: avoid inclusion of ansic.h via ansic.c */

struct lex {
  short code;
  short column;
  int line;
  const char *id;
  struct lex *next;
};

static os_t lexs;
static struct lex *list;
static struct lex *curr = NULL;

static int column = 0;
static int line = 1;

static hash_table_t table;

/* Keep the hash/eq functions const-correct: hash_table_entry_t is used
   as an opaque pointer; treat it as a pointer to const char to avoid
   cast-discard-const warnings. */
static unsigned
hash (hash_table_entry_t el)
{
  const char *id = (const char *) el;
  unsigned result, i;

  for (result = i = 0; *id != '\0'; ++id, ++i)
    result += ((unsigned char) *id << (i % CHAR_BIT));
  return result;
}

static int
eq (hash_table_entry_t el1, hash_table_entry_t el2)
{
  /* Compare through const char pointers to preserve const-correctness
     and avoid casting away qualifiers. */
  return strcmp ((const char *) el1, (const char *) el2) == 0;
}

static void initiate_typedefs( YaepAllocator * alloc ) {
  table = create_hash_table( alloc, 50000, hash, eq );
}

/* Now we ignore level */
static
#if defined(__GNUC__)
__attribute__((unused))
#endif
void add_typedef (const char *id, int scope_level)
{
  hash_table_entry_t *entry_ptr;

  (void) scope_level; /* Unused - included for API compatibility */
  
  /* Use const-aware probe helper to avoid discarding qualifiers at call sites. */
  entry_ptr = find_hash_table_entry_c (table, id, 1);
  if (*entry_ptr == NULL)
    {
      /* Use union to avoid cast-qual warning when storing const char* */
      union { const char *cc; hash_table_entry_t v; } u;
      u.cc = id;
      *entry_ptr = u.v;
    }
  else
    assert (strcmp (id, *entry_ptr) == 0);
#ifdef DEBUG
  fprintf (stderr, "add typedef %s\n", id);
#endif
}

#ifdef __GNUC__
inline
#endif
static
int find_typedef (const char *id, int scope_level)
{
  hash_table_entry_t *entry_ptr;

  (void) scope_level; /* Unused - included for API compatibility */
  
  entry_ptr = find_hash_table_entry_c (table, id, 0);
#ifdef DEBUG
  if (*entry_ptr != NULL)
    fprintf (stderr, "found typedef %s\n", id);
#endif
  return *entry_ptr != NULL;
}
