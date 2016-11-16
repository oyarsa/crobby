#ifndef MYRAND_H
#define MYRAND_H

#include <limits.h>

#define MYRAND_MAX ULONG_MAX

/**
 * Obtém um número pseudoaleatório a partir da semente 'y'. Esta é uma função
 * pura, não dependendo de nenhum estado global, ao contrário de rand. Sendo
 * assim, é perfeitamente thread-safe.
 */
static inline unsigned long
myrand(unsigned long* y)
{
  *y ^= (*y << 13);
  *y ^= (*y >> 17);
  return (*y ^= (*y << 5));
}

#endif
