#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){

    int pid;
    pid= fork();

    if(pid<0){
        printf("deu erro");
    }
    else if(pid==0){
        //execl("./aux", "aux", NULL);
        execlp("echo", "echo", "oieee", NULL);
        exit(1);
    }
    else{
        waitpid(pid, NULL, 0);
    }
    return 0;

}