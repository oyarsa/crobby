#include <stdio.h>
#include <time.h>
#include "constantes.h"
#include "solucao.h"
#include "mem.h"
#include "cronom.h"
#include "ag.h"

#include <pthread.h>
#include <winsock2.h>
#include <windows.h>
#include <mstcpip.h>
#include <curl/curl.h>

#define NUM_ITER_EXPER 3

void print_crom(int* cromossomo)
{
    for (int i = 0; i < TAM_CROM; i++) {
        printf("%d", cromossomo[i]);
    }
    printf("\n");
}

void teste_avaliacao()
{
    char cstr[] = "360353003053624022343046544151664253264654254406132601244625324426641432620215262066064443254034144531341433655664056335163054534043220266636353562346042663630315216456253252656455641566605115431214154445214431613043365440130042415066610554501";
    int* genes = myalloc(TAM_CROM * sizeof(int));

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

void teste_ag()
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

int read_one_config_from_file(FILE* fp, Agbuilder* agb, char* id)
{
    int i_oper_cruz;
    int i_metodo_selec;

    // ID %Mut %Cruz TPop TTor NPCruz MISM %Per OCruz MSelec
    int rv = fscanf(fp, "%s %lf %lf %d %d %d %d %lf %d %d", id,
        &agb->taxa_mutacao, &agb->taxa_cruzamento, &agb->tam_populacao,
        &agb->tam_torneio, &agb->num_pontos_cruz, &agb->max_iter_sem_melhoria,
        &agb->taxa_perturbacao, &i_oper_cruz, &i_metodo_selec);

    if (rv == EOF)
        return EOF;

    switch (i_oper_cruz) {
    case 1:
        agb->oper_cruz = UM_PONTO;
        break;
    case 2:
        agb->oper_cruz = MULTIPLOS_PONTOS;
        break;
    case 3:
        agb->oper_cruz = SEGMENTADO;
        break;
    case 4:
        agb->oper_cruz = UNIFORME;
        break;
    default:
        fprintf(stderr, "Operador de cruzamento invalido\n");
        fclose(fp);
        exit(1);
    }

    switch (i_metodo_selec) {
    case 1:
        agb->metodo_selec = TORNEIO;
        break;
    case 2:
        agb->metodo_selec = ROLETA;
        break;
    default:
        fprintf(stderr, "Metodo de selecao invalido\n");
        fclose(fp);
        exit(1);
    }

    return rv;
}

void* send_result(void* vdata)
{
    char** data = (char**)vdata;
    char* nome = (char*)data[0];
    char* result = (char*)data[1];
    char* server = (char*)data[2];

    CURL* curl = curl_easy_init();
    CURLcode err;
    char buf[1024];
    snprintf(buf, 1024, "nome=%s&result=%s", nome, result);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
    curl_easy_setopt(curl, CURLOPT_URL, server);
    while ((err = curl_easy_perform(curl)) != CURLE_OK) {
        fprintf(stderr, "Um erro ocorreu: %s\n", curl_easy_strerror(err));
        Sleep(1000);
    }
    curl_free(curl);

    return NULL;
}

void run_experimento(char* infile, char* server)
{
    FILE* in = fopen(infile, "r");
    Agbuilder agb = Agbuilder_novo();
    char id[128];
    char out[1024];
    pthread_t threads[50];
    int nthreads = 0;

    while (read_one_config_from_file(in, &agb, id) != EOF) {
        Ag* ag = Ag_create(&agb);

        snprintf(out, 1024, "Iteracao,FO\r\n");
        for (int i = 0; i < NUM_ITER_EXPER; i++) {
            Solucao* s = Ag_resolver(ag);
            snprintf(out, 1024, "%s%d %f\r\n", out, i, Solucao_fo(s));
            Solucao_free(s);
        }

        Ag_free(ag);
        char* data[] = { id, out, server };
        pthread_create(&threads[nthreads++], NULL, send_result, data);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(in);
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        //teste_avaliacao();
        teste_ag();
    } else {
        run_experimento(argv[1], argv[2]);
    }
}
