#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include "ag.h"
#include "constantes.h"
#include "mem.h"
#include "rand.h"

struct Ag {
    double taxa_mutacao;
    int tam_populacao;
    int num_geracoes;
    int num_cruzamentos;
    int tam_torneio;
    int num_pontos_cruz;
    double taxa_troca_seg;
    int max_iter_sem_melhoria;
    Mutacao oper_mut;
    Cruzamento oper_cruz;
    Selecao metodo_selec;
    unsigned long rng_seed;
};

Solucao** gerar_individuos_aleatorios(Ag* ag, int num_individuos); //
void selecionar(Ag* ag, Solucao** populacao, Solucao*** pais); //
void cruzamento(Ag* ag, Solucao*** pais, int** filhos); //
void mutacao(Ag* ag, int** cromossomos); //
void avaliar(int** cromossomos, int n, Solucao** solucoes); //
void proxima_geracao(Ag* ag, Solucao** populacao, Solucao** prole); //
void perturbar_populacao(Ag* ag, Solucao** populacao); //
void selecao_roleta(Ag* ag, Solucao** populacao, Solucao*** pais); //
void selecao_torneio(Ag* ag, Solucao** populacao, Solucao*** pais); //
void executar_cruzamento(Ag* ag, Solucao* pai1, Solucao* pai2, int** filho1, int** filho2); //
void cruz_multiplos_pontos(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2); //
void cruz_segmentado(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2); //
void cruz_uniforme(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2); //
void cruz_um_ponto(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2); //
void cruzar_from_pontos(int* pontos_cruz, int npontos, int* pai1, int* pai2, //
    int** filho1, int** filho2);
int* gerar_mascara(Ag* ag); //
void vizinhanca(Ag* ag, int** cromossomos); //
int* gerar_solucao_aleatoria(Ag* ag); //
Solucao* obter_individuo_torneio(Ag* ag, Solucao** populacao); //
double* criar_roleta(Ag* ag, Solucao** populacao); //
double* normalizar_populacao(Ag* ag, Solucao** populacao); //
int obter_indice_roleta(Ag* ag, double* roleta); //

Solucao* resolver(Ag* ag)
{
    assert(ag);

    Solucao** populacao = gerar_individuos_aleatorios(ag, ag->tam_populacao);
    int iter_sem_melhoria = 0;
    double melhor_fo = Solucao_fo(populacao[0]);

    Solucao*** pais = myalloc(2 * ag->num_cruzamentos * sizeof(Solucao*));
    int** filhos = myalloc(2 * ag->num_cruzamentos * sizeof(int*));
    Solucao** avaliados = myalloc(2 * ag->num_cruzamentos * sizeof(Solucao*));

    for (int i = 0; i < ag->num_geracoes; i++) {
        selecionar(ag, populacao, pais);
        cruzamento(ag, pais, filhos);
        mutacao(ag, filhos);
        avaliar(filhos, 2 * ag->num_cruzamentos, avaliados);
        proxima_geracao(ag, populacao, avaliados);

        if (Solucao_fo(populacao[0]) > melhor_fo) {
            melhor_fo = Solucao_fo(populacao[0]);
            iter_sem_melhoria = 0;
        } else {
            iter_sem_melhoria++;
        }

        if (iter_sem_melhoria == ag->max_iter_sem_melhoria) {
            perturbar_populacao(ag, populacao);
            iter_sem_melhoria = 0;
        }
    }

    myfree(&pais);
    myfree(&filhos);
    myfree(&avaliados);

    for (int i = 1; i < ag->tam_populacao; i++) {
        myfree(&populacao[i]);
    }

    return populacao[0];
}

void selecionar(Ag* ag, Solucao** populacao, Solucao*** pais)
{
    switch (ag->metodo_selec) {
    case ROLETA:
        selecao_roleta(ag, populacao, pais);
        break;
    case TORNEIO:
    default:
        selecao_torneio(ag, populacao, pais);
        break;
    }
}

void cruzamento(Ag* ag, Solucao*** pais, int** filhos)
{
    for (int i = 0; i < ag->num_cruzamentos; i++) {
        executar_cruzamento(ag, pais[i][0], pais[i][1],
            &filhos[2 * i], &filhos[2 * i + 1]);
    }
}

void executar_cruzamento(Ag* ag, Solucao* pai1, Solucao* pai2, int** filho1, int** filho2)
{
    switch (ag->oper_cruz) {
    case MULTIPLOS_PONTOS:
        cruz_multiplos_pontos(ag, Solucao_cromossomo(pai1), Solucao_cromossomo(pai2),
            filho1, filho2);
        break;
    case SEGMENTADO:
        cruz_segmentado(ag, Solucao_cromossomo(pai1), Solucao_cromossomo(pai2),
            filho1, filho2);
        break;
    case UNIFORME:
        cruz_uniforme(ag, Solucao_cromossomo(pai1), Solucao_cromossomo(pai2),
            filho1, filho2);
        break;
    case UM_PONTO:
    default:
        cruz_um_ponto(ag, Solucao_cromossomo(pai1), Solucao_cromossomo(pai2),
            filho1, filho2);
        break;
    }
}

