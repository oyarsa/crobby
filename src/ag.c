#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <float.h>
#include <omp.h>
#include "ag.h"
#include "cronom.h"
#include "constantes.h"
#include "mem.h"
#include "rand.h"

/**
 * Configuração do Ag. Contém também o estado atual da semente do PRNG.
 */
struct Ag
{
  double taxa_mutacao;
  int tam_populacao;
  int num_geracoes;
  int num_cruzamentos;
  int tam_torneio;
  int num_pontos_cruz;
  double taxa_troca_seg;
  int max_iter_sem_melhoria;
  double taxa_perturbacao;
  Mutacao oper_mut;
  Cruzamento oper_cruz;
  Selecao metodo_selec;
  double tempo_max;
  unsigned long rng_seed;
};

// Protótipos das funções.
Solucao** gerar_individuos_aleatorios(Ag* ag, int num_individuos);
void selecionar(Ag* ag, Solucao** populacao, Solucao*** pais);
void cruzamento(Ag* ag, Solucao*** pais, Movimento** filhos);
void mutacao(Ag* ag, Movimento** cromossomos);
void executar_cruzamento(Ag* ag, Solucao* pai1, Solucao* pai2,
                         Movimento** filho1, Movimento** filho2);
void cruz_multiplos_pontos(Ag* ag, Movimento* pai1, Movimento* pai2,
                           Movimento** filho1, Movimento** filho2);
void cruz_segmentado(Ag* ag, Movimento* pai1, Movimento* pai2,
                     Movimento** filho1, Movimento** filho2);
void cruz_uniforme(Ag* ag, Movimento* pai1, Movimento* pai2, Movimento** filho1,
                   Movimento** filho2);
void cruz_um_ponto(Ag* ag, Movimento* pai1, Movimento* pai2, Movimento** filho1,
                   Movimento** filho2);
void cruzar_from_pontos(int* pontos_cruz, int npontos, Movimento* pai1,
                        Movimento* pai2, Movimento** filho1,
                        Movimento** filho2);
int* gerar_mascara(Ag* ag);
void vizinhanca(Ag* ag, Movimento** cromossomos);
void avaliar(Movimento** cromossomos, int n, Solucao** solucoes);
Movimento* gerar_solucao_aleatoria(Ag* ag);
void proxima_geracao(Ag* ag, Solucao** populacao, Solucao** prole);
void perturbar_populacao(Ag* ag, Solucao** populacao);
void selecao_roleta(Ag* ag, Solucao** populacao, Solucao*** pais);
void selecao_torneio(Ag* ag, Solucao** populacao, Solucao*** pais);
Solucao* obter_individuo_torneio(Ag* ag, Solucao** populacao);
double* criar_roleta(Ag* ag, Solucao** populacao);
double* normalizar_populacao(Ag* ag, Solucao** populacao);
int obter_indice_roleta(Ag* ag, double* roleta);

