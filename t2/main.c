#include <stdio.h>
#include <unistd.h> // fork()
#include <stdlib.h> // exit()
#include <sys/wait.h> // wait()

#include "utils.h"

void criaProcessos(void);

int main(void)
{
    // Cria 4 processos
    criaProcessos();

    // Cria os arquivos com as ordens de acesso de cada processo

    // Cria a memória que será compartilhada pelos processos

    // Executa uma thread que será responsável por gerenciar a memória

    // Escalonamento Round-Robin

    // Limpa o que for necessário

    return 0;
}

void criaProcessos(void)
{
    int pid[4];
    for(int i=0; i<4; i++)
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
        printf("Todos os 4 processos foram criados\n");
    #endif
}