#include <time.h>
#include <assert.h>
#include "cronometro.h"
#include "mem.h"

struct Cronometro {
    clock_t comeco;
};

Cronometro* Cronometro_novo()
{
    Cronometro* c = myalloc(sizeof(Cronometro));
    c->comeco = clock();
    return c;
}

double Cronometro_tempo_decorrido(Cronometro* c)
{
    assert(c);
    clock_t agora = clock();
    return (double)(agora - c->comeco) / (double)CLOCKS_PER_SEC;
}

void Cronometro_free(Cronometro** c)
{
    myfree(c);
    c = NULL;
}