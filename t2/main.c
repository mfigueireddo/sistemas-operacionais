#include "utils.h"
#include <string.h>

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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char algoritmo[10];
int total_rodadas;
extern int WS_K;

int main(int argc, char *argv[])
{

     if (argc < 3) {
        printf("Uso: ./main <algoritmo> <rodadas> [<ws_k>]\n");
        return 1;
    }

    strcpy(algoritmo, argv[1]);
    total_rodadas = atoi(argv[2]);

    if (strcmp(algoritmo, "WS") == 0 && argc == 4) {
        WS_K = atoi(argv[3]); // permite ajustar k pela linha de comando
    }

    imprimeLegenda();
    
    printf("--- --- --- --- --- --- --- --- --- + --- --- --- --- --- --- --- --- ---\n\n");

    printf("> Iniciando o programa\n");

    printf("Algoritmo de Substituição: %s\n", algoritmo);
    printf("Rodadas executadas: %d\n", total_rodadas);

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

    LOG("\n--- --- --- --- --- Procedimentos iniciais concluídos --- --- --- --- ---\n");

    sleep(1);

    for (int rodada = 0; rodada < total_rodadas && flag_main; rodada++)
    {
        // Escalonamento Round-Robin
        forProcessos(i)
        {
            LOG("\n> Escalonamento: vez do processo %d\n", i+1);

            escalonamento(pids[i]);
            sleep(1);

            LOG("\n--- --- --- Round do escalonamento concluído\n");

            LOG("\n> Checando permissão para continuar via mutex...\n");
            pthread_mutex_lock(&mutex);
            LOG("> Permissão concedida!\n");
            pthread_mutex_unlock(&mutex);
            LOG("> Trecho em mutex finalizado\n");
        }
    }

    LOG("\n--- --- --- --- --- Programa concluído --- --- --- --- ---\n");

    // Aguarda o encerramento da thread e dos processos
    aguardaEncerramento();

    // Libera a memória utilizada dinamicamente pelo programa
    limpaMemoriaMain();

    printf("> Encerrando o programa\n");

    return 0;
}

// Processos
void criaArquivosTexto(void)
{
    LOG("\n> Iniciando criação dos arquivos texto...\n");

    FILE *arquivos[4] = {NULL};
    char caminho[100];

    /* 1. cria-ou-abre */
    forProcessos(i)
    {
        sprintf(caminho, "./arquivos_txt/ordem_processo%d.txt", i+1);

        if (access(caminho, F_OK) == 0) {
            LOG("> Arquivo texto %d já existe, pulando criação...\n", i+1);
            continue;                     /* NÃO abre em 'w', nem grava */
        }

        LOG("> Criando o arquivo texto %d...\n", i+1);
        arquivos[i] = abreArquivoTexto(caminho, 'w');
        LOG("> Arquivo texto %d criado!\n", i+1);
    }

    /* 2. preenche só quem foi criado agora */
    forProcessos(i)
    {
        if (arquivos[i] == NULL) continue;          /* já existia – pula */

        LOG("> Preenchendo o arquivo texto %d...\n", i+1);

        int *nums = geraVetorBaguncado();
        forPaginas(j)
            fprintf(arquivos[i], "%d %c\n", nums[j], geraReadWrite());

        free(nums);                  /* libera o vetor desse processo  */
        LOG("> Arquivo texto %d preenchido!\n", i+1);
    }

    /* 3. fecha só quem foi aberto em 'w' */
    forProcessos(i)
        if (arquivos[i] != NULL) {
            LOG("> Fechando o arquivo texto %d...\n", i+1);
            fechaArquivoTexto(arquivos[i]);
            LOG("> Arquivo texto %d fechado!\n", i+1);
        }

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
    LOG("> Criando a thread GMV...\n");

    if( pthread_create(&gmv_thread, NULL, gmv, NULL) != 0 ){ fprintf(stderr, "(!) Erro na criação da thread GMV\n"); exit(1); } 

    LOG("> Thread GMV criada!\n");
}

// Outros
void imprimeLegenda(void)
{
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
    int status;
    forProcessos(i)
    { 
        LOG("> Aguardando o encerramento do processo %d...\n", i+1);

        waitpid(pids[i], &status, WUNTRACED); 
        if ( WIFSTOPPED(status) ){ kill(pids[i], SIGCONT); sleep(1); }
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
    
        LOG("> PIPE %d removida!\n", i+1);
    }

    LOG("> Memória da main limpa!\n\n");
}