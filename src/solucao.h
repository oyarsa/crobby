#ifndef SOLUCAO_H
#define SOLUCAO_H

#include "constantes.h"

/**
 * Define uma struct opaca. Representa uma Solução para o problema,
 * sendo composta por um vetor de movimentos (cromossomo) e o valor de sua
 * aptidão (fo). Construtor e métodos de acesso estão disponíveis.
 */
typedef struct Solucao Solucao;

/**
 * Cria uma nova solução a partir do cromossomo e de uma sementa para os números
 * aleatórios envolvidos no cálculo da FO. Considera-se que a Solução agora
 * possui a posse do cromossomo.
 * Retorna um objeto alocado dinamicamente, a ser liberado com Solucao_free
 */
Solucao* Solucao_nova(Movimento* cromossomo, unsigned long* seed);

/**
 * Retorna o valor da aptidão de uma solução não-nula.
 */
double Solucao_fo(Solucao* s);

/**
 * Retorna o cromossomo de uma solução não-nula. Não transfere posse.
 */
Movimento* Solucao_cromossomo(Solucao* s);

/**
 * Libera a memória do objeto da solução, assim como de seu cromossomo.
 */
void Solucao_free(Solucao* s);

/**
 * Função de comparação entre duas soluções não-nulas, em ordem decrescente,
 * podendo ser utilizada no qsort. Retorna -1 se fo(a) > fo(b), 1 se
 * fo(b) > fo(a) e 0 se forem iguais.
 */
int Solucao_cmp_desc(const void* a, const void* b);

#endif // SOLUCAO_H
