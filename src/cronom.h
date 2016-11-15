#ifndef Cronom_H
#define Cronom_H

typedef struct Cronom Cronom;

Cronom* Cronom_novo(void);
double Cronom_tempo(Cronom* c);
void Cronom_free(Cronom* c);

#endif // Cronom_H
