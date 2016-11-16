#ifndef AG_H
#define AG_H

#include "solucao.h"

/**
 * Operadores de mutação implementados.
 */
typedef enum { VIZINHANCA } Mutacao;

/**
 * Operadores de cruzamento implementados.
 */
typedef enum { UM_PONTO, MULTIPLOS_PONTOS, UNIFORME, SEGMENTADO } Cruzamento;

/**
 * Métodos de seleção implementados.
 */
typedef enum { ROLETA, TORNEIO } Selecao;

/**
 * Define uma struct opaca que representa a configuração de um AG.
 */
typedef struct Ag Ag;

/**
 * Executa o AG como configurado em 'ag', retornando a melhor solução
 * encontrada. Este objeto será alocado dinamicamente e deverá ser liberado com
 * Ag_free.
 */
Solucao* Ag_resolver(Ag* ag);

/**
 * Define uma struct transparente, que será utilizada para configurar os
 * parâmetros arbitrários do AG.
 */
typedef struct
{
  double taxa_mutacao;       // 0.005, 0.010
  double taxa_cruzamento;    // 0.95, 0.99
  int tam_populacao;         // 200, 400, 800
  int tam_torneio;           // 2, 4
  int num_pontos_cruz;       // 2, 4
  int max_iter_sem_melhoria; // 10, 20
  double taxa_perturbacao;   // 0.2, 0.5, 0.8
  Cruzamento oper_cruz;      // UNIFORME, UM_PONTO, MULTIPLOS_PONTOS, SEGMENTADO
  Selecao metodo_selec;      // TORNEIO, ROLETA
  double taxa_troca_seg;     // fixo em 0.2
  Mutacao oper_mut;          // fixo em VIZINHANCA
  int num_geracoes;          // ?
  double tempo_max;          // ?
} Agbuilder;

/**
 * Retorna um Agbuilder configurado com os valores padrão. Este objeto deverá
 * ser modificado de acordo com as configurações desejadas.
 */
Agbuilder Agbuilder_novo(void);

/**
 * Retorna um Ag configurado com os parâmetros do Agbuilder. Esse objeto será
 * alocado dinamicamente.
 */
Ag* Ag_create(Agbuilder* agb);

/**
 * Libera a memória do objeto Ag.
 */
void Ag_free(Ag* ag);

#endif // AG_H