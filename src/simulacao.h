#ifndef SIMULACAO_H
#define SIMULACAO_H

/**
 * Executa uma nova simulação, utilizando um tabuleiro gerado aleatoriamente
 * a partir de 'seed', avaliando quantos pontos 'cromossomo' consegue
 * em NUM_ACOES movimentos.
 * Retorna a pontuação obtida.
 */
int nova_simulacao(Movimento* cromossomo, unsigned long* seed);

#endif // SIMULACAO_H
