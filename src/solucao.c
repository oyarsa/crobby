#include "solucao.h"
#include "simulacao.h"
#include "constantes.h"
#include "mem.h"

struct Solucao
{
  Movimento* cromossomo;
  double fo;
};

double
calcula_fo(Movimento* cromossomo, unsigned long* seed)
{
  double soma = 0.0;

  for (int i = 0; i < NUM_SESSOES; i++) {
    soma += nova_simulacao(cromossomo, seed);
  }

  return soma / NUM_SESSOES;
}

Solucao*
Solucao_nova(Movimento* cromossomo, unsigned long* seed)
{
  Solucao* s = myalloc(sizeof(Solucao));
  s->cromossomo = cromossomo;
  s->fo = calcula_fo(cromossomo, seed);
  return s;
}

double
Solucao_fo(Solucao* s)
{
  return s->fo;
}

Movimento*
Solucao_cromossomo(Solucao* s)
{
  return s->cromossomo;
}

void
Solucao_free(Solucao* s)
{
  free(s->cromossomo);
  free(s);
}

int
Solucao_cmp_desc(const void* a, const void* b)
{
  const Solucao** sa = (const Solucao**)a;
  const Solucao** sb = (const Solucao**)b;

  if ((*sa)->fo > (*sb)->fo)
    return -1;
  else if ((*sa)->fo < (*sb)->fo)
    return 1;
  return 0;
}