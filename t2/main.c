#include "utils.h"

// Processos
#include <unistd.h>
#include <sys/wait.h>

// Threads
#include <pthread.h>

// Sinais
#include <signal.h>

// PIPEs
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/types.h>

// Outros
#include <time.h> 

// Processos
void criaArquivosTexto(void);
void criaProcessos(void);
void pausaProcessos(void);
void escalonamento(int pid);

// PIPEs
void criaPipes(void);

// GMV
void criaThreadGMV(void);
extern void* gmv(void*);

// Outros
void imprimeLegenda(void);
int* geraVetorBaguncado(void);
char geraReadWrite(void);
void aguardaEncerramento(void);
void limpaMemoriaMain(void);

// Essenciais
int pids[4];
pthread_t gmv_thread;

// Auxiliares
int flag_main = 1;
int flag_gmv = 1;
int paginas_lidas = 0;

int main(void)
{
    imprimeLegenda();

    LOG("> Iniciando o programa\n");

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
            LOG("\n> Escalonamento: vez do processo %d\n", i+1);

            escalonamento(pids[i]);
            sleep(1);
        }
    }

    // Aguarda o encerramento da thread e dos processos
    aguardaEncerramento();

    // Libera a memória utilizada dinamicamente pelo programa
    limpaMemoriaMain();

    LOG("> Encerrando o programa\n");

    return 0;
}

// Processos
void criaArquivosTexto(void)
{
    LOG("\n> Iniciando criação dos arquivos texto...\n");

    // Abre os arquivos no modo escrita
    FILE *arquivos[4]; char caminho[100];
    forProcessos(i)
    {
        LOG("> Criando o arquivo texto %d...\n", i+1);

        sprintf(caminho, "./arquivos_txt/ordem_processo%d.txt", i+1);
        arquivos[i] = abreArquivoTexto(caminho, 'w');

        LOG("> Arquivo texto %d criado!\n", i+1);
    }

    // Monta os arquivos de cada processo
    int *nums, modo;
    forProcessos(i)
    {
        LOG("> Preenchendo o arquivo texto %d...\n", i+1);

        nums = geraVetorBaguncado(); // Gera um vetor de números

        forPaginas(j)
        {
            modo = geraReadWrite(); // Gera 'W' ou 'R'
            fprintf(arquivos[i], "%d %c\n", nums[j], modo);
        }

        LOG("> Arquivo texto %d preenchido!\n", i+1);
    }

    // Fecha os arquivos
    forProcessos(i){
        LOG("> Fechando o arquivo texto %d...\n", i+1);

        fechaArquivoTexto(arquivos[i]);

        LOG("> Arquivo texto %d fechado!\n", i+1);
    }

    // Libera a memória alocada para nums
    free(nums);

    LOG("> Todos os arquivos texto foram criados!\n");
}

void criaProcessos(void)
{
    LOG("\n> Iniciando criação dos processos...\n");

    forProcessos(i)
    {
        LOG("> Criando o processo %d...\n", i+1);

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

    LOG("> Todos os processos foram criados!\n");
}

void pausaProcessos(void)
{
    LOG("\n> Ordenando a pausa dos processos...\n");

    forProcessos(i)
    {
        LOG("> Pausando o processo %d...\n", i+1);

        kill(pids[i], SIGSTOP); 

        LOG("> Processo %d pausado!\n", i+1);
    }

    LOG("> Todos os processos foram pausados!\n");
}

void escalonamento(int pid)
{
    LOG("> Ordenando que o processo continue\n");
    kill(pid, SIGCONT);

    sleep(1);

    LOG("> Ordenando que o processo pare\n");
    kill(pid, SIGSTOP);
}

// PIPEs
void criaPipes(void)
{
    LOG("\n> Iniciando criação das PIPEs...\n");

    char nome_pipe[50];
    forProcessos(i){

        LOG("> Criando a PIPE %d...\n", i+1);

        sprintf(nome_pipe, "pipes/pipe%d", i+1);
        if ( mkfifo(nome_pipe, READWRITE_MODE) != 0){ fprintf(stderr, "(!) Erro na criação da PIPE %d\n", i+1); exit(1); }

        LOG("> PIPE %d criada!\n", i+1);
    }

    LOG("> Todas as PIPEs foram criadas!\n");
}

// GMV
void criaThreadGMV(void)
{
    LOG("> Criando a thread GMV...");

    if( pthread_create(&gmv_thread, NULL, gmv, NULL) != 0 ){ fprintf(stderr, "(!) Erro na criação da thread GMV\n"); exit(1); } 

    LOG("> Thread GMV criada!\n");
}

// Outros
void imprimeLegenda(void)
{
    printf(">>> Legenda <<<\n");
    printf(">   indica mensagem escrita pela main\n");
    printf(">>  indica mensagem escrita pela GMV\n");
    printf("<>  indica mensagem escrita pelos processos\n");
    printf("(!) indica mensagem de erro\n\n");
}

int* geraVetorBaguncado(void)
{
    // Preenche um vetor com números de 0 à 32 (inclusos)
    int *nums;
    nums = (int*)malloc(sizeof(int)*QTD_PAGINAS);
    forPaginas(i){ nums[i] = i; }

    int new_pos, temp;
    for(int old_pos=QTD_PAGINAS-1; old_pos>0; old_pos--)
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

void aguardaEncerramento(void)
{
    LOG("\n> Aguardando o encerramento da thread...\n");

    // Aguarda o encerreamento da thread GMV
    pthread_join(gmv_thread, NULL);

    LOG("> Thread encerrada!\n");

    // Aguarda o encerramento dos processos
    forProcessos(i)
    { 
        LOG("> Aguardando o encerramento do processo %d...\n", i+1);

        waitpid(pids[i], NULL, 0); 

        LOG("> Processo %d encerrado!\n", i+1);
    }
}

void limpaMemoriaMain(void)
{
    LOG("\n> Iniciando a limpeza da memória da main...\n");

    // Remoção das PIPEs
    char nome_pipe[50];
    forProcessos(i)
    {
        LOG("> Removendo a PIPE %d...\n", i+1);

        sprintf(nome_pipe, "pipes/pipe%d", i+1);
        if ( unlink(nome_pipe) != 0){ fprintf(stderr, "(!) Erro na remoção da PIPE %d\n", i+1); exit(1); }
    
        LOG("> PIPE %d removida!", i+1);
    }

    LOG("> Memória da main limpa!\n");
}