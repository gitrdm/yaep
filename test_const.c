#include <string.h>

struct sterm
{
  const char *repr;
  int code;
};

static int
sterm_name_cmp (const void *t1, const void *t2)
{
  return strcmp (((const struct sterm *) t1)->repr, ((const struct sterm *) t2)->repr);
}

int main() {
  return 0;
}
