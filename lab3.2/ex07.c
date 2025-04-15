#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void){

    int pid1, pid2, pid3;

    pid1 = fork();
    // Erro
    if (pid1<0) perror("Erro no fork");
    // Filho 1
    else if(pid1==0){
        while(1){
            printf("Eu sou o filho 1\n");
            sleep(1);
        }
    }

    pid2 = fork();
    // Erro
    if (pid2<0) perror("Erro no fork");
    // Filho 2
    else if(pid2==0){
        while(1){
            printf("Eu sou o filho 2\n");
            sleep(1);
        }
    }

    pid3 = fork();
    // Erro
    if (pid3<0) perror("Erro no fork");
    // Filho 3
    else if(pid3==0){
        while(1){
            printf("Eu sou o filho 3\n");
            sleep(1);
        }
    }

    sleep(1);

    kill(pid1, SIGSTOP);
    kill(pid2, SIGSTOP);
    kill(pid3, SIGSTOP);

    printf("Iniciando o revezamento\n");

    // Pai
    while(1){
        kill(pid1, SIGCONT);
        sleep(1);
        kill(pid1, SIGSTOP);

        kill(pid2, SIGCONT);
        sleep(2);
        kill(pid2, SIGSTOP);

        kill(pid3, SIGCONT);
        sleep(2);
        kill(pid3, SIGSTOP);
    }

    return 0;
}