#include "utils.h"
#include "algoritmos/second_chance.h"
#include "algoritmos/nru.h"
#include "algoritmos/lru.h"
#include "algoritmos/working_set.h"

// Processos
#include <unistd.h>

// Threads
#include <pthread.h>

// PIPEs
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/types.h>

// Outros
#include <string.h>

// PIPEs
void* gmv(void *arg);
void abrePipes(void);
int checaPipes(char *retorno);

// GMV
void limpaMemoriaGMV(void);
void imprimeTabelas(void);
void criaMemoriaRAM(void);
int checaMemoriaVazia(BasePage *pagina);
int procuraMemoriaVazia(void);
int procuraPaginaExistente(char *dados);
void atribuiPagina(char *dados, int idx_memoria, int idx_processo);
void acionaRedistribuicao(char *dados, int idx_memoria);

// Outros
int checaFim(void);
extern char algoritmo[10]; // ex: "2nCH", "NRU", "LRU", "WS"

// Essenciais
int *pipes;
BasePage** memoria_ram;
BasePage pagina_vazia = {-1, '-', -1, NULL};

// Auxiliares
extern int flag_main;
extern int flag_gmv;
extern int paginas_lidas;
extern pthread_mutex_t mutex;
int processo_que_fez_falta = -1;
int paginas_substituidas = 0;
extern int tempo_global;


// PIPEs

void* gmv(void *arg)
{
    LOG("\n>> Gerenciador de Memória Virtual iniciado...\n");

    abrePipes();
    criaMemoriaRAM();

    char buffer[10]; int idx_memoria, idx_processo;
    while(flag_gmv)
    {
        // Confere se algum processo enviou uma mensagem
        if ( (idx_processo = checaPipes(buffer)) != -1)
        { 
            processo_que_fez_falta = idx_processo;
            LOG(">> Checando permissão para continuar via mutex...\n");
            pthread_mutex_lock(&mutex);
            LOG(">> Permissão concedida!\n");

            idx_memoria = procuraMemoriaVazia();

            if (idx_memoria != -1)
            {
                printf(">> Espaço de memória vago no índice %d. Acionando registro de página...\n", idx_memoria);

                atribuiPagina(buffer, idx_memoria, idx_processo); 

                printf(">> Registro concluído!\n");
            }
            else
            {
                idx_memoria = procuraPaginaExistente(buffer);

                // Redistribuição
                if (idx_memoria == -1)
                {
                    printf(">> Não há nenhum espaço disponível na memória. Acionando redistruição de páginas...\n");

                    acionaRedistribuicao(buffer, idx_memoria); 

                    printf(">> Redistribuição concluída!\n");
                }

                // Página existente
                else
                {
                    printf(">> A página já está presente na memória!\n");
                }
            }

            paginas_lidas++; 

            if (strcmp(algoritmo, "NRU") == 0 && paginas_lidas % 10 == 0) 
            {
                atualizaBitsNRU(memoria_ram); // zera bits R a cada 10 páginas
            }

            if (strcmp(algoritmo, "LRU") == 0) 
            {
                atualizaContadoresLRU(memoria_ram, idx_processo);
            }

            if (strcmp(algoritmo, "WS") == 0) 
            {
                atualizaContadoresWS(memoria_ram, idx_processo);
                tempo_global++;
            }

            printf(">> %d páginas lidas\n", paginas_lidas);

            pthread_mutex_unlock(&mutex);
            LOG(">> Trecho em mutex finalizado\n");
        }
        if ( checaFim() ){ break; }
    }

    LOG("\n>> Todos os processos conseguiram colocar suas páginas na memória. Encerrando GMV...\n");

    printf(">> Total de páginas substituídas: %d\n", paginas_substituidas);

    LOG(">> Gerenciador de Memória Virtual encerrado!\n");

    imprimeTabelas();

    limpaMemoriaGMV();

    return NULL;
}

void abrePipes(void)
{
    LOG(">> Iniciando a abertura das PIPEs...\n");

    pipes = (int*)malloc(sizeof(int)*QTD_PROCESSOS);

    char nome_pipe[50];

    forProcessos(i)
    {
        LOG(">> Abrindo a PIPE %d...\n", i+1);

        sprintf(nome_pipe, "pipes/pipe%d", i+1);
        pipes[i] = open(nome_pipe, READ_MODE);
        if (pipes[i] < 0){ fprintf(stderr, "(!) Erro na abertura da PIPE %d", i+1); exit(1); }

        LOG(">> PIPE %d aberta!\n", i+1);
    }

    LOG(">> Todas as PIPEs foram abertas!\n");
}

int checaPipes(char *retorno)
{
    char buffer[10]; int tam;

    forProcessos(i){
        tam = read(pipes[i], &buffer, sizeof(buffer));
        if (tam > 0)
        {
            sleep(1); // Tempo para o processo acabar seu loop
            LOG("\n>> Nova mensagem na PIPE %d", i+1);

            buffer[tam] = '\0';
            printf("\n>> GMV - %s", buffer);
            strcpy(retorno, buffer);
            return i;
        }
    }

    return -1;
}

// GMV

