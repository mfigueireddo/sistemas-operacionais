#include <stdio.h>
#include <unistd.h> // fork()
#include <stdlib.h> // exit()
#include <sys/wait.h> // wait()

#include "utils.h"

#define forProcessos(i) for(int i=0; i<4; i++)

void criaProcessos(void);
void criaArquivosTexto(void);

int main(void)
{
    // Cria 4 processos
    criaProcessos();

    // Cria os arquivos com as ordens de acesso de cada processo
    criaArquivosTexto();

    // Cria a memória que será compartilhada pelos processos

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

    // Escreve nos arquivos
    forProcessos(i)
    {
        fprintf(arquivos[i], "Teste");
    }

    // Fecha os arquivos
    forProcessos(i){ fechaArquivoTexto(arquivos[i]); }

    #if MODO_TESTE
        printf("> Todos os 4 arquivos texto foram criados\n");
    #endif
}