#include <stdio.h>
#include <string.h>
#include <time.h>
#include "constantes.h"
#include "solucao.h"
#include "mem.h"
#include "cronom.h"
#include "ag.h"

#define NUM_ITER_EXPER 3

void
print_crom(Movimento* cromossomo)
{
  for (int i = 0; i < TAM_CROM; i++) {
    printf("%d", cromossomo[i]);
  }
  printf("\n");
}

void
teste_avaliacao()
{
  char cstr[] = "36035300305362402234304654415166425326465425440613260124462532"
                "44266414326202152620660644432540341445313414336556640563351630"
                "54534043220266636353562346042663630315216456253252656455641566"
                "605115431214154445214431613043365440130042415066610554501";
  Movimento* genes = myalloc(TAM_CROM * sizeof(Movimento));

  for (int i = 0; i < TAM_CROM; i++) {
    genes[i] = cstr[i] - '0';
  }

  unsigned long seed = time(NULL);

  Cronom* c = Cronom_novo();
  Solucao* s = Solucao_nova(genes, &seed);
  printf("%g, %gs\n", Solucao_fo(s), Cronom_tempo(c));

  Cronom_free(c);
  Solucao_free(s);
}

void
teste_ag()
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

int
read_one_config_from_file(FILE* fp, Agbuilder* agb, char* id)
{
  char s_oper_cruz[32];
  char s_metodo_selec[32];

  // ID %Mut %Cruz TPop MISM %Per OCruz MSelec
  int rv = fscanf(fp, "%s %lf %lf %d %d %lf %s %s", id, &agb->taxa_mutacao,
                  &agb->taxa_cruzamento, &agb->tam_populacao,
                  &agb->max_iter_sem_melhoria, &agb->taxa_perturbacao,
                  s_oper_cruz, s_metodo_selec);

  if (rv == EOF)
    return EOF;

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

void
run_experimento()
{
  Agbuilder agb = Agbuilder_novo();
  char id[128];

  int rv = read_one_config_from_file(stdin, &agb, id);
  if (rv == EOF) {
    fputs("Erro ao ler a entrada padrao\n", stderr);
  }

  Ag* ag = Ag_create(&agb);
  printf("ID,Exec,FO,Tempo\n");

  for (int i = 0; i < NUM_ITER_EXPER; i++) {
    Cronom* c = Cronom_novo();
    Solucao* s = Ag_resolver(ag);
    printf("%s,%d,%f,%f\n", id, i, Solucao_fo(s), Cronom_tempo(c));
    Solucao_free(s);
    Cronom_free(c);
  }

  Ag_free(ag);
}

void
print_usage()
{
  // clang-format off
 char* usage ="\
    robby [-h|-e]\n\n\
    Parametros:\n\
        <nada> lanca o algoritmo com as configuracoes padrao\n\
        -h     mostra essa mensagem\n\
        -e     executa um experimento. Sera solicitado na entrada padrao um configuracao\n\
               no seguinte formato:\n\
                 ID %Mut %Cruz TPop MISM %Per OCruz MSelec";

  puts(usage);
  // clang-format on
}

int
main(int argc, char** argv)
{
  if (argc != 2) {
    // teste_avaliacao();
    teste_ag();
  } else {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
      print_usage();
    else if (strcmp(argv[1], "-e") || strcmp(argv[1], "--experimento"))
      run_experimento(argv[1]);
    else {
      fputs("Opcao invalida", stderr);
      return 1;
    }
  }
  return 0;
}
