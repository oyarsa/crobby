#ifndef SOLUCAO_H
#define SOLUCAO_H

typedef struct Solucao Solucao;

Solucao* Solucao_nova(int* cromossomo, unsigned* seed);
double Solucao_fo(Solucao* s);
int* Solucao_cromossomo(Solucao* s);
void Solucao_free(Solucao* s);

int Solucao_cmp_desc(const void* a, const void* b);

#endif // SOLUCAO_H
