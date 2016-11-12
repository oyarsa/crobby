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

int* gerar_solucao_aleatoria(Ag* ag)
{
    int* genes = myalloc(TAM_CROM * sizeof(int));
    for (int i = 0; i < TAM_CROM; i++) {
        genes[i] = xorshift(&ag->rng_seed) % TOTAL_MOVIMENTOS;
    }
    return genes;
}

void proxima_geracao(Ag* ag, Solucao** populacao, Solucao** prole)
{
    int begin = ag->tam_populacao - 2 * ag->num_cruzamentos;
    for (int i = begin; i < ag->tam_populacao; i++) {
        myfree(&populacao[i]);
        populacao[i] = prole[i - begin];
        prole[i - begin] = NULL;
    }
    qsort(populacao, ag->tam_populacao, sizeof(Solucao*), Solucao_cmp_desc);
}

void perturbar_populacao(Ag* ag, Solucao** populacao)
{
}

void selecao_roleta(Ag* ag, Solucao** populacao, Solucao*** pais)
{
}

void selecao_torneio(Ag* ag, Solucao** populacao, Solucao*** pais)
{
}

Solucao* obter_individuo_torneio(Ag* ag, Solucao** populacao)
{
    return NULL;
}

double* criar_roleta(Ag* ag, Solucao** populacao)
{
    return NULL;
}

double* normalizar_populacao(Ag* ag, Solucao** populacao)
{
    return NULL;
}

int obter_indice_roleta(Ag* ag, double* roleta)
{
    return 0;
}

void avaliar(int** cromossomos, int n, Solucao** solucoes)
{
    unsigned long seed = time(NULL);
    for (int i = 0; i < n; i++) {
        solucoes[i] = Solucao_nova(cromossomos[i], &seed);
    }
}
