#include <stdio.h>
#include <unistd.h> // fork()
#include <stdlib.h> // exit()
#include <sys/wait.h> // wait()

#include "utils.h"

void cria_processos(void);

int main(void)
{
    // Cria 4 processos
    cria_processos();

    // Cria os arquivos com as ordens de acesso de cada processo

    // Cria a memória que será compartilhada pelos processos

    // Executa uma thread que será responsável por gerenciar a memória

    // Escalonamento Round-Robin

    // Limpa o que for necessário

    return 0;
}

void cria_processos(void)
{
    int pid[4];
    for(int i=0; i<4; i++)
    {
        pid[i] = fork();
        if (pid[i] == 0) // Filho
        {
            char executavel[20], nome_programa[20];
            sprintf(executavel, "./processo%d", i+1);
            sprintf(nome_programa, "processo%d", i+1);
            execl(executavel, nome_programa, NULL);
            exit(0);
        }
    }

    for(int i=0; i<4; i++){ wait(NULL); } // Apenas para testar se os processos foram criados
}