#include <stdio.h>
#include <time.h>
#include "constantes.h"
#include "solucao.h"
#include "mem.h"
#include "cronometro.h"
#include "ag.h"

void teste_avaliacao()
{
    char cstr[] = "360353003053624022343046544151664253264654254406132601244625324426641432620215262066064443254034144531341433655664056335163054534043220266636353562346042663630315216456253252656455641566605115431214154445214431613043365440130042415066610554501";
    int* genes = myalloc(TAM_CROM * sizeof(int));

    for (int i = 0; i < TAM_CROM; i++) {
        genes[i] = cstr[i] - '0';
    }

    unsigned long seed = time(NULL);

    Cronometro* c = Cronometro_novo();
    Solucao* s = Solucao_nova(genes, &seed);
    printf("%g, %gs\n", Solucao_fo(s), Cronometro_tempo_decorrido(c));

    Cronometro_free(&c);
    Solucao_free(&s);
}

void teste_ag()
{
    AgBuilder agb = AgBuilder_novo();
    Ag* ag = create_ag(&agb);
    Solucao* s = Ag_resolver(ag);
    printf("%g\n", Solucao_fo(s));
}

int main()
{
    //teste_avaliacao();
    teste_ag();
}
