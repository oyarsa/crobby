#include <stdio.h>
#include "mem.h"

void* myalloc(size_t nbytes)
{
    void* ptr = malloc(nbytes);
    if (!ptr) {
        fprintf(stderr, "Erro ao alocar memória. Saindo.\n");
        exit(1);
    }
    return ptr;
}
