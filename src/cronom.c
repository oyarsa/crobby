#include <time.h>
#include "cronom.h"
#include "mem.h"

struct Cronom
{
  clock_t comeco;
};

Cronom*
Cronom_novo()
{
  Cronom* c = myalloc(sizeof(Cronom));
  c->comeco = clock();
  return c;
}

double
Cronom_tempo(Cronom* c)
{
  clock_t agora = clock();
  return (double)(agora - c->comeco) / (double)CLOCKS_PER_SEC;
}

void
Cronom_free(Cronom* c)
{
  free(c);
}