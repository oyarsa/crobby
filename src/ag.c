#define _XOPEN_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
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

Solucao** gerar_individuos_aleatorios(Ag* ag, int num_individuos);
void selecionar(Ag* ag, Solucao** populacao, Solucao*** pais);
void cruzamento(Ag* ag, Solucao*** pais, int** filhos);
void mutacao(Ag* ag, int** cromossomos);
void executar_cruzamento(Ag* ag, Solucao* pai1, Solucao* pai2, int** filho1, int** filho2);
void cruz_multiplos_pontos(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2);
void cruz_segmentado(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2);
void cruz_uniforme(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2);
void cruz_um_ponto(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2);
void cruzar_from_pontos(int* pontos_cruz, int npontos, int* pai1, int* pai2,
    int** filho1, int** filho2);
int* gerar_mascara(Ag* ag);
void vizinhanca(Ag* ag, int** cromossomos);

void avaliar(int** cromossomos, int n, Solucao** solucoes);
int* gerar_solucao_aleatoria(Ag* ag); //
void proxima_geracao(Ag* ag, Solucao** populacao, Solucao** prole); //
void perturbar_populacao(Ag* ag, Solucao** populacao); //
void selecao_roleta(Ag* ag, Solucao** populacao, Solucao*** pais); //
void selecao_torneio(Ag* ag, Solucao** populacao, Solucao*** pais); //
Solucao* obter_individuo_torneio(Ag* ag, Solucao** populacao); //
double* criar_roleta(Ag* ag, Solucao** populacao); //
double* normalizar_populacao(Ag* ag, Solucao** populacao); //
int obter_indice_roleta(Ag* ag, double* roleta); //

