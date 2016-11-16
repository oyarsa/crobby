#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include <stdio.h>

/**
 * Aloca memória usando malloc, mas checa o retorno e mata o programa se
 * não tiver memória.
 */
static inline void*
myalloc(size_t nbytes)
{
  void* ptr = malloc(nbytes);
  if (!ptr) {
    fprintf(stderr, "Erro ao alocar memória. Saindo.\n");
    exit(1);
  }
  return ptr;
}

#endif // MEM_H