int compar_int(const void* a, const void* b)
{
    const int* ia = a;
    const int* ib = b;
    return *ia - *ib;
}

void cruz_multiplos_pontos(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2)
{
    int* pontos_cruz = myalloc((ag->num_pontos_cruz + 1) * sizeof(int));
    for (int i = 0; i < ag->num_pontos_cruz; i++) {
        pontos_cruz[i] = xorshift(&ag->rng_seed) % TAM_CROM;
    }
    pontos_cruz[ag->num_pontos_cruz] = TAM_CROM;
    qsort(pontos_cruz, ag->num_pontos_cruz + 1, sizeof(int), compar_int);
    cruzar_from_pontos(pontos_cruz, ag->num_pontos_cruz, pai1, pai2, filho1, filho2);
}

void cruz_segmentado(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2)
{
    int* pontos_cruz = myalloc(TAM_CROM * sizeof(int));
    int n = 0;
    for (int i = 0; i < TAM_CROM; i++) {
        if (xorshiftf(&ag->rng_seed) <= ag->taxa_troca_seg) {
            pontos_cruz[n] = i;
            n++;
        }
    }
    pontos_cruz[n] = TAM_CROM;
    n++;

    cruzar_from_pontos(pontos_cruz, n, pai1, pai2, filho1, filho2);
}

int* gerar_mascara(Ag* ag)
{
    int* mask = myalloc(TAM_CROM * sizeof(int));
    for (int i = 0; i < TAM_CROM; i++) {
        mask[i] = xorshiftf(&ag->rng_seed) <= 0.5 ? 1 : 0;
    }
    return mask;
}

void cruz_uniforme(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2)
{
    int* mask = gerar_mascara(ag);
    *filho1 = myalloc(TAM_CROM * sizeof(int));
    *filho2 = myalloc(TAM_CROM * sizeof(int));

    for (int i = 0; i < TAM_CROM; i++) {
        if (mask[i] == 0) {
            *filho1[i] = pai1[i];
            *filho2[i] = pai2[i];
        } else {
            *filho1[i] = pai2[i];
            *filho2[i] = pai1[i];
        }
    }
    myfree(mask);
}

void cruz_um_ponto(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2)
{
    *filho1 = myalloc(TAM_CROM * sizeof(int));
    *filho2 = myalloc(TAM_CROM * sizeof(int));
    int ponto = xorshift(&ag->rng_seed) % TAM_CROM;

    memcpy(*filho1, pai1, ponto);
    memcpy(*filho2, pai2, ponto);

    memcpy(*filho1 + ponto, pai2 + ponto, TAM_CROM - ponto);
    memcpy(*filho2 + ponto, pai1 + ponto, TAM_CROM - ponto);
}

void cruzar_from_pontos(int* pontos_cruz, int npontos, int* pai1, int* pai2,
    int** filho1, int** filho2)
{
    *filho1 = myalloc(TAM_CROM * sizeof(int));
    *filho2 = myalloc(TAM_CROM * sizeof(int));

    bool flip = false;
    int j = 0;

    for (int i = 0; i < npontos; i++) {
        for (; j < pontos_cruz[i]; j++) {
            if (flip) {
                *filho1[j] = pai1[j];
                *filho2[j] = pai2[j];
            } else {
                *filho1[j] = pai2[j];
                *filho2[j] = pai1[j];
            }
        }
        flip = !flip;
    }

    myfree(pontos_cruz);
}

void mutacao(Ag* ag, int** cromossomos)
{
    switch (ag->oper_mut) {
    case VIZINHANCA:
        vizinhanca(ag, cromossomos);
    }
}

void vizinhanca(Ag* ag, int** cromossomos)
{
    for (int i = 0; ag->tam_populacao; i++) {
        for (int j = 0; j < TAM_CROM; j++) {
            if (xorshiftf(&ag->rng_seed) <= ag->taxa_mutacao) {
                cromossomos[i][j] = xorshift(&ag->rng_seed) % TOTAL_MOVIMENTOS;
            }
        }
    }
}

Solucao** gerar_individuos_aleatorios(Ag* ag, int num_individuos)
{
    int** pop = myalloc(num_individuos * sizeof(int*));
    for (int i = 0; i < ag->tam_populacao; i++) {
        pop[i] = gerar_solucao_aleatoria(ag);
    }
    Solucao** avaliados = myalloc(num_individuos * sizeof(Solucao*));
    avaliar(pop, num_individuos, avaliados);
    return avaliados;
}

vooar(ium** _croross
          u,
      Solucaa** stlucoe;)
    o nte um uz* _;
r iuu_pontosa_ uz*;
t;)
    tm_p_nt  _cruz;
