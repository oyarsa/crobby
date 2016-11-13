#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "constantes.h"
#include "rand.h"

typedef struct
{
    int tabuleiro[TAM_GRID][TAM_GRID];
    int pos_x;
    int pos_y;
    int pontuacao;
} Estado;

void proximo_estado(Estado* e, int movimento, unsigned long* seed);
void move_aleatorio(Estado* e, unsigned long* seed);

void print_tabuleiro(int tabuleiro[][TAM_GRID])
{
    for (int i = 0; i < TAM_GRID; i++) {
        for (int j = 0; j < TAM_GRID; j++) {
            printf("%d ", tabuleiro[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void init_tabuleiro(int tabuleiro[][TAM_GRID], unsigned long* seed)
{
    for (int i = 0; i < TAM_GRID; i++) {
        tabuleiro[i][0] = PAREDE;
        tabuleiro[0][i] = PAREDE;
        tabuleiro[i][TAM_GRID - 1] = PAREDE;
        tabuleiro[TAM_GRID - 1][i] = PAREDE;
    }

    for (int i = 1; i < TAM_GRID - 1; i++) {
        for (int j = 1; j < TAM_GRID - 1; j++) {
            double x = myrand(seed) / (double)MYRAND_MAX;
            tabuleiro[i][j] = x <= 0.5 ? LATA : VAZIO;
        }
    }
}

void init_estado(Estado* e, unsigned long* seed)
{
    init_tabuleiro(e->tabuleiro, seed);
    e->pos_x = e->pos_y = 1;
    e->pontuacao = 0;
}

int get_cenario(Estado* e)
{
    int norte = e->tabuleiro[e->pos_y - 1][e->pos_x];
    int oeste = e->tabuleiro[e->pos_y][e->pos_x - 1];
    int atual = e->tabuleiro[e->pos_y][e->pos_x];
    int leste = e->tabuleiro[e->pos_y][e->pos_x + 1];
    int sul = e->tabuleiro[e->pos_y + 1][e->pos_x];

    return 81 * norte + 27 * sul + 9 * leste + 3 * oeste + atual;
}

void move_norte(Estado* e)
{
    if (e->tabuleiro[e->pos_y - 1][e->pos_x] == PAREDE) {
        e->pontuacao -= PENALIDADE_PAREDE;
    } else {
        e->pos_y--;
    }
}

void move_sul(Estado* e)
{
    if (e->tabuleiro[e->pos_y + 1][e->pos_x] == PAREDE) {
        e->pontuacao -= PENALIDADE_PAREDE;
    } else {
        e->pos_y++;
    }
}

void move_leste(Estado* e)
{
    if (e->tabuleiro[e->pos_y][e->pos_x + 1] == PAREDE) {
        e->pontuacao -= PENALIDADE_PAREDE;
    } else {
        e->pos_x++;
    }
}

void move_oeste(Estado* e)
{
    if (e->tabuleiro[e->pos_y][e->pos_x - 1] == PAREDE) {
        e->pontuacao -= PENALIDADE_PAREDE;
    } else {
        e->pos_x--;
    }
}

void pega_lata(Estado* e)
{
    if (e->tabuleiro[e->pos_y][e->pos_x] == LATA) {
        e->tabuleiro[e->pos_y][e->pos_x] = VAZIO;
        e->pontuacao += PONTUACAO_LATA;
    } else {
        e->pontuacao -= PENALIDADE_LATA;
    }
}

void proximo_estado(Estado* e, int movimento, unsigned long* seed)
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
        /* Faz nada*/
        break;
    case MOVE_ALEATORIO:
        move_aleatorio(e, seed);
        break;
    }
}

void move_aleatorio(Estado* e, unsigned long* seed)
{
    int movimento = myrand(seed) % TOTAL_MOVIMENTOS;
    proximo_estado(e, movimento, seed);
}

int nova_simulacao(int* cromossomo, unsigned long* seed)
{
    Estado e;
    init_estado(&e, seed);

    for (int i = 0; i < NUM_ACOES; i++) {
        int cenario = get_cenario(&e);
        int movimento = cromossomo[cenario];
        proximo_estado(&e, movimento, seed);
    }

    return e.pontuacao;
}
