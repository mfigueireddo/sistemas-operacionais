#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // fork(), execl(), kill(), sleep()
#include <pthread.h> // pthread()
#include <time.h> // time()
#include <signal.h> // SIGSTOP, SIGCONT
#include <fcntl.h> // mkfifo()

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
void escalonamento(int pid);
void aguardaEncerramento(void);
void limpaMemoria(void);

// Variáveis globais do módulo
int pids[4];
pthread_t gmv_thread;
int flag_main = 1;
int flag_gmv = 1;

int main(void)
{
    // Pega a semente do rand()
    srand(time(NULL));

    // Cria os arquivos com as ordens de acesso de cada processo
    criaArquivosTexto();

    // Cria as PIPEs utilizadas para a transmissão de dados
    criaPipes();

    // Cria 4 processos e os interrompe logo em seguida
    criaProcessos(); pausaProcessos();

    // Executa uma thread que será responsável por gerenciar a memória
    criaThreadGMV();

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
        if ( mkfifo(nome_pipe, READWRITEMODE) != 0){ fprintf(stderr, "(!) Erro na criação da PIPE %d\n", i+1); exit(1); }

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

    while(flag_gmv)
    {

    }

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

    // Remoção das PIPEs
    char nome_pipe[50];
    forProcessos(i){
        sprintf(nome_pipe, "pipes/pipe%d", i+1);
        if ( unlink(nome_pipe) != 0){ fprintf(stderr, "(!) Erro na remoção da PIPE %d\n", i+1); exit(1); }
    }

    #if MODO_TESTE
        printf("> Memória limpa\n");
    #endif
}