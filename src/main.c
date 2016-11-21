#include <stdio.h>
#include <string.h>
#include <time.h>
#include "constantes.h"
#include "solucao.h"
#include "mem.h"
#include "cronom.h"
#include "ag.h"

/**
 * Número de vezes que o AG será executado para uma configuração durante o
 * experimento.
 */
#define NUM_ITER_EXPER 10

/**
 * Imprime um cromossomo como uma sequência de números seguida por uma quebra de
 * linha.
 */
void
print_crom(Movimento* cromossomo)
{
  for (int i = 0; i < TAM_CROM; i++) {
    printf("%d", cromossomo[i]);
  }
  printf("\n");
}

/**
 * Testa a função de avaliação do cromossomo para um cromossomo arbitrário.
 */
void
teste_avaliacao(void)
{
  // O cromossomo em uma string
  char cstr[] = "36035300305362402234304654415166425326465425440613260124462532"
                "44266414326202152620660644432540341445313414336556640563351630"
                "54534043220266636353562346042663630315216456253252656455641566"
                "605115431214154445214431613043365440130042415066610554501";

  // Converte a string para um cromossomo de fato.
  Movimento* genes = myalloc(TAM_CROM * sizeof(Movimento));
  for (int i = 0; i < TAM_CROM; i++) {
    genes[i] = cstr[i] - '0';
  }

  unsigned long seed = time(NULL);

  // Obtém a FO para o cromossomo e a imprime.
  Cronom* c = Cronom_novo();
  Solucao* s = Solucao_nova(genes, &seed);
  printf("%g, %gs\n", Solucao_fo(s), Cronom_tempo(c));

  Cronom_free(c);
  Solucao_free(s);
}

/**
 * Executa o AG com as configurações padrão, imprimindo o tempo de execução e o
 * cromossomo ao final.
 */
void
teste_ag(void)
{
  Agbuilder agb = Agbuilder_novo();
  agb.num_geracoes = 500;
  agb.taxa_cruzamento = 0.99;
  agb.tam_torneio = 4;
  agb.metodo_selec = TORNEIO;
  agb.num_pontos_cruz = 4;
  Ag* ag = Ag_create(&agb);

  Cronom* c = Cronom_novo();
  Solucao* s = Ag_resolver(ag);
  printf("%g, %gs\n", Solucao_fo(s), Cronom_tempo(c));
  printf("Cromossomo: ");
  print_crom(Solucao_cromossomo(s));

  Cronom_free(c);
  Solucao_free(s);
  Ag_free(ag);
}

/**
 * Lê uma configuração para o AG a partir do arquivo em 'fp'. Modifica o objeto
 * 'agb' com a configuração, assim como 'id' com a id lida. Retorna o valor
 * que fscanf retorna, principalmente EOF se ocorrer um problema.
 */
int
read_one_config_from_file(FILE* fp, Agbuilder* agb, char* id)
{
  // O operador de cruzamento e método de seleção serão lidos como strings.
  // Ex: TORNEIO_2, TORNEIO_4 (para torneios com 2 e 4 indivíduos).
  char s_oper_cruz[32];
  char s_metodo_selec[32];

  // Formato da entrada: ID %Mut %Cruz TPop MaxItSMel %Per OCruz MSelec NGer
  int rv = fscanf(
    fp, "%127s %lf %lf %d %d %lf %31s %31s, %d", id, &agb->taxa_mutacao,
    &agb->taxa_cruzamento, &agb->tam_populacao, &agb->max_iter_sem_melhoria,
    &agb->taxa_perturbacao, s_oper_cruz, s_metodo_selec, &agb->num_geracoes);

  // Interrompe se nada foi lido.
  if (rv == EOF)
    return EOF;

  // Operador de Cruzamento
  // Configura o operador de cruzamento a partir da string obtida. O operador
  // MULTIPLOS_PONTOS possui configurações com 2 e 4 pontos de cruzamento.
  // Se a operador for inválido, avisa o usuário e interrompe o programa.
  if (strcmp(s_oper_cruz, "UM_PONTO") == 0)
    agb->oper_cruz = UM_PONTO;
  else if (strcmp(s_oper_cruz, "UNIFORME") == 0)
    agb->oper_cruz = UNIFORME;
  else if (strcmp(s_oper_cruz, "SEGMENTADO") == 0)
    agb->oper_cruz = SEGMENTADO;
  else if (strcmp(s_oper_cruz, "MULTIPLOS_PONTOS_2") == 0) {
    agb->oper_cruz = MULTIPLOS_PONTOS;
    agb->num_pontos_cruz = 2;
  } else if (strcmp(s_oper_cruz, "MULTIPLOS_PONTOS_4") == 0) {
    agb->oper_cruz = MULTIPLOS_PONTOS;
    agb->num_pontos_cruz = 4;
  } else {
    fprintf(stderr, "Operador de cruzamento invalido\n");
    fclose(fp);
    exit(1);
  }

  // Método de seleção
  // Configura o método de seleção a partir da string obtida. O método TORNEIO
  // possui configurações com 2 e 4 indivíduos. Se o método for inválido, avisa
  // o usuário e interrompe o programa.
  if (strcmp(s_metodo_selec, "ROLETA"))
    agb->metodo_selec = ROLETA;
  else if (strcmp(s_metodo_selec, "TORNEIO_2")) {
    agb->metodo_selec = TORNEIO;
    agb->tam_torneio = 2;
  } else if (strcmp(s_metodo_selec, "TORNEIO_4")) {
    agb->metodo_selec = TORNEIO;
    agb->tam_torneio = 4;
  } else {
    fprintf(stderr, "Metodo de selecao invalido\n");
    fclose(fp);
    exit(1);
  }

  return rv;
}