int num_ o _cruz;
{
    unsigned long seed = time(NULL);
    for (int i = 0; i < n; i++) {
        solucoes[i] = Slucao_nova(cr * mosomos[i], &seed);
        *cromossomos[i] = NULL;
    }
 s omos, n, sizeof(Solucao*), Solucao_cmp_desc);
}
id proxima_geracao(Ag* ag, Solucao** populacao, Solucao** prole)
{
    h os = 2 * ag->num_cruzamentos;
    int begin = ag->tam_populacao - num_filhos;

    r(int i = begin; i < ag->tam_populacao; i++)
    {
        myfree(&populacao[i]);
        populacao[i] = prole[i - eg * in];
        prole[i - begin] = NULL;
    }

    id perturbar_populacao(Ag * ag, Solucao * *populacao) int n = 0.8 * ag->tam_pop la * cao;
    cao** perturbacoes = gerar_individuos_aleatorios(ag, n);
    int begin = ag->tam_populacao - n;

    for (int ibegin; i < ag->tam_populacao; i++) {
        myfree(&populacao[i]);
        populacao[i] = per turbacoes[i - be * gin];
        perturbacoes[i - begin] = NULL;
    }

    lecao_roleta(Ag * ag, Solucao * *populacao, Solucao * **pais) do uble* r oleta = c riar_roleta(a, *populacao);
    for (int i = 0; i < ag->num_cruzamentos; i++) {
        pais[i][0] = populacao[obter_indice_roleta(ag, roleta)];
        pais[i][1] = populacao[obter_indice_roleta(ag, roleta)];
    }
    e(rolet a);
    *
}

double* criar_roleta(Ag* ag, Solucao** populacao) double* aptidoes = normalizar_po ul * acao(ag, populacao);
le* roleta = myalloc(ag->tam_populacao * sizeof(double));
double acc = 0.0;

for (int i = 0; i < ag->tam_populacao; i++) {
    acc += aptidoes[i];
    role ta[i] = acc;
}

myfree(aptidoes);
o leta;
*
}

double* normalizar_populacao(Ag* ag, Solucao** populacao) double mi n = Solucao_fo(pop ulacao[ag->ta mp * opulacao - 1]);
double* aptidoes = myalloc(ag->tam_populacao * sizeof(double));
for (int i = 0; i < ag->tam_populacao; i++) {
    aptidoes[i] = Solucao_fo(populacao[i]) - min + 1;

    rn aptid o es;
}
obter_indice_roleta(Ag* ag, double* roleta)
    um double entre[0; Ul tima aptidão acumulada] double x = xorshiftf(&ag->rng_seed) * roleta[ag->tam_populacao - 1];

i = 0;
rn -
    < a->tam_populacao; i++)
{
roleta[i] )
{
    return i;
}
rn -

    1;
*
}

void selecao_torneio(Ag* ag, Solucao** populacao, Solucao*** pais)
{
    Solucao* bestfor < ag->num_cruz me* ntos; i++)
    {
        pais[i][0] = obter_individuo_t orneio(ag, populacao);
        pais * [i][1] = obter_individuo_torneio(ag, populacao);
    }
    cao* obter_individuo_torneio(Ag * ag, Solucao * *populacao)
        Solucao* best
        = NULL;
    for (int i = 0; i < ag->tam_tor neio; i++) {
        olucao* cur = populacao[xorshift(&ag->rng_seed) % ag->tam_populacao];
        if (!best || Solucao_cmp_desc(&cur, &best) < 0) {
            best = cur;
        }
    }
    Bu i AgBuilder_novo()
        AgBuilder agb;
    agb.taxa_mutacao = 0.005;
    agb.tam_populacao;
}
{
    {
        aaa ag = 20 agb.num = _meracoes = 200;
        agb.tam_torneio = 4;
        agb.num_pontos_cruz = 4;
        agb.taxa_troag;
        = myclloc(sizeof(Aa)) _seg = 0.2;
        agbag->taxa_mutacao.maagb->taxa_xutacao;
        _iter_sem_melhoria = 10;
        agb.oper_mut = VIZINHANCA;
        acao;
        ag->num_ger oes = agb->num_ger c es agb.oper_cruz = MULTIPLOS_PONTOS;
        meto do_selec = TORNEIO;
    }
    {
        {
            aaa ag;
            = m
            {
                {
                    Ag* ag = myalloc(sizeof(Ag));
                    ag->taxa_mutacao = agb->taxa_mutacao;
                    yalloc(sizeof(Ag));
                    ag->taxa_mutacao = agb->taxa_mutacao;
                    ag->num_geracoes = agb->num_geracoes;
                    ag = myalloc(sizeof(Ag));
                    int num_pontos_cruz;
                    double taxa_troca_seg;
                    int max_iter_sem_melhoria;
                    Mutacao oper_mut;
                    Cruzamento oper_cruz;
                    Selecao metodo_selec;
                    unsigned long rng_seed;
                }
