#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // fork(), execlp(), kill(), sleep()
#include <sys/wait.h> // wait()

#include <pthread.h> // pthread()
#include <time.h> // time()
#include <signal.h> // SIGSTOP, SIGCONT

#include <sys/shm.h> // shmget(), shmctl()
#include <sys/ipc.h> // IPCs
#include <sys/stat.h> // S_IUSR, S_IWUSR

#include "utils.h"

// Estrutura
typedef struct BasePage BasePage;

// Funções do módulo
void criaArquivosTexto(void);
void criaMemoriaCompartilhada(void);
void marcaMemoriaCompartilhada(void);
void criaProcessos(void);
void pausaProcessos(void);
void criaThreadGMV(void);
int* geraVetorBaguncado(void);
char geraReadWrite(void);
void* gmv(void *arg);
void aguardaEncerramento(void);
void limpaMemoria(void);

// Variáveis globais do módulo
int segmento_memoria;
int pids[4];
pthread_t gmv_thread;
int flag_loop = 1;

int main(void)
{
    // Pega a semente do rand()
    srand(time(NULL));

    // Cria os arquivos com as ordens de acesso de cada processo
    criaArquivosTexto();

    // Cria a memória que será compartilhada pelos processos e marca todos os espaços com NULL
    criaMemoriaCompartilhada(); marcaMemoriaCompartilhada();

    // Cria 4 processos e os interrompe logo em seguida
    criaProcessos(); pausaProcessos();

    // Executa uma thread que será responsável por gerenciar a memória
    criaThreadGMV();

    sleep(1);

    while(flag_loop)
    {
        // Escalonamento Round-Robin
        forProcessos(i)
        {
            #if MODO_TESTE
                printf("\n> Escalonamento: vez do processo %d\n", i+1);
            #endif

            escalonamento(pids[i]);
        }
    }

    // Aguarda o encerramento da thread e dos processos
    aguardaEncerramento();

    // Libera a memória utilizada dinamicamente pelo programa
    limpaMemoria();

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

void criaMemoriaCompartilhada(void)
{
    segmento_memoria = shmget(IPC_PRIVATE, sizeof(BasePage)*MAX_PAGINAS, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento_memoria == -1){ fprintf(stderr, "(!) Erro na criação de memória compartilhada\n"); exit(1); }
}

void marcaMemoriaCompartilhada(void)
{
    BasePage* memoria = getMemoria(segmento_memoria);

    forMemoriaCompartilhada(i)
    {
        memoria[i] = pagina_vazia;
    }
}

void criaProcessos(void)
{
    #if MODO_TESTE
        printf("\n> Iniciando criação dos 4 processos\n");
    #endif

    forProcessos(i)
    {
        #if MODO_TESTE
            printf("> Criando o processo %d\n", i+1);
        #endif

        pids[i] = fork();
        if (pids[i] == 0) // Filho
        {
            char executavel[100], nome_programa[100], str_segmento_memoria[100];
            sprintf(executavel, "./processos/processo%d", i+1);
            sprintf(nome_programa, "processo%d", i+1);
            sprintf(str_segmento_memoria, "%d", segmento_memoria);
            execlp(executavel, nome_programa, str_segmento_memoria, NULL);
            exit(0);
        }

        sleep(1);
    }

    #if MODO_TESTE
        printf("> Todos os 4 processos foram criados\n");
    #endif
}

void pausaProcessos(void)
{
    #if MODO_TESTE
        printf("\n> Ordenando a pausa dos 4 processos\n");
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
        printf("> Todos os 4 processos foram pausados\n");
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

    #if MODO_TESTE
        printf("> Gerenciador de Memória Virtual encerrado\n");
    #endif
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

void limpaMemoria(void)
{
    #if MODO_TESTE
        printf("\n> Iniciando a limpeza da memória\n");
    #endif

    // Remoção da área de memória compartilhada
    shmctl(segmento_memoria, IPC_RMID, NULL);

    #if MODO_TESTE
        printf("> Memória limpa\n");
    #endif
}