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

// PIPEs

void* gmv(void *arg)
{
    #if MODO_TESTE
        printf("\n> Gerenciador de Memória Virtual iniciado...\n");
    #endif

    abrePipes();
    criaMemoriaRAM();

    char buffer[10]; int idx_memoria, idx_processo;
    while(flag_gmv)
    {
        // Confere se algum processo enviou uma mensagem
        if ( idx_processo = checaPipes(buffer) )
        { 
            idx_memoria = procuraMemoriaVazia();

            if (idx_memoria != -1)
            {
                #if MODO_TESTE
                    printf("> Espaço de memória vago no índice %d. Acionando registro de página...\n", idx_memoria);
                #endif

                atribuiPagina(buffer, idx_memoria, idx_processo); 

                #if MODO_TESTE
                    printf("> Registro concluído!\n");
                #endif
            }
            else
            {
                #if MODO_TESTE
                    printf("> Não há nenhum espaço disponível na memória. Acionando redistruição de páginas...\n");
                #endif

                acionaRedistribuicao(buffer, idx_memoria); 

                #if MODO_TESTE
                    printf("> Redistribuição concluída!\n");
                #endif
            }

            paginas_lidas++; 
            
            #if MODO_TESTE
                printf("> %d páginas lidas\n", paginas_lidas);
            #endif
        }
        if ( checaFim() ){ break; }
    }

    #if MODO_TESTE
        printf("> Todos os processos conseguiram colocar suas páginas na memória. Encerrando GMV...\n");
    #endif

    limpaMemoriaGMV();

    #if MODO_TESTE
        printf("> Gerenciador de Memória Virtual encerrado!\n");
    #endif
}

void abrePipes(void)
{
    pipes = (int*)malloc(sizeof(int)*QTD_PROCESSOS);

    char nome_pipe[50];

    forProcessos(i)
    {
        sprintf(nome_pipe, "pipes/pipe%d", i+1);
        pipes[i] = open(nome_pipe, READ_MODE);
        if (pipes[i] < 0){ fprintf(stderr, "(!) Erro na abertura da PIPE %d", i+1); exit(1); }
    }
}

int checaPipes(char *retorno)
{
    char buffer[10]; int tam;

    forProcessos(i){
        tam = read(pipes[i], &buffer, sizeof(buffer));
        if (tam > 0)
        {
            buffer[tam] = '\0';
            printf("> GMV - %s\n", buffer);
            strcpy(retorno, buffer);
            return i;
        }
    }

    return 0;
}

// GMV

void limpaMemoriaGMV(void)
{
    #if MODO_TESTE
        printf("\n> Iniciando a limpeza da memória da GMV...\n");
    #endif

    forProcessos(i){ close(pipes[i]); }

    forMemoria(i){ free(memoria_ram[i]); }
    free(memoria_ram);

    #if MODO_TESTE
        printf("> Memória da GMV limpa!\n");
    #endif
}

void criaMemoriaRAM(void)
{
    memoria_ram = (BasePage**)malloc(sizeof(BasePage*)*MAX_PAGINAS);
    
    forMemoria(i)
    { 
        memoria_ram[i] = (BasePage*)malloc(sizeof(BasePage)); 
        memoria_ram[i]->num = pagina_vazia.num;
        memoria_ram[i]->modo = pagina_vazia.modo;
        memoria_ram[i]->processo = pagina_vazia.processo;
        memoria_ram[i]->extra = pagina_vazia.extra;
    }
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
    int num; char modo;
    sscanf(dados, "%d %c", &num, &modo);

    BasePage *pagina;
    pagina = memoria_ram[idx_memoria];

    pagina->num = num;
    pagina->modo = modo;
    pagina->processo = idx_processo;
    pagina->extra = NULL;


}

void acionaRedistribuicao(char *dados, int idx_memoria)
{
}

int checaFim(void)
{
    if (paginas_lidas == QTD_PAGINAS*QTD_PROCESSOS)
    {
        flag_main = 0; flag_gmv = 0;
        return 1;
    }

    return 0;
}