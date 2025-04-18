#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(void){

    char mensagem[81], *ptr;
    int segmento;

    // Recebe uma mensagem do usuário
    printf("Qual a mensagem do dia? ");
    fgets(mensagem, sizeof(mensagem), stdin);

    segmento = shmget(8752, sizeof(mensagem), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR); // Cria um SHM
    
    ptr = (char*)shmat(segmento, 0, 0); // Cria um ponteiro para a SHM

    strcpy(ptr, mensagem); // Salvando a mensagem

    return 0;
}