/**
 * Executa um experimento. Captura as configurações no formato descrito em
 * 'print_usage' e configura o AG. Roda o algoritmo NUM_ITER_EXPER vezes,
 * imprimindo a FO e o Tempo de cada execução.
 */
void
run_experimento()
{
  // Configuração do AG. Será configuradoa partir dos dados lidos.
  Agbuilder agb = Agbuilder_novo();
  // ID do algoritmo. Utilizado para descrever as opções utilizadas. Entrada.
  char id[128];

  // Tenta ler a configuração da entrada padrão. Pode falhar e retornar fim de
  // arquivo (EOF)
  int rv = read_one_config_from_file(stdin, &agb, id);
  if (rv == EOF) {
    fputs("Erro ao ler a entrada padrao\n", stderr);
  }

  // Cria o AG a partir das configurações lidas.
  Ag* ag = Ag_create(&agb);
  // Imprime o cabeçalho dos resultados (útil se eles forem gravados em um CSV).
  printf("ID,Exec,FO,Tempo\n");

  // Executa o algoritmo NUM_ITER_EXPER vezes, imprimindo os resultados.
  for (int i = 0; i < NUM_ITER_EXPER; i++) {
    Cronom* c = Cronom_novo();
    Solucao* s = Ag_resolver(ag);
    printf("%s,%d,%f,%f\n", id, i, Solucao_fo(s), Cronom_tempo(c));
    Solucao_free(s);
    Cronom_free(c);
  }

  Ag_free(ag);
}

/**
 * Imprime o modo de usar do programa.
 */
void
print_usage(void)
{
  // clang-format off
 char* usage ="\
    robby [-h|-e]\n\n\
    Parametros:\n\
        <nada> lanca o algoritmo com as configuracoes padrao\n\
        -h     mostra essa mensagem\n\
        -e     executa um experimento. Sera solicitado na entrada padrao um configuracao\n\
               no seguinte formato:\n\
                 ID %Mut %Cruz TPop MISM %Per OCruz MSelec NGer";

  puts(usage);
  // clang-format on
}

/**
 * Ponto de entrada. Pode receber parâmetros pela linha de comando.
 */
int
main(int argc, char** argv)
{
  // Se não existirem argumentos, executa o AG com as configurações padrão.
  if (argc != 2) {
    teste_ag();
    // Se existirem, testa se é o pedido de ajuda ou a sinalização de que um
    // experimento está ocorrendo.
  } else {
    // Se for ajuda, imprime o guia de uso do programa.
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
      print_usage();
    // Se for um experimento, ele é iniciado.
    else if (strcmp(argv[1], "-e") || strcmp(argv[1], "--experimento"))
      run_experimento();
    // Também pode ser uma opção inválida. Nesse caso, o programa é encerrado.
    else {
      fputs("Opcao invalida", stderr);
      return 1;
    }
  }
  return 0;
}
