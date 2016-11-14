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
    double taxa_mutacao; // 0.005, 0.010
    double taxa_cruzamento; // 0.95, 0.99
    int tam_populacao; // 200, 400, 800
    int tam_torneio; // 2, 4
    int num_pontos_cruz; // 2, 4
    double taxa_troca_seg; // fixo em 0.2
    int max_iter_sem_melhoria; // 10, 20
    double taxa_perturbacao; // 0.2, 0.5, 0.8
    Cruzamento oper_cruz; // UNIFORME, UM_PONTO, MULTIPLOS_PONTOS, SEGMENTADO
    Selecao metodo_selec; // TORNEIO, ROLETA
    Mutacao oper_mut; // fixo em VIZINHANCA
    int num_geracoes;
    double tempo_max;
} Agbuilder;

Agbuilder Agbuilder_novo();
Ag* Ag_create(Agbuilder* agb);
void Ag_free(Ag* ag);

#endif // AG_H