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
void imprimeRAM(void);
void criaMemoriaRAM(void);
void criaTabelasProcessos(void);
int checaMemoriaVazia(BasePage *pagina);
int procuraMemoriaVazia(void);
int procuraPaginaExistente(char *dados);
void atribuiPagina(char *dados, int idx_memoria, int idx_processo);
void acionaRedistribuicao(char *dados, int idx_memoria);
void imprimePagina(BasePage* pagina);

// Outros
int checaFim(void);
extern char algoritmo[10]; // ex: "2nCH", "NRU", "LRU", "WS"

// Essenciais
int *pipes;
BasePage** memoria_ram;
BasePage*** tabelas_processos;
BasePage pagina_vazia = {-1, '-', -1, NULL};

// Auxiliares
extern int flag_main;
extern int flag_gmv;
extern int paginas_lidas;
extern pthread_mutex_t mutex;
int processo_que_fez_falta = -1;
int paginas_substituidas = 0;
extern int tempo_global;
int paginas_sujas = 0;
extern int penalidade[];

// PIPEs

void* gmv(void *arg)
{
    LOG("\n>> Gerenciador de Memória Virtual iniciado...\n");

    abrePipes();
    criaMemoriaRAM();
    criaTabelasProcessos();

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

            idx_memoria = procuraPaginaExistente(buffer);

            // Página existente
            if ( idx_memoria != -1 )
            {
                printf(">> A página já está presente na memória!\n");
            }

            else
            {
                idx_memoria = procuraMemoriaVazia();

                // Página disponível na mmeória
                if (idx_memoria != -1)
                {
                    printf(">> Espaço de memória vago no índice %d. Acionando registro de página...\n", idx_memoria);

                    atribuiPagina(buffer, idx_memoria, idx_processo); 

                    printf(">> Registro concluído!\n");
                }
                // Redistribuição
                else
                {
                    printf(">> Não há nenhum espaço disponível na memória. Acionando redistruição de páginas...\n");

                    acionaRedistribuicao(buffer, idx_memoria); 

                    printf(">> Redistribuição concluída!\n");
                }
            }

            paginas_lidas++; 

            if (strcmp(algoritmo, "NRU") == 0 && paginas_lidas % 10 == 0) 
            {
                atualizaBitsNRU(tabelas_processos); // zera bits R a cada 10 páginas
            }

            if (strcmp(algoritmo, "LRU") == 0) 
            {
                atualizaContadoresLRU(tabelas_processos, idx_processo);
            }

            if (strcmp(algoritmo, "WS") == 0) 
            {
                atualizaContadoresWS(tabelas_processos, idx_processo);
                tempo_global++;
            }

            printf(">> %d páginas lidas\n", paginas_lidas);
            printf(">> %d páginas sujas\n", paginas_sujas);
            printf(">> %d páginas substituídas\n", paginas_substituidas);

            pthread_mutex_unlock(&mutex);
            LOG(">> Trecho em mutex finalizado\n");
        }
        if ( checaFim() ){ break; }
    }

    LOG("\n>> Todos os processos conseguiram colocar suas páginas na memória. Encerrando GMV...\n");

    imprimeTabelas();

    imprimeRAM();

    printf(">> Total de páginas substituídas: %d\n", paginas_substituidas);
    printf(">> Total de páginas sujas: %d\n", paginas_sujas); 

    limpaMemoriaGMV();
    
    LOG(">> Gerenciador de Memória Virtual encerrado!\n");

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

    LOG("\n>> Liberando as tabelas dos processos...\n");
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (tabelas_processos[i][j] != NULL)
            {
                if (tabelas_processos[i][j]->extra != NULL && tabelas_processos[i][j]->extra != pagina_vazia.extra)
                {
                    free(tabelas_processos[i][j]->extra);
                }
                free(tabelas_processos[i][j]);
            }
        }
        free(tabelas_processos[i]);
    }
    free(tabelas_processos);
    
    LOG("\n>> Tabelas dos processos liberadas...\n");

    LOG("\n>> Liberando a memória alocada para Memória RAM...\n");
    free(memoria_ram);
    LOG("\n>> Memória RAM liberada!\n");

    LOG(">> Memória da GMV limpa!\n");
}


void imprimeTabelas(void) {
    printf("\nTabela de Páginas Final na RAM:\n");

    for (int p = 0; p < QTD_PROCESSOS; p++) {
        printf("P%d\n", p + 1);

        for (int i = 0; i < 32; i++) {
            imprimePagina(tabelas_processos[p][i]);
        }
        printf("\n");
    }
}