void limpaMemoriaGMV(void)
{
    LOG("\n>> Iniciando a limpeza da memória da GMV...\n");

    LOG("\n>> Ordenando o fechamento das PIPEs...\n");
    forProcessos(i)
    { 
        LOG(">> Fechando a PIPE %d...\n", i+1);
        close(pipes[i]); 
        LOG(">> PIPE %d fechada!\n", i+1);
    }
    LOG("\n>> Todas as PIPEs foram fechadas!\n");

    LOG("\n>> Liberando a memória alocada para Memória RAM...\n");

    forMemoria(i) 
    {
        if (memoria_ram[i]->extra != NULL) 
        {
            free(memoria_ram[i]->extra);
        }
        free(memoria_ram[i]);
    }

    free(memoria_ram);
    LOG("\n>> Memória liberada!\n");

    LOG(">> Memória da GMV limpa!\n");
}

void imprimeTabelas(void) {
    printf("\nTabela de Páginas Final na RAM:\n");

    if (memoria_ram == NULL) {
        printf("(!) Erro: memoria_ram não foi alocada\n");
        return;
    }

    for (int p = 0; p < QTD_PROCESSOS; p++) {
        printf("P%d: ", p + 1);
        for (int i = 0; i < MAX_PAGINAS; i++) {
            if (memoria_ram[i] != NULL &&
                memoria_ram[i]->processo == p &&
                memoria_ram[i]->num != -1) {
                printf("%d ", memoria_ram[i]->num);
            }
        }
        printf("\n");
    }
}



void criaMemoriaRAM(void)
{
    LOG(">> Reservando memória para Memória RAM...\n");

    memoria_ram = (BasePage**)malloc(sizeof(BasePage*)*MAX_PAGINAS);
    
    forMemoria(i)
    { 
        memoria_ram[i] = (BasePage*)malloc(sizeof(BasePage)); 
        memoria_ram[i]->num = pagina_vazia.num;
        memoria_ram[i]->modo = pagina_vazia.modo;
        memoria_ram[i]->processo = pagina_vazia.processo;
        memoria_ram[i]->extra = pagina_vazia.extra;
    }

    LOG(">> Memória reservada!\n");
}

int checaMemoriaVazia(BasePage *pagina)
{   
    if(
        pagina->num != pagina_vazia.num ||
        pagina->modo != pagina_vazia.modo ||
        pagina->processo != pagina_vazia.processo ||
        pagina->extra != pagina_vazia.extra
    ) return 0;

    return 1;
}

int procuraMemoriaVazia(void)
{
    forMemoria(i)
    {
        if ( checaMemoriaVazia(memoria_ram[i]) ) return i;
    }

    return -1;
}

int procuraPaginaExistente(char *dados)
{
    int page_num; char modo;
    sscanf(dados, "%d %c", &page_num, &modo);

    forMemoria(i)
    {
        if ( memoria_ram[i]->num == page_num && memoria_ram[i]->processo == processo_que_fez_falta )
        {
            memoria_ram[i]->modo = modo;
            return i;
        }
    }

    return -1;
}

void atribuiPagina(char *dados, int idx_memoria, int idx_processo)
{
    int num; char modo;
    sscanf(dados, "%d %c", &num, &modo);

    BasePage *pagina = memoria_ram[idx_memoria];

    pagina->num = num;
    pagina->modo = modo;
    pagina->processo = idx_processo;

    // Apaga o extra antigo se ainda existir
    if (pagina->extra != NULL) {
        free(pagina->extra);
        pagina->extra = NULL;
    }

    // Algoritmo específico
    if (strcmp(algoritmo, "2nCH") == 0) {
        ExtraSecondChance *extra = malloc(sizeof(ExtraSecondChance));
        extra->bit_R = 1;
        pagina->extra = (void*)extra;
    }
    else if (strcmp(algoritmo, "NRU") == 0) {
        ExtraNRU *extra = malloc(sizeof(ExtraNRU));
        extra->bit_R = 1;
        extra->bit_M = (modo == 'W') ? 1 : 0;
        pagina->extra = (void*)extra;
    }
    else if (strcmp(algoritmo, "LRU") == 0) {
        ExtraLRU *extra = malloc(sizeof(ExtraLRU));
        extra->contador = 0;     // começa com 0
        extra->bit_R = 1;        // foi referenciada agora
        pagina->extra = (void*)extra;
    }
    else if (strcmp(algoritmo, "WS") == 0) {
        ExtraWS *extra = malloc(sizeof(ExtraWS));
        extra->bit_R = 1;
        extra->tempo_ultimo_acesso = tempo_global;
        pagina->extra = (void*)extra;
    }

}


void acionaRedistribuicao(char *dados, int idx_memoria)
{
    int idx;

    if (strcmp(algoritmo, "2nCH") == 0)
        idx = select_SecondChance(memoria_ram);
    else if (strcmp(algoritmo, "NRU") == 0)
        idx = select_NRU(memoria_ram);
    else if (strcmp(algoritmo, "LRU") == 0)
        idx = select_LRU(memoria_ram, processo_que_fez_falta);
    else if (strcmp(algoritmo, "WS") == 0)
        idx = select_WorkingSet(memoria_ram, processo_que_fez_falta);

    // substituição genérica
    BasePage *vitima = memoria_ram[idx];
    if (vitima->modo == 'W') {
        printf(">> Página suja escrita para o disco (processo %d)\n", vitima->processo + 1);
    }
    printf(">> Page fault: P%d causou substituição de página do P%d (página %d)\n",
    processo_que_fez_falta + 1, vitima->processo + 1, vitima->num);
    paginas_substituidas++;
    atribuiPagina(dados, idx, processo_que_fez_falta);
}


int checaFim(void)
{
    if (paginas_lidas == QTD_PAGINAS*QTD_PROCESSOS)
    {
        flag_main = 0; flag_gmv = 0;
        printf(">> Todas as páginas foram lidas!\n\n");
        return 1;
    }

    return 0;
}