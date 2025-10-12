#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // processos
#include <signal.h> // sinais

int main(void){

    int pid1, pid2;
    
    pid1 = fork();
    if (pid1 < 0){
        perror("Erro no fork");
        exit(1);
    }
    // Filho 1
    else if(pid1 == 0){
        while(1){
            printf("Executando o filho 1...\n");
            sleep(1);
        }
    } 

    pid2 = fork();
    if (pid2 < 0){
        perror("Erro no fork");
        exit(1);
    }
    // Filho 2
    else if(pid2 == 0){
        while(1){
            printf("Executando o filho 2...\n");
            sleep(1);
        }
    }

    // Pai
    for(int i=0; i<10; i++){
        printf("Fazendo pela %d vez\n", i+1);

        // Filho 1
        printf("Pausando o filho 1 e continuando o filho 2\n");
        kill(pid1, SIGSTOP);
        kill(pid2, SIGCONT);
        sleep(1);

        // Filho 2
        printf("Pausando o filho 2 e continuando o filho 1\n");
        kill(pid2, SIGSTOP);
        kill(pid1, SIGCONT);
        sleep(1);
    }
    
    // Filho 1
    printf("(!) Matando o filho 1\n");
    kill(pid1, SIGKILL);
    sleep(1);

    // Filho 2
    printf("(!) Matando o filho 2\n");
    kill(pid2, SIGKILL);
    sleep(1);

    return 0;
}