Solucao*
Ag_resolver(Ag* ag)
{
  // Inicializa o cronômetro, gera a população inicial e determina a melhor_fo
  // como sendo o melhor indivíduo desta.
  Cronom* c = Cronom_novo();

  Solucao** populacao = gerar_individuos_aleatorios(ag, ag->tam_populacao);
  qsort(populacao, ag->tam_populacao, sizeof(Solucao*), Solucao_cmp_desc);

  int iter_sem_melhoria = 0;
  double melhor_fo = Solucao_fo(populacao[0]);

  // Aloca memória para o vetor que irá guardar os pares de pais selecionados
  // para cruzamento.
  Solucao*** pais = myalloc(ag->num_cruzamentos * sizeof(Solucao**));
  for (int i = 0; i < ag->num_cruzamentos; i++) {
    pais[i] = myalloc(2 * sizeof(Solucao*));
  }

  // Aloca memória para o vetor de filhos e de suas soluções.
  Movimento** filhos = myalloc(2 * ag->num_cruzamentos * sizeof(Movimento*));
  Solucao** avaliados = myalloc(2 * ag->num_cruzamentos * sizeof(Solucao*));

  // Executa até um máximo de num_geracoes ou até o tempo_max ser excedido.
  for (int i = 0; i < ag->num_geracoes && Cronom_tempo(c) < ag->tempo_max;
       i++) {

// Imprime a FO do melhor indivíduo de cada iteração.
#ifdef MOSTRAR_ITERACOES
    printf("%d: %g\n", i + 1, Solucao_fo(populacao[0]));
#endif

    // Coração do algoritmo:
    // Gera os pares de pais a serem cruzados
    selecionar(ag, populacao, pais);
    // Gera 2 filhos para cada cruzamento
    cruzamento(ag, pais, filhos);
    // Potencialmente aplica mutações sobre eles
    mutacao(ag, filhos);
    // Calcula suas aptidões
    avaliar(filhos, 2 * ag->num_cruzamentos, avaliados);
    // Determina quais indivíduos irão para próxima geração.
    proxima_geracao(ag, populacao, avaliados);

    // Determina se houve uma melhoria em relação à última iteração. Se sim,
    // reseta o contador.
    if (Solucao_fo(populacao[0]) > melhor_fo) {
      melhor_fo = Solucao_fo(populacao[0]);
      iter_sem_melhoria = 0;
    } else {
      iter_sem_melhoria++;
    }

    // Se o máximo de iterações sem melhoria for atingido, efetua uma
    // perturbação na população e reseta o contador.
    if (iter_sem_melhoria == ag->max_iter_sem_melhoria) {
      perturbar_populacao(ag, populacao);
      iter_sem_melhoria = 0;
    }
  }

  // Limpa a memória alocada.
  for (int i = 0; i < ag->num_cruzamentos; i++) {
    free(pais[i]);
  }
  free(pais);
  free(filhos);
  free(avaliados);

  // Limpa todos os indivíduos exceto o melhor (índice 0)
  for (int i = 1; i < ag->tam_populacao; i++) {
    Solucao_free(populacao[i]);
  }

  // Salva o ponteiro para o melhor indivíduo
  Solucao* best = populacao[0];

  // Limpa então o vetor. E o cronômetro.
  free(populacao);
  Cronom_free(c);

  return best;
}

/**
 * Efetua a seleção dos pais que irão passar pelo cruzamento. O método de
 * seleção é configurado em 'ag'. Retorna um vetor de pares de pais através
 * do parâmetros 'pais'.
 */
void
selecionar(Ag* ag, Solucao** populacao, Solucao*** pais)
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

/**
 * Efetua o cruzamento, gerando um vetor de cromossomos filhos. O operador é
 * configurado em 'ag'. Retorna os filhos através do parâmetro 'filhos'.
 */
void
cruzamento(Ag* ag, Solucao*** pais, Movimento** filhos)
{
  // Cada cruzamento gera 2 filhos. O vetor filhos deve ter o número de posições
  // como o dobro do número de cruzamentos.
  // O primeiro pai estará em pai[i][0], o segundo está em pai[i][1]. O primeiro
  // filho irá para filhos[j] e o segundo irá para filhos[j + 1]. Serão passados
  // ponteiros para esses elementos pois eles serão alocados pelos operadores.
  int j = 0;
  for (int i = 0; i < ag->num_cruzamentos; i++) {
    executar_cruzamento(ag, pais[i][0], pais[i][1], &filhos[j], &filhos[j + 1]);
    // Avança duas posições no vetor de filhos.
    j += 2;
  }
}

/**
 * Efetua o cruzamento de dois pais, gerando dois filhos, que serão retornados
 * nos parâmetros filho1 e filho2. O operador de seleção é obtido através da
 * configuração em 'ag'.
 */
