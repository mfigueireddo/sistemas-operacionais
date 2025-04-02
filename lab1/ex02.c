#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int valor = 0;

int main(void){

    valor = 1;

    printf("o valor é (antes do fork): %d\n", valor);

    int pid;
    pid= fork();

    if(pid<0){
        printf("deu erro");
    }
    else if(pid==0){
        valor=5;
        printf("o valor é (depois do fork - filho): %d\n", valor);
        exit(1);
    }
    else{
        waitpid(pid, NULL, 0);
        printf("o valor é (depois do fork - pai): %d\n", valor);
    }
    return 0;


}