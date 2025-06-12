#include "utils.h"

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
void criaMemoriaRAM(void);
int checaMemoriaVazia(BasePage *pagina);
int procuraMemoriaVazia(void);
void atribuiPagina(char *dados, int idx_memoria, int idx_processo);
void acionaRedistribuicao(char *dados, int idx_memoria);

// Outros
int checaFim(void);

// Essenciais
int *pipes;
BasePage** memoria_ram;
BasePage pagina_vazia = {-1, '-', -1, NULL};

// Auxiliares
extern int flag_main;
extern int flag_gmv;
extern int paginas_lidas;
extern pthread_mutex_t mutex;

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
                printf(">> Não há nenhum espaço disponível na memória. Acionando redistruição de páginas...\n");

                acionaRedistribuicao(buffer, idx_memoria); 

                printf(">> Redistribuição concluída!\n");
            }

            paginas_lidas++; 
            
            LOG(">> %d páginas lidas\n", paginas_lidas);

            pthread_mutex_unlock(&mutex);
            LOG(">> Trecho em mutex finalizado\n");
        }
        if ( checaFim() ){ break; }
    }

    LOG("\n>> Todos os processos conseguiram colocar suas páginas na memória. Encerrando GMV...\n");

    limpaMemoriaGMV();

    LOG(">> Gerenciador de Memória Virtual encerrado!\n");
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
    forMemoria(i){ free(memoria_ram[i]); }
    free(memoria_ram);
    LOG("\n>> Memória liberada!\n");

    LOG(">> Memória da GMV limpa!\n");
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

void atribuiPagina(char *dados, int idx_memoria, int idx_processo)
{
    LOG(">> Criando página na Memória RAM...\n");

    int num; char modo;
    sscanf(dados, "%d %c", &num, &modo);

    BasePage *pagina;
    pagina = memoria_ram[idx_memoria];

    pagina->num = num;
    pagina->modo = modo;
    pagina->processo = idx_processo;
    pagina->extra = NULL;

    LOG(">> Página criada!\n");
}

void acionaRedistribuicao(char *dados, int idx_memoria)
{
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