void
executar_cruzamento(Ag* ag, Solucao* pai1, Solucao* pai2, Movimento** filho1,
                    Movimento** filho2)
{
  switch (ag->oper_cruz) {
    case MULTIPLOS_PONTOS:
      cruz_multiplos_pontos(ag, Solucao_cromossomo(pai1),
                            Solucao_cromossomo(pai2), filho1, filho2);
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

/**
 * Função de comparação de dois inteiros. Utilizado no qsort.
 */
int
compar_int(const void* a, const void* b)
{
  const int* ia = a;
  const int* ib = b;
  return *ia - *ib;
}

/**
 * Cruzamento de múltiplos pontos.
 * Efetua um cruzamento em n pontos no cromossomo. O número de pontos de
 * cruzamento é configurado em 'ag'.
 */
void
cruz_multiplos_pontos(Ag* ag, Movimento* pai1, Movimento* pai2,
                      Movimento** filho1, Movimento** filho2)
{
  // Aloca num_pontos_cruz + 1 porque o final do vetor deve sempre ser inserido
  // como ponto de cruzamento.
  int* pontos_cruz = myalloc((ag->num_pontos_cruz + 1) * sizeof(int));
  // Gera aleatoriamente n pontos de cruzamento.
  for (int i = 0; i < ag->num_pontos_cruz; i++) {
    pontos_cruz[i] = myrand(&ag->rng_seed) % TAM_CROM;
  }

  // Coloca o final do vetor como um ponto também.
  pontos_cruz[ag->num_pontos_cruz] = TAM_CROM;
  // Ordena o vetor para o pontos aparecem em ordem.
  qsort(pontos_cruz, ag->num_pontos_cruz + 1, sizeof(int), compar_int);

  // Efetua o cruzamento a partir desses pontos.
  cruzar_from_pontos(pontos_cruz, ag->num_pontos_cruz + 1, pai1, pai2, filho1,
                     filho2);
  free(pontos_cruz);
}

/**
 * Cruzamento segmentado.
 * Cada ponto do cromossomo possui uma chance (taxa_troca_seg) de ser um ponto
 * de cruzamento.
 */
void
cruz_segmentado(Ag* ag, Movimento* pai1, Movimento* pai2, Movimento** filho1,
                Movimento** filho2)
{
  // Vetor que irá guardar os pontos de cruzamento obtidos.
  int pontos_cruz[TAM_CROM];
  int n = 0;

  // Percorre o cromossomo decidindo se a i-ésima posição será um ponto de
  // cruzamento, a partir de uma chance pré-determinada.
  for (int i = 0; i < TAM_CROM - 1; i++) {
    double x = myrand(&ag->rng_seed) / (double)MYRAND_MAX;
    if (x <= ag->taxa_troca_seg) {
      pontos_cruz[n] = i;
      n++;
    }
  }
  // Insere o final do vetor como um ponto de cruzamento.
  pontos_cruz[n] = TAM_CROM;
  n++;

  // Efetua o cruzamento a partir desses pontos.
  cruzar_from_pontos(pontos_cruz, n, pai1, pai2, filho1, filho2);
}

/**
 * Gera uma máscara de cruzamento que determina de que pai cada gene virá,
 * com chance de 50% para cada pai.
 */
int*
gerar_mascara(Ag* ag)
{
  int* mask = myalloc(TAM_CROM * sizeof(int));
  for (int i = 0; i < TAM_CROM; i++) {
    double x = myrand(&ag->rng_seed) / (double)MYRAND_MAX;
    // Determina se o i-ésimo gene virá do pai 0 ou 1
    mask[i] = x <= 0.5 ? 1 : 0;
  }
  return mask;
}

/**
 * Cruzamento uniforme.
 * Efetua o cruzamento a partir de uma máscara obtida aleatóriamente, que
 * determina de onde vem cada gene. São obtidos dois filhos, o segundo
 * invertendo os pais.
 */
void
cruz_uniforme(Ag* ag, Movimento* pai1, Movimento* pai2, Movimento** filho1,
              Movimento** filho2)
{
  int* mask = gerar_mascara(ag);
  // Aloca memória para os filhos.
  *filho1 = myalloc(TAM_CROM * sizeof(Movimento));
  *filho2 = myalloc(TAM_CROM * sizeof(Movimento));

  // Percorre o cromossomo, copiando os genes dependendo de qual pai foi
  // assinalado pela máscara.
  for (int i = 0; i < TAM_CROM; i++) {
    if (mask[i] == 0) {
      (*filho1)[i] = pai1[i];
      (*filho2)[i] = pai2[i];
    } else {
      (*filho1)[i] = pai2[i];
      (*filho2)[i] = pai1[i];
    }
  }

  free(mask);
}

/**
 * Cruzamento em um ponto.
 * Obtém aleatoriamente um ponto no cromossomo, e copia todos os genes até
 * aquele ponto de um pai, e o restante de um outro.
 */
void
cruz_um_ponto(Ag* ag, Movimento* pai1, Movimento* pai2, Movimento** filho1,
              Movimento** filho2)
{
  // Aloca memória para os filhos.
  *filho1 = myalloc(TAM_CROM * sizeof(Movimento));
  *filho2 = myalloc(TAM_CROM * sizeof(Movimento));
  // Obtém o ponto de cruzamento.
  int ponto = myrand(&ag->rng_seed) % TAM_CROM;

  // Copia todos os genes até aquele ponto de um pai.
  memcpy(*filho1, pai1, ponto);
  memcpy(*filho2, pai2, ponto);

  // Copia o restante do outro pai.
  memcpy(*filho1 + ponto, pai2 + ponto, TAM_CROM - ponto);
  memcpy(*filho2 + ponto, pai1 + ponto, TAM_CROM - ponto);
}

/**
 * Efetua cruzamento a partir de um vetor ordenado de pontos de cruzamento.
 * Ao alcançar cada ponto, alterna qual pai cede o gene para o filho.
 */
void
cruzar_from_pontos(int* pontos_cruz, int npontos, Movimento* pai1,
                   Movimento* pai2, Movimento** filho1, Movimento** filho2)
{
  // Aloca memória para os filhos
  *filho1 = myalloc(TAM_CROM * sizeof(Movimento));
  *filho2 = myalloc(TAM_CROM * sizeof(Movimento));

  // Sinaliza se os pais estão invertidos
  bool flip = false;
  // Índice do gene no cromossomo
  int j = 0;

  // Percorre o vetor de pontos
  for (int i = 0; i < npontos; i++) {
    // Para cada trecho, percorre as posições até ele e copia os genes do pai
    // atual
    for (; j < pontos_cruz[i]; j++) {
      if (flip) {
        (*filho1)[j] = pai1[j];
        (*filho2)[j] = pai2[j];
      } else {
        (*filho1)[j] = pai2[j];
        (*filho2)[j] = pai1[j];
      }
    }
    // Ao final do trecho, alterna os pais
    flip = !flip;
  }
}

/**
 * Efetua a mutação, de acordo com o operador configurado em 'ag'.
 */
void
mutacao(Ag* ag, Movimento** cromossomos)
{
  switch (ag->oper_mut) {
    case VIZINHANCA:
      vizinhanca(ag, cromossomos);
  }
}

/**
 * Efetua a mutação por vizinhança uniforme. Cada gene de cada cromossomo possui
 * uma chance (configurada em 'ag') de ser alterado para um movimento aleatório.
 */
void
vizinhanca(Ag* ag, Movimento** cromossomos)
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

/**
 * Gera 'num_individuos' aleatórios, retornando um vetor com as Solucoes
 * avaliadas.
 */
Solucao**
gerar_individuos_aleatorios(Ag* ag, int num_individuos)
{
  Movimento** pop = myalloc(num_individuos * sizeof(Movimento*));
  for (int i = 0; i < num_individuos; i++) {
    pop[i] = gerar_solucao_aleatoria(ag);
  }
  Solucao** avaliados = myalloc(num_individuos * sizeof(Solucao*));
  avaliar(pop, num_individuos, avaliados);
  free(pop);
  return avaliados;
}

/**
 * Gera um cromossomo aleatório.
 */
Movimento*
gerar_solucao_aleatoria(Ag* ag)
{
  Movimento* genes = myalloc(TAM_CROM * sizeof(Movimento));
  for (int i = 0; i < TAM_CROM; i++) {
    genes[i] = myrand(&ag->rng_seed) % TOTAL_MOVIMENTOS;
  }
  return genes;
}

/**
 * Determina quais indivíduos irão compor a próxima geração, através da
 * composição da população e da prole. Toda a prole vai para a próxima geração,
 * o número restante de elementos que faltam para completar 'tam_populacao' é
 * retirado dos melhores indivíduos da 'populacao'.
 */
void
proxima_geracao(Ag* ag, Solucao** populacao, Solucao** prole)
{
  // Determina em que posição os filhos serão inseridos na população.
  // As anteriores (os melhores da geração anterior) são mantidos.
  int begin = ag->tam_populacao - 2 * ag->num_cruzamentos;
  for (int i = begin; i < ag->tam_populacao; i++) {
    Solucao_free(populacao[i]);
    populacao[i] = prole[i - begin];
  }
  // Ordena a população de acordo com a FO, em ordem decrescente.
  qsort(populacao, ag->tam_populacao, sizeof(Solucao*), Solucao_cmp_desc);
}

/**
 * Executa uma perturbação na população, gerando novos indivíduos e substituindo
 * os piores.
 */
void
perturbar_populacao(Ag* ag, Solucao** populacao)
{
  // Determina o número de indivíduos a serem gerados
  int n = ag->taxa_perturbacao * ag->tam_populacao;
  // Gera esses indivíduos aleatórios
  Solucao** perturbacoes = gerar_individuos_aleatorios(ag, n);

  // Insere esses indivíduos no final da população
  int begin = ag->tam_populacao - n;
  for (int i = begin; i < ag->tam_populacao; i++) {
    Solucao_free(populacao[i]);
    populacao[i] = perturbacoes[i - begin];
  }

  free(perturbacoes);
}

/**
 * Seleção por roleta.
 * Obtém 'num_cruzamentos' pares de pais, cada par obtido através da geração de
 * dois índices aleatórios por meio de uma roleta simples (probabilidade
 * proporcional). Os pares são retornados em 'pais'.
 */
void
selecao_roleta(Ag* ag, Solucao** populacao, Solucao*** pais)
{
  double* roleta = criar_roleta(ag, populacao);
  for (int i = 0; i < ag->num_cruzamentos; i++) {
    pais[i][0] = populacao[obter_indice_roleta(ag, roleta)];
    pais[i][1] = populacao[obter_indice_roleta(ag, roleta)];
  }
  free(roleta);
}

/**
 * Seleção por torneio.
 * Obtém 'num_cruzamentos' pares de pais, cada pai de cada par obtido através de
 * um torneio n-ário, com tamanho configurado em 'ag'. Os pares são retornados
 * em 'pais'.
 */
void
selecao_torneio(Ag* ag, Solucao** populacao, Solucao*** pais)
{
  for (int i = 0; i < ag->num_cruzamentos; i++) {
    pais[i][0] = obter_individuo_torneio(ag, populacao);
    pais[i][1] = obter_individuo_torneio(ag, populacao);
  }
}

/**
 * Obtém um indivíduo através de um torneio n-ário. São obtidos n indivíduos
 * aleatoriamente da população, e o selecionado é aquele com melhor fo.
 */
Solucao*
obter_individuo_torneio(Ag* ag, Solucao** populacao)
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

/**
 * Cria uma roleta simples (probabilidade proporcional) através da população.
 * A roleta não lida com valores nulos ou negativos, então antes é efetuada uma
 * normalização nas fos. A roleta pronta é retornada.
 */
double*
criar_roleta(Ag* ag, Solucao** populacao)
{
  double* aptidoes = normalizar_populacao(ag, populacao);
  double* roleta = myalloc(ag->tam_populacao * sizeof(double));
  double acc = 0.0;

  for (int i = 0; i < ag->tam_populacao; i++) {
    acc += aptidoes[i];
    roleta[i] = acc;
  }

  free(aptidoes);
  return roleta;
}

/**
 * Normaliza as FOs da população, uma vez que a roleta só lida com valores
 * positivos. Essa normalização é obtida ao encontrar a menor FO a população, e
 * então somar o -min+1 a todas as FOs. Isso faz com que a menor FO passe a
 * valer 1 e as outras sejam ajustadas de acordo.
 */
double*
normalizar_populacao(Ag* ag, Solucao** populacao)
{
  double min = Solucao_fo(populacao[ag->tam_populacao - 1]);
  double* aptidoes = myalloc(ag->tam_populacao * sizeof(double));
  for (int i = 0; i < ag->tam_populacao; i++) {
    aptidoes[i] = Solucao_fo(populacao[i]) - min + 1;
  }
  return aptidoes;
}

/**
 * Obtém um índice aleatório a partir da roleta.
 * Um número aleatório 'x' é gerado, e o primeiro elemento da roleta com FO
 * acumulada maior que 'x' tem seu índice retornado.
 */
int
obter_indice_roleta(Ag* ag, double* roleta)
{
  // Número aleatório entre 0 e a maior FO acumulada
  double x = (myrand(&ag->rng_seed) / (double)MYRAND_MAX) *
             roleta[ag->tam_populacao - 1];
  for (int i = 0; i < ag->tam_populacao; i++) {
    if (roleta[i] > x) {
      return i;
    }
  }
  // Não deve acontecer.
  fprintf(stderr, "Isso não era pra acontecer\n");
  exit(123);
  return -1;
}

/**
 * Avalia os cromossomos, obtendo soluções com FOs calculadas. Cada cromossomo
 * é avaliado em paralelo.
 */
void
avaliar(Movimento** cromossomos, int n, Solucao** solucoes)
{
  unsigned long seed;
#pragma omp parallel private(seed)
  {
    seed = time(NULL) ^ omp_get_thread_num();
#pragma omp for
    for (int i = 0; i < n; i++) {
      solucoes[i] = Solucao_nova(cromossomos[i], &seed);
    }
  }
}

Agbuilder
Agbuilder_novo(void)
{
  Agbuilder agb;
  agb.taxa_mutacao = 0.005;
  agb.taxa_cruzamento = 0.99;
  agb.tam_populacao = 200;
  agb.num_geracoes = 500;
  agb.tam_torneio = 4;
  agb.num_pontos_cruz = 4;
  agb.taxa_troca_seg = 0.2;
  agb.max_iter_sem_melhoria = 10;
  agb.taxa_perturbacao = 0.5;
  agb.oper_mut = VIZINHANCA;
  agb.oper_cruz = MULTIPLOS_PONTOS;
  agb.metodo_selec = TORNEIO;
  agb.tempo_max = DBL_MAX; // Tempo "infinito"
  return agb;
}

Ag*
Ag_create(Agbuilder* agb)
{
  Ag* ag = myalloc(sizeof(Ag));

  // O número de cruzamentos é metade da multiplicação da porcentagem com o
  // tamanho da população porque ele diz respeito ao número de par de pais
  // a serem selecionados, e cada par gerará dois filhos.
  ag->num_cruzamentos = (int)agb->tam_populacao * agb->taxa_cruzamento / 2;

  ag->taxa_mutacao = agb->taxa_mutacao;
  ag->tam_populacao = agb->tam_populacao;
  ag->num_geracoes = agb->num_geracoes;
  ag->tam_torneio = agb->tam_torneio;
  ag->num_pontos_cruz = agb->num_pontos_cruz;
  ag->taxa_troca_seg = agb->taxa_troca_seg;
  ag->max_iter_sem_melhoria = agb->max_iter_sem_melhoria;
  ag->taxa_perturbacao = agb->taxa_perturbacao;
  ag->tempo_max = agb->tempo_max;
  ag->oper_mut = agb->oper_mut;
  ag->oper_cruz = agb->oper_cruz;
  ag->metodo_selec = agb->metodo_selec;
  ag->rng_seed = time(NULL);

  return ag;
}

void
Ag_free(Ag* ag)
{
  free(ag);
}
