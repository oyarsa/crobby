#ifndef CONSTANTES_H
#define CONSTANTES_H

/**
 * Número de cenário possíveis para o robô. Cada célula pode possuir 3 conteúdos
 * diferentes (vazio, lata e parede), e o robô conhece 5 células em um
 * determinado momento (norte, sul, leste, oeste e atual). Sendo assim, existem
 * 5^3 = 243 cenários diferentes, e um cromossomo representa qual ação o robô
 * tomará em cada um desses cenários.
 */
#define TAM_CROM 243

/**
 * Tamanho do tabuleiro. Ele é 10x10, somando às colunas e linhas extras para as
 * paredes temos 12x12.
 */
#define TAM_GRID 12

/**
 * Tipos de conteúdos que uma célula deve conter. O tabuleiro será uma matriz
 * 2D desses conteúdos.
 */
typedef enum { VAZIO = 0, LATA = 1, PAREDE = 2 } Conteudo;

/**
 * Número de ações que o robô pode fazer em uma simulação
 */
#define NUM_ACOES 200

/**
 * Número de simulações que serão executadas para avaliar um cromossomo.
 */
#define NUM_SESSOES 200

/**
 * Movimentos que o robô pode fazer. O cromossomo será um vetor desses
 * movimentos.
 */
#define TOTAL_MOVIMENTOS 7
typedef enum {
  MOVE_NORTE = 0,
  MOVE_SUL = 1,
  MOVE_LESTE = 2,
  MOVE_OESTE = 3,
  FICAR_PARADO = 4,
  PEGAR_LATA = 5,
  MOVE_ALEATORIO = 6
} Movimento;

/**
 * Penalidades e pontuações pelas ações do robô.
 * PENALIDADE_PAREDE: penalidade recebida ao bater em uma parede.
 * PENALIDADE_LATA: penalidade recebida ao tentar pegar uma lata em uma célula
 *                  vazia
 * PONTUACAO_LATA: pontuação recebida ao pegar uma lata em uma célula que contém
 *                 uma
 */
#define PENALIDADE_PAREDE 5
#define PENALIDADE_LATA 1
#define PONTUACAO_LATA 10

#endif // CONSTANTES_H
