#include <stdio.h>
#include <stdlib.h>
#include "constantes.h"
#include "rand.h"

/**
 * Define o estado da simulação. Contém o cenário atual (o tabuleiro), as
 * coordenadas x e y do robô (pos_x e y), assim como a pontuação acumulada.
 */
typedef struct
{
  Conteudo tabuleiro[TAM_GRID][TAM_GRID];
  int pos_x;
  int pos_y;
  int pontuacao;
} Estado;

/**
 * Imprime o tabuleiro como uma matriz de inteiros.
 */
static inline void
print_tabuleiro(Conteudo tabuleiro[][TAM_GRID])
{
  for (int i = 0; i < TAM_GRID; i++) {
    for (int j = 0; j < TAM_GRID; j++) {
      printf("%d ", tabuleiro[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

/**
 * Inicializa o tabuleiro aleatoriamente.
 */
static inline void
init_tabuleiro(Conteudo tabuleiro[][TAM_GRID], unsigned long* seed)
{
  // Inicializa as bordas como paredes.
  for (int i = 0; i < TAM_GRID; i++) {
    tabuleiro[i][0] = PAREDE;
    tabuleiro[0][i] = PAREDE;
    tabuleiro[i][TAM_GRID - 1] = PAREDE;
    tabuleiro[TAM_GRID - 1][i] = PAREDE;
  }

  // Percorre as outras células, que terão 50% de chance de conter uma lata.
  for (int i = 1; i < TAM_GRID - 1; i++) {
    for (int j = 1; j < TAM_GRID - 1; j++) {
      double x = myrand(seed) / (double)MYRAND_MAX;
      tabuleiro[i][j] = x <= 0.5 ? LATA : VAZIO;
    }
  }
}

/**
 * Inicializa a simulação. O tabuleiro é gerado, o robô é colocado na origem
 * (que é 1,1 porque 0,0 é uma parede) e a pontuação é zerada.
 */
static inline void
init_estado(Estado* e, unsigned long* seed)
{
  init_tabuleiro(e->tabuleiro, seed);
  e->pos_x = e->pos_y = 1;
  e->pontuacao = 0;
}

/**
 * Obtém o índice linear do cenário a partir dos conteúdos das células
 * adjacentes ao robô. Esse índice será utilizado para consultar o movimento a
 * ser executado pelo robô, tal qual descrito no cromossomo.
 */
static inline int
get_cenario(Estado* e)
{
  // Cada uma das células adjacentes possui um conteúdo, representado por 0, 1
  // ou 2. Os 5 algarismos formam um número em base 3, que será convertido para
  // base 10 e retornado.
  Conteudo norte = e->tabuleiro[e->pos_y - 1][e->pos_x];
  Conteudo oeste = e->tabuleiro[e->pos_y][e->pos_x - 1];
  Conteudo atual = e->tabuleiro[e->pos_y][e->pos_x];
  Conteudo leste = e->tabuleiro[e->pos_y][e->pos_x + 1];
  Conteudo sul = e->tabuleiro[e->pos_y + 1][e->pos_x];

  return 81 * norte + 27 * sul + 9 * leste + 3 * oeste + atual;
}

/**
 * Funções de movimentação
 *
 * Recebem um estado e tentam se movimentar em uma direção. Se naquela direção
 * existir uma parede, a penalidade é aplicada. Senão, a coordenada é
 * modificada.
 */

static inline void
move_norte(Estado* e)
{
  if (e->tabuleiro[e->pos_y - 1][e->pos_x] == PAREDE) {
    e->pontuacao -= PENALIDADE_PAREDE;
  } else {
    e->pos_y--;
  }
}

static inline void
move_sul(Estado* e)
{
  if (e->tabuleiro[e->pos_y + 1][e->pos_x] == PAREDE) {
    e->pontuacao -= PENALIDADE_PAREDE;
  } else {
    e->pos_y++;
  }
}

static inline void
move_leste(Estado* e)
{
  if (e->tabuleiro[e->pos_y][e->pos_x + 1] == PAREDE) {
    e->pontuacao -= PENALIDADE_PAREDE;
  } else {
    e->pos_x++;
  }
}

static inline void
move_oeste(Estado* e)
{
  if (e->tabuleiro[e->pos_y][e->pos_x - 1] == PAREDE) {
    e->pontuacao -= PENALIDADE_PAREDE;
  } else {
    e->pos_x--;
  }
}

/**
 * Tenta pegar uma lata na célula atual do robô. Se não existir uma, sofre a
 * penalidade. Se existir, esvazia a célula e adicionada a pontuação.
 */
static inline void
pega_lata(Estado* e)
{
  if (e->tabuleiro[e->pos_y][e->pos_x] == LATA) {
    e->tabuleiro[e->pos_y][e->pos_x] = VAZIO;
    e->pontuacao += PONTUACAO_LATA;
  } else {
    e->pontuacao -= PENALIDADE_LATA;
  }
}

// Protótpos das funções, uma vez que elas chamam uma à outra.
static inline void proximo_estado(Estado* e, int movimento,
                                  unsigned long* seed);
static inline void move_aleatorio(Estado* e, unsigned long* seed);

/**
 * Obtém um movimento aleatório e o executa.
 */
void
move_aleatorio(Estado* e, unsigned long* seed)
{
  int movimento = myrand(seed) % TOTAL_MOVIMENTOS;
  proximo_estado(e, movimento, seed);
}

/**
 * Executa um Movimento, modificando o Estado.
 */
void
proximo_estado(Estado* e, int movimento, unsigned long* seed)
{
  switch (movimento) {
    case MOVE_NORTE:
      move_norte(e);
      break;
    case MOVE_SUL:
      move_sul(e);
      break;
    case MOVE_LESTE:
      move_leste(e);
      break;
    case MOVE_OESTE:
      move_oeste(e);
      break;
    case PEGAR_LATA:
      pega_lata(e);
      break;
    case FICAR_PARADO:
      // Faz nada
      break;
    case MOVE_ALEATORIO:
      move_aleatorio(e, seed);
      break;
  }
}

int
nova_simulacao(Movimento* cromossomo, unsigned long* seed)
{
  Estado e;
  init_estado(&e, seed);

  for (int i = 0; i < NUM_ACOES; i++) {
    // Obtém o cenário a partir do estado atual
    int cenario = get_cenario(&e);
    // Encontra o movimento correspondente ao cenário conforme dita o cromossomo
    int movimento = cromossomo[cenario];
    // Executa o Movimento e encontra o próximo estado
    proximo_estado(&e, movimento, seed);
  }

  return e.pontuacao;
}
