#ifndef AG_H
#define AG_H

#include "solucao.h"

typedef enum {
    VIZINHANCA
} Mutacao;

typedef enum {
    UM_PONTO,
    MULTIPLOS_PONTOS,
    UNIFORME,
    SEGMENTADO
} Cruzamento;

typedef enum {
    ROLETA,
    TORNEIO
} Selecao;

typedef struct Ag Ag;

Solucao* Ag_resolver(Ag* ag);

typedef struct {
    double taxa_mutacao;
    int tam_populacao;
    int num_geracoes;
    int tam_torneio;
    int num_pontos_cruz;
    double taxa_troca_seg;
    int max_iter_sem_melhoria;
    Mutacao oper_mut;
    Cruzamento oper_cruz;
    Selecao metodo_selec;
} AgBuilder;

AgBuilder AgBuilder_novo();
Ag* create_ag(AgBuilder* agb);

#endif // AG_H