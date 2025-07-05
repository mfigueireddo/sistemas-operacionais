#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(void){

    char *ptr;
    int segmento;

    segmento = shmget(8752, sizeof(char)*81 , S_IRUSR); // Lê o SHM
    
    ptr = (char*)shmat(segmento, 0, 0); // Cria um ponteiro para a SHM

    printf("%s", ptr); // Imprimindo a mensagem do dia

    shmdt(ptr); // Desvincula a SHM
    shmctl(segmento, IPC_RMID, 0); // Exclui a SHM

    return 0;
}