Solucao* Ag_resolver(Ag* ag)
{
    assert(ag);

    Solucao** populacao = gerar_individuos_aleatorios(ag, ag->tam_populacao);
    int iter_sem_melhoria = 0;
    double melhor_fo = Solucao_fo(populacao[0]);

    Solucao*** pais = myalloc(ag->num_cruzamentos * sizeof(Solucao**));
    for (int i = 0; i < ag->num_cruzamentos; i++) {
        pais[i] = myalloc(2 * sizeof(Solucao*));
    }

    int** filhos = myalloc(2 * ag->num_cruzamentos * sizeof(int*));
    Solucao** avaliados = myalloc(2 * ag->num_cruzamentos * sizeof(Solucao*));

    for (int i = 0; i < ag->num_geracoes; i++) {
        //printf("%d: %g\n", i + 1, Solucao_fo(populacao[0]));

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

    for (int i = 0; i < ag->num_cruzamentos; i++) {
        myfree(pais[i]);
    }
    myfree(pais);
    myfree(filhos);
    myfree(avaliados);

    for (int i = 1; i < ag->tam_populacao; i++) {
        Solucao_free(populacao[i]);
    }

    Solucao* best = populacao[0];
    myfree(populacao);
    return best;
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
    int j = 0;
    for (int i = 0; i < ag->num_cruzamentos; i++) {
        executar_cruzamento(ag, pais[i][0], pais[i][1], &filhos[j], &filhos[j + 1]);
        j += 2;
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
        pontos_cruz[i] = myrand(&ag->rng_seed) % TAM_CROM;
    }
    pontos_cruz[ag->num_pontos_cruz] = TAM_CROM;
    qsort(pontos_cruz, ag->num_pontos_cruz + 1, sizeof(int), compar_int);
    cruzar_from_pontos(pontos_cruz, ag->num_pontos_cruz + 1, pai1, pai2, filho1, filho2);
}

void cruz_segmentado(Ag* ag, int* pai1, int* pai2, int** filho1, int** filho2)
{
    int* pontos_cruz = myalloc(TAM_CROM * sizeof(int));
    int n = 0;
    for (int i = 0; i < TAM_CROM; i++) {
        double x = myrand(&ag->rng_seed) / (double)MYRAND_MAX;
        if (x <= ag->taxa_troca_seg) {
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
        double x = myrand(&ag->rng_seed) / (double)MYRAND_MAX;
        mask[i] = x <= 0.5 ? 1 : 0;
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
    int ponto = myrand(&ag->rng_seed) % TAM_CROM;

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
                (*filho1)[j] = pai1[j];
                (*filho2)[j] = pai2[j];
            } else {
                (*filho1)[j] = pai2[j];
                (*filho2)[j] = pai1[j];
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
    int num_filhos = 2 * ag->num_cruzamentos;
    for (int i = 0; i < num_filhos; i++) {
        for (int j = 0; j < TAM_CROM; j++) {
            double x = myrand(&ag->rng_seed) / (double)MYRAND_MAX;
            if (x <= ag->taxa_mutacao) {
                cromossomos[i][j] = myrand(&ag->rng_seed) % TOTAL_MOVIMENTOS;
            }
        }
    }
}

Solucao** gerar_individuos_aleatorios(Ag* ag, int num_individuos)
{
    int** pop = myalloc(num_individuos * sizeof(int*));
    for (int i = 0; i < num_individuos; i++) {
        pop[i] = gerar_solucao_aleatoria(ag);
    }
    Solucao** avaliados = myalloc(num_individuos * sizeof(Solucao*));
    avaliar(pop, num_individuos, avaliados);
    myfree(pop);
    return avaliados;
}

int* gerar_solucao_aleatoria(Ag* ag)
{
    int* genes = myalloc(TAM_CROM * sizeof(int));
    for (int i = 0; i < TAM_CROM; i++) {
        genes[i] = myrand(&ag->rng_seed) % TOTAL_MOVIMENTOS;
    }
    return genes;
}

void proxima_geracao(Ag* ag, Solucao** populacao, Solucao** prole)
{
    int begin = ag->tam_populacao - 2 * ag->num_cruzamentos;
    for (int i = begin; i < ag->tam_populacao; i++) {
        Solucao_free(populacao[i]);
        populacao[i] = prole[i - begin];
    }
    qsort(populacao, ag->tam_populacao, sizeof(Solucao*), Solucao_cmp_desc);
}

void perturbar_populacao(Ag* ag, Solucao** populacao)
{
    int n = 0.8 * ag->tam_populacao;
    Solucao** perturbacoes = gerar_individuos_aleatorios(ag, n);
    int begin = ag->tam_populacao - n;
    for (int i = begin; i < ag->tam_populacao; i++) {
        Solucao_free(populacao[i]);
        populacao[i] = perturbacoes[i - begin];
    }
    myfree(perturbacoes);
}

void selecao_roleta(Ag* ag, Solucao** populacao, Solucao*** pais)
{
    double* roleta = criar_roleta(ag, populacao);
    for (int i = 0; i < ag->tam_populacao; i++) {
        pais[i][0] = populacao[obter_indice_roleta(ag, roleta)];
        pais[i][1] = populacao[obter_indice_roleta(ag, roleta)];
    }
}

void selecao_torneio(Ag* ag, Solucao** populacao, Solucao*** pais)
{
    for (int i = 0; i < ag->num_cruzamentos; i++) {
        pais[i][0] = obter_individuo_torneio(ag, populacao);
        pais[i][1] = obter_individuo_torneio(ag, populacao);
    }
}

Solucao* obter_individuo_torneio(Ag* ag, Solucao** populacao)
{
    Solucao* best = NULL;
    for (int i = 0; i < ag->tam_torneio; i++) {
        Solucao* cur = populacao[myrand(&ag->rng_seed) % ag->tam_populacao];
        if (best == NULL || Solucao_cmp_desc(&cur, &best) < 0) {
            best = cur;
        }
    }
    return best;
}

double* criar_roleta(Ag* ag, Solucao** populacao)
{
    double* aptidoes = normalizar_populacao(ag, populacao);
    double* roleta = myalloc(ag->tam_populacao * sizeof(double));
    double acc = 0.0;

    for (int i = 0; i < ag->tam_populacao; i++) {
        acc += aptidoes[i];
        roleta[i] = acc;
    }

    myfree(aptidoes);
    return roleta;
}

double* normalizar_populacao(Ag* ag, Solucao** populacao)
{
    double min = Solucao_fo(populacao[ag->tam_populacao - 1]);
    double* aptidoes = myalloc(ag->tam_populacao * sizeof(double));
    for (int i = 0; i < ag->tam_populacao; i++) {
        aptidoes[i] = Solucao_fo(populacao[i]) - min + 1;
    }
    return aptidoes;
}

int obter_indice_roleta(Ag* ag, double* roleta)
{
    double x = (myrand(&ag->rng_seed) / (double)MYRAND_MAX) * roleta[ag->tam_populacao - 1];
    for (int i = 0; i < ag->tam_populacao; i++) {
        if (roleta[i] > x) {
            return i;
        }
    }
    return -1;
}

void avaliar(int** cromossomos, int n, Solucao** solucoes)
{
    unsigned long seed;
#pragma omp parallel private(seed) shared(n)
    {
        seed = time(NULL) ^ omp_get_thread_num();
#pragma omp for
        for (int i = 0; i < n; i++) {
            solucoes[i] = Solucao_nova(cromossomos[i], &seed);
        }
    }
}

AgBuilder AgBuilder_novo()
{
    AgBuilder agb;
    agb.taxa_mutacao = 0.005;
    agb.taxa_cruzamento = 0.99;
    agb.tam_populacao = 200;
    agb.num_geracoes = 500;
    agb.tam_torneio = 4;
    agb.num_pontos_cruz = 4;
    agb.taxa_troca_seg = 0.2;
    agb.max_iter_sem_melhoria = 10;
    agb.oper_mut = VIZINHANCA;
    agb.oper_cruz = MULTIPLOS_PONTOS;
    agb.metodo_selec = TORNEIO;
    return agb;
}

Ag* Ag_create(AgBuilder* agb)
{
    Ag* ag = myalloc(sizeof(Ag));

    ag->taxa_mutacao = agb->taxa_mutacao;
    ag->tam_populacao = agb->tam_populacao;
    ag->num_geracoes = agb->num_geracoes;
    ag->num_cruzamentos = (int)agb->tam_populacao * agb->taxa_cruzamento / 2;
    ag->tam_torneio = agb->tam_torneio;
    ag->num_pontos_cruz = agb->num_pontos_cruz;
    ag->taxa_troca_seg = agb->taxa_troca_seg;
    ag->max_iter_sem_melhoria = agb->max_iter_sem_melhoria;
    ag->oper_mut = agb->oper_mut;
    ag->oper_cruz = agb->oper_cruz;
    ag->metodo_selec = agb->metodo_selec;
    ag->rng_seed = time(NULL);

    return ag;
}

void Ag_free(Ag* ag)
{
    myfree(ag);
}
