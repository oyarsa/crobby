#ifndef CRONOMETRO_H
#define CRONOMETRO_H

typedef struct Cronometro Cronometro;

Cronometro* Cronometro_novo();
double Cronometro_tempo_decorrido(Cronometro* c);
void Cronometro_free(Cronometro* c);

#endif // CRONOMETRO_H
