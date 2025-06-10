#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // fork(), execl(), kill(), sleep()
#include <pthread.h> // pthread()
#include <time.h> // time()
#include <signal.h> // SIGSTOP, SIGCONT
#include <fcntl.h> // mkfifo()
#include <string.h> // strcpy()

#include <sys/wait.h> // wait()
#include <sys/shm.h> // shmget(), shmctl()
#include <sys/ipc.h> // IPCs
#include <sys/stat.h> // S_IUSR, S_IWUSR

#include "utils.h"

// Estrutura
typedef struct BasePage BasePage;

// Funções do módulo
void criaArquivosTexto(void);
void criaPipes(void);
void criaProcessos(void);
void pausaProcessos(void);
void criaThreadGMV(void);
int* geraVetorBaguncado(void);
char geraReadWrite(void);
void* gmv(void *arg);
int* abrePipes(void);
int checaPipes(int *pipes, char *retorno);
void escalonamento(int pid);
void aguardaEncerramento(void);
void limpaMemoriaMain(void);
void limpaMemoriaGMV(int *pipes, BasePage **memoria_ram);
int checaFim(void);
BasePage** criaMemoriaRAM(void);
int checaMemoriaVazia(BasePage *pagina);
int procuraMemoriaVazia(BasePage** memoria_ram);
void atribuiPagina(char *dados, int idx_processo, BasePage **memoria_ram, int idx_memoria);

// Variáveis globais do módulo
int pids[4];
pthread_t gmv_thread;
int flag_main = 1;
int flag_gmv = 1;
int paginas_lidas = 0;

/* 
Próximos passos
1. Revisar funcionamento geral do programa
2. Criar variáveis globais para evitar passar para as funções
3. Procurar maneira mais eficiente de checar se há algo novo nas PIPEs
4. Mudar limite da criação de arquivos
*/

int main(void)
{
    // Pega a semente do rand()
    srand(time(NULL));

    // Cria os arquivos com as ordens de acesso de cada processo
    criaArquivosTexto();

    // Cria as PIPEs utilizadas para a transmissão de dados
    criaPipes();

    // Executa uma thread que será responsável por gerenciar a memória
    criaThreadGMV(); sleep(1);

    // Cria 4 processos e os interrompe logo em seguida
    criaProcessos(); pausaProcessos();

    sleep(1);

    while(flag_main)
    {
        // Escalonamento Round-Robin
        forProcessos(i)
        {
            #if MODO_TESTE
                printf("\n> Escalonamento: vez do processo %d\n", i+1);
            #endif

            escalonamento(pids[i]);
            sleep(1);
        }
    }

    // Aguarda o encerramento da thread e dos processos
    aguardaEncerramento();

    // Libera a memória utilizada dinamicamente pelo programa
    limpaMemoriaMain();

    return 0;
}

void criaArquivosTexto(void)
{
    #if MODO_TESTE
        printf("\n> Iniciando criaçao dos 4 arquivos texto\n");
    #endif

    // Abre os arquivos no modo escrita
    FILE *arquivos[4]; char caminho[100];
    forProcessos(i)
    {
        sprintf(caminho, "./arquivos_txt/ordem_processo%d.txt", i+1);
        arquivos[i] = abreArquivoTexto(caminho, 'w');
    }

    // Monta os arquivos de cada processo
    int *nums, modo;
    forProcessos(i)
    {
        nums = geraVetorBaguncado(); // Gera um vetor de números

        forPaginas(j)
        {
            modo = geraReadWrite(); // Gera 'W' ou 'R'
            fprintf(arquivos[i], "%d %c\n", nums[j], modo);
        }
    }

    // Fecha os arquivos
    forProcessos(i){ fechaArquivoTexto(arquivos[i]); }

    // Libera a memória alocada para nums
    free(nums);

    #if MODO_TESTE
        printf("> Todos os 4 arquivos texto foram criados\n");
    #endif
}

void criaPipes(void)
{
    #if MODO_TESTE
        printf("\n> Iniciando criação das PIPEs\n");
    #endif

    char nome_pipe[50];
    forProcessos(i){

        #if MODO_TESTE
            printf("> Criando a PIPE %d\n", i+1);
        #endif

        sprintf(nome_pipe, "pipes/pipe%d", i+1);
        if ( mkfifo(nome_pipe, READWRITE_MODE) != 0){ fprintf(stderr, "(!) Erro na criação da PIPE %d\n", i+1); exit(1); }

        #if MODO_TESTE
            printf("> PIPE %d criada\n", i+1);
        #endif
    }

    #if MODO_TESTE
        printf("> Todos as PIPEs foram criadas\n");
    #endif
}

void criaProcessos(void)
{
    #if MODO_TESTE
        printf("\n> Iniciando criação dos processos\n");
    #endif

    forProcessos(i)
    {
        #if MODO_TESTE
            printf("> Criando o processo %d\n", i+1);
        #endif

        pids[i] = fork();
        if (pids[i] == 0) // Filho
        {
            char executavel[100], nome_programa[100];
            sprintf(executavel, "./processos/processo%d", i+1);
            sprintf(nome_programa, "processo%d", i+1);
            execl(executavel, nome_programa, NULL);
            exit(0);
        }

        sleep(1);
    }

    #if MODO_TESTE
        printf("> Todos os processos foram criados\n");
    #endif
}

