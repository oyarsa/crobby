#ifndef CONSTANTES_H
#define CONSTANTES_H

#define TAM_CROM 243
#define TAM_GRID 12

typedef enum { VAZIO = 0, LATA = 1, PAREDE = 2 } Conteudo;

#define NUM_ACOES 200
#define NUM_SESSOES 200

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

#define PENALIDADE_PAREDE 5
#define PENALIDADE_LATA 1
#define PONTUACAO_LATA 10

#endif // CONSTANTES_H
