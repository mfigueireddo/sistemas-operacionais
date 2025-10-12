#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){

    int pid, mypid;

    pid = fork();

    if(pid<0){
        printf("deu erro");
    }
    else if(pid==0){
        mypid = getpid();
        printf("filho - meu pid é: %d\n", mypid);
        exit(0);
    }
    else{
        mypid = getpid();
        printf("pai - meu pid é: %d\n", mypid);
        waitpid(pid, NULL, 0);
    }

    return 0;
}