#ifndef Cronom_H
#define Cronom_H

/**
 * Define uma struct opaca. Representa o cronômetro.
 */
typedef struct Cronom Cronom;

/**
 * Cria um novo cronômetro, com tempo inicializado com o momento de sua criação.
 * O objeto é alocado dinamicamente e deve ser liberado com Cronom_free.
 */
Cronom* Cronom_novo(void);

/**
 * Retorna o tempo decorrido desde a criação do cronômetro, em millisegundos.
 */
double Cronom_tempo(Cronom* c);

/**
 * Libera a memória alocada para o objeto.
 */
void Cronom_free(Cronom* c);

#endif // Cronom_H
