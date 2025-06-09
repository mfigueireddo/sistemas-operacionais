#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // fork()

#include <time.h> // time()

#include <sys/wait.h> // wait()
#include <sys/shm.h> // shmget()
#include <sys/ipc.h> // IPCs
#include <sys/stat.h> // S_IUSR, S_IWUSR

#include "utils.h"

void criaProcessos(void);
void criaArquivosTexto(void);
int* geraVetorBaguncado(void);
char geraReadWrite(void);

int segmento_memoria;

int main(void)
{
    // Pega a semente do rand()
    srand(time(NULL));

    // Cria 4 processos
    criaProcessos();

    // Cria os arquivos com as ordens de acesso de cada processo
    criaArquivosTexto();

    // Cria a memória que será compartilhada pelos processos
    segmento_memoria = shmget(IPC_PRIVATE, (sizeof(int)*QTD_PAGINAS)*QTD_PROCESSOS, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento_memoria == -1){ fprintf(stderr, "Erro na criação de memória compartilhada.\n"); exit(1); }

    // Executa uma thread que será responsável por gerenciar a memória

    // Escalonamento Round-Robin

    // Limpa o que for necessário

    return 0;
}

void criaProcessos(void)
{
    #if MODO_TESTE
        printf("> Iniciando criaçao dos 4 processos\n");
    #endif

    int pid[4];
    forProcessos(i)
    {
        pid[i] = fork();
        if (pid[i] == 0) // Filho
        {
            char executavel[20], nome_programa[20];
            sprintf(executavel, "./processos/processo%d", i+1);
            sprintf(nome_programa, "processo%d", i+1);
            execl(executavel, nome_programa, NULL);
            exit(0);
        }
        sleep(1); // Espera os processos iniciarem
    }

    #if MODO_TESTE
        printf("> Todos os 4 processos foram criados\n");
    #endif
}

void criaArquivosTexto(void)
{
    #if MODO_TESTE
        printf("> Iniciando criaçao dos 4 arquivos texto\n");
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

    #if MODO_TESTE
        printf("> Todos os 4 arquivos texto foram criados\n");
    #endif
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