#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(void){

    char *ptr;
    int segmento;

    segmento = shmget(8752, sizeof(char)*81 , S_IRUSR); // Cria um segmento de memória
    
    ptr = (char*)shmat(segmento, 0, 0); // Linkando com o segmento de memória

    printf("%s", ptr); // Imprimindo a mensagem

    shmdt(ptr); // Desvincula o segmento de memória
    shmctl(segmento, IPC_RMID, 0); // Remove o segmento de memória de compartilhada

    return 0;
}