void pausaProcessos(void)
{
    #if MODO_TESTE
        printf("\n> Ordenando a pausa dos processos\n");
    #endif

    forProcessos(i)
    {
        #if MODO_TESTE
            printf("> Pausando o processo %d\n", i+1);
        #endif 

        kill(pids[i], SIGSTOP); 

        #if MODO_TESTE
            printf("> Processo %d pausado\n", i+1);
        #endif 
    }

    #if MODO_TESTE
        printf("> Todos os processos foram pausados\n");
    #endif
}

void criaThreadGMV(void)
{
    if( pthread_create(&gmv_thread, NULL, gmv, NULL) != 0 ){ fprintf(stderr, "(!) Erro na criação da thread GMV\n"); exit(1); } 
}

int* geraVetorBaguncado(void)
{
    // Preenche um vetor com números de 0 à 64 (inclusos)
    int *nums;
    nums = (int*)malloc(sizeof(int)*64);
    forPaginas(i){ nums[i] = i+1; }

    int new_pos, temp;
    for(int old_pos=63; old_pos>0; old_pos--)
    {
        new_pos = rand() % (old_pos+1);
        temp = nums[new_pos];
        nums[new_pos] = nums[old_pos];
        nums[old_pos] = temp;
    }
    return nums;
}

char geraReadWrite(void)
{
    int aleatorio = rand() % 2;
    return (aleatorio % 2 == 0) ? 'W' : 'R';
}

void* gmv(void *arg)
{
    #if MODO_TESTE
        printf("\n> Gerenciador de Memória Virtual iniciado\n");
    #endif

    int *pipes = abrePipes();
    BasePage** memoria_ram = criaMemoriaRAM();

    char buffer[10]; int idx_memoria, idx_processo;
    while(flag_gmv)
    {
        if ( idx_processo = checaPipes(pipes, buffer) )
        { 
            paginas_lidas++; 
            idx_memoria = procuraMemoriaVazia(memoria_ram);

            if (idx_memoria != -1){ atribuiPagina(buffer, memoria_ram, idx_memoria, idx_processo); }
            else{ acionaRedistribuicao(buffer, memoria_ram, idx_memoria); }
        }
        if ( checaFim() ){ break; }
    }

    limpaMemoriaGMV(pipes, memoria_ram);

    #if MODO_TESTE
        printf("> Gerenciador de Memória Virtual encerrado\n");
    #endif
}

int* abrePipes(void)
{
    int *pipes;
    pipes = (int*)malloc(sizeof(int)*QTD_PROCESSOS);

    char nome_pipe[50];

    forProcessos(i)
    {
        sprintf(nome_pipe, "pipes/pipe%d", i+1);
        pipes[i] = open(nome_pipe, READ_MODE);
        if (pipes[i] < 0){ fprintf(stderr, "(!) Erro na abertura da PIPE %d", i+1); exit(1); }
    }

    return pipes;
}

int checaPipes(int *pipes, char *retorno)
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

void escalonamento(int pid)
{
    kill(pid, SIGCONT);
    sleep(1);
    kill(pid, SIGSTOP);
}

void aguardaEncerramento(void)
{
    #if MODO_TESTE
        printf("\n> Aguardando o encerramento da thread\n");
    #endif

    // Aguarda o encerreamento da thread GMV
    pthread_join(gmv_thread, NULL);

    #if MODO_TESTE
        printf("> Thread encerrada\n");
    #endif

    // Aguarda o encerramento dos processos
    forProcessos(i)
    { 
        #if MODO_TESTE
            printf("> Aguardando o encerramento do processo %d\n", i+1);
        #endif

        waitpid(pids[i], NULL, 0); 

        #if MODO_TESTE
            printf("> Processo %d encerrado\n", i+1);
        #endif
    }
}

void limpaMemoriaMain(void)
{
    #if MODO_TESTE
        printf("\n> Iniciando a limpeza da memória da main\n");
    #endif

    // Remoção das PIPEs
    char nome_pipe[50];
    forProcessos(i){
        sprintf(nome_pipe, "pipes/pipe%d", i+1);
        if ( unlink(nome_pipe) != 0){ fprintf(stderr, "(!) Erro na remoção da PIPE %d\n", i+1); exit(1); }
    }

    #if MODO_TESTE
        printf("> Memória da main limpa\n");
    #endif
}

void limpaMemoriaGMV(int *pipes, BasePage **memoria_ram)
{
    #if MODO_TESTE
        printf("\n> Iniciando a limpeza da memória da GMV\n");
    #endif

    forProcessos(i){ close(pipes[i]); }

    forMemoria(i){ free(memoria_ram[i]); }
    free(memoria_ram);

    #if MODO_TESTE
        printf("> Memória da GMV limpa\n");
    #endif
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

BasePage** criaMemoriaRAM(void)
{
    BasePage** memoria_ram;
    memoria_ram = (BasePage**)malloc(sizeof(BasePage*)*MAX_PAGINAS);
    
    forMemoria(i){ memoria_ram[i] = (BasePage*)malloc(sizeof(BasePage)); }

    return memoria_ram;
}

int checaMemoriaVazia(BasePage *pagina)
{
    BasePage pagina_vazia = {-1, '-', -1, NULL};
    
    if(
        pagina->num != pagina_vazia.num ||
        pagina->modo != pagina_vazia.modo ||
        pagina->processo != pagina_vazia.processo ||
        pagina->extra != pagina_vazia.extra
    ) return 0;

    return 1;
}

int procuraMemoriaVazia(BasePage** memoria_ram)
{
    forMemoria(i)
    {
        if ( checaMemoriaVazia(memoria_ram[i]) ) return i;
    }

    return -1;
}

void atribuiPagina(char *dados, int idx_processo, BasePage **memoria_ram, int idx_memoria)
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