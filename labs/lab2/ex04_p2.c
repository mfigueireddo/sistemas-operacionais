#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

// Chave da SHM
#define KEY2 5678

// Estrutura utilizada na SHM
struct Data {
    int value; // valor randômico
    int seq; // flag de mudança de sinal
};

int main(void) {

    // Acessa uma SHM criada previamente
    int shm = shmget(KEY2, sizeof(struct Data), 0666);
    if (shm == -1) {
        perror("Erro ao acessar memória compartilhada");
        exit(1);
    }

    // Cria ponteiro para a SHM
    struct Data *m2 = (struct Data *) shmat(shm, NULL, 0);
    
    // Armazena o número de sequência
    int seq = 0;

    // Cria um número randômico com base no PID
    srand(getpid());

    // Processo-filho
    while (1) {
        sleep(rand() % 5 + 1);
        m2->value = rand() % 10; // escreve um valor na SHM
        m2->seq = ++seq;
    }

    return 0;
}