void imprimeRAM(void) {
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

void criaTabelasProcessos(void)
{
    LOG(">> Criando tabelas para cada processo...\n");

    tabelas_processos = (BasePage***)malloc(sizeof(BasePage**)*4);
    
    for(int i=0; i<4; i++)
    {
        tabelas_processos[i] = (BasePage**)malloc(sizeof(BasePage*)*32);
        
        for(int j=0; j<32; j++)
        { 
            tabelas_processos[i][j] = (BasePage*)malloc(sizeof(BasePage)); 
            tabelas_processos[i][j]->num = j;
            tabelas_processos[i][j]->modo = pagina_vazia.modo;
            tabelas_processos[i][j]->processo = i;
            tabelas_processos[i][j]->extra = NULL;
        }
    }

    LOG(">> Tabelas criadas!\n");
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

    memoria_ram[idx_memoria] = tabelas_processos[idx_processo][num];

    if ( (tabelas_processos[idx_processo][num]->modo == 'R' ||  tabelas_processos[idx_processo][num]->modo == '-') && modo == 'W') paginas_sujas++;

    tabelas_processos[idx_processo][num]->modo = modo;

    if (tabelas_processos[idx_processo][num]->extra == NULL)
    {
        // Algoritmo específico
        if (strcmp(algoritmo, "2nCH") == 0) {
            ExtraSecondChance *extra = malloc(sizeof(ExtraSecondChance));
            extra->bit_R = 1;
            tabelas_processos[idx_processo][num]->extra = (void*)extra;
        }
        else if (strcmp(algoritmo, "NRU") == 0) {
            ExtraNRU *extra = malloc(sizeof(ExtraNRU));
            extra->bit_R = 1;
            extra->bit_M = (modo == 'W') ? 1 : 0;
            tabelas_processos[idx_processo][num]->extra = (void*)extra;
        }
        else if (strcmp(algoritmo, "LRU") == 0) {
            ExtraLRU *extra = malloc(sizeof(ExtraLRU));
            extra->contador = 0;     // começa com 0
            extra->bit_R = 1;        // foi referenciada agora
            tabelas_processos[idx_processo][num]->extra = (void*)extra;
        }
        else if (strcmp(algoritmo, "WS") == 0) {
            ExtraWS *extra = malloc(sizeof(ExtraWS));
            extra->bit_R = 1;
            extra->tempo_ultimo_acesso = tempo_global;
            tabelas_processos[idx_processo][num]->extra = (void*)extra;
        }
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
        paginas_sujas++;
    }

    printf(">> Page fault: P%d causou substituição de página do P%d (página %d)\n",
    processo_que_fez_falta + 1, vitima->processo + 1, vitima->num);

    paginas_substituidas++;
    penalidade[processo_que_fez_falta] = 1;

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

void imprimePagina(BasePage* pagina)
{
    if (pagina == NULL)
    {
        printf("Página nula\n");
        return;
    }

    printf("Página %d | Modo: %c | Processo: %d",
           pagina->num,
           pagina->modo,
           pagina->processo+1
        );

    if (pagina->extra == NULL)
    {
        printf(" | [Extra] (null)\n");
        return;
    }

    if (strcmp(algoritmo, "2nCH") == 0)
    {
        ExtraSecondChance* extra = (ExtraSecondChance*)pagina->extra;
        printf(" | [2nCH] bit_R: %d\n", extra->bit_R);
    }
    else if (strcmp(algoritmo, "NRU") == 0)
    {
        ExtraNRU* extra = (ExtraNRU*)pagina->extra;
        printf(" | [NRU] bit_R: %d | bit_M: %d\n", extra->bit_R, extra->bit_M);
    }
    else if (strcmp(algoritmo, "LRU") == 0)
    {
        ExtraLRU* extra = (ExtraLRU*)pagina->extra;
        printf("  [LRU] contador: %u | bit_R: %d\n", extra->contador, extra->bit_R);
    }
    else if (strcmp(algoritmo, "WS") == 0)
    {
        ExtraWS* extra = (ExtraWS*)pagina->extra;
        printf(" | [WS] bit_R: %d | tempo_ultimo_acesso: %d\n", extra->bit_R, extra->tempo_ultimo_acesso);
    }
}
