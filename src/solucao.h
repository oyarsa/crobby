#ifndef SOLUCAO_H
#define SOLUCAO_H

#include "constantes.h"

typedef struct Solucao Solucao;

Solucao* Solucao_nova(Movimento* cromossomo, unsigned long* seed);
double Solucao_fo(Solucao* s);
Movimento* Solucao_cromossomo(Solucao* s);
void Solucao_free(Solucao* s);

int Solucao_cmp_desc(const void* a, const void* b);

#endif // SOLUCAO_H
