#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

// Chaves da SHM
#define KEY1 1234
#define KEY2 5678

// Estrutura utilizada na SHM
struct Data {
    int value; // valor randômico
    int seq; // flag de mudança de sinal
};

int main(void) {

    // Cria as áreas de memória compartilhadas
    int shm1 = shmget(KEY1, sizeof(struct Data), IPC_CREAT | 0666);
    int shm2 = shmget(KEY2, sizeof(struct Data), IPC_CREAT | 0666);
    if (shm1 == -1 || shm2 == -1) {
        perror("Erro ao criar memória compartilhada");
        exit(1);
    }

    // Cria ponteiros para a SHM
    struct Data *m1 = (struct Data *) shmat(shm1, NULL, 0);
    struct Data *m2 = (struct Data *) shmat(shm2, NULL, 0);

    // Indica que ainda não houveram alterações nas variáveis
    m1->seq = -1;
    m2->seq = -1;

    // Cria um novo processo que executa um outro arquivo
    if (fork() == 0) {
        execl("./ex04_p1", "ex04_p1", NULL);
        perror("Erro ao executar p1");
        exit(1);
    }

    // Cria um novo processo que executa um outro arquivo
    if (fork() == 0) {
        execl("./ex04_p2", "ex04_p2", NULL);
        perror("Erro ao executar p2");
        exit(1);
    }

    // Variáveis auxiliares
    int last_seq1 = -1, last_seq2 = -1;

    // Processo-pai em loop procurando por mudanças nos valores dos filhos
    while (1) {
        if (m1->seq != last_seq1 && m2->seq != last_seq2) { // Se houve alguma mudança nos valores
            printf("Novo cálculo: %d * %d = %d\n", m1->value, m2->value, m1->value * m2->value);
            last_seq1 = m1->seq;
            last_seq2 = m2->seq;
        }
        sleep(1);
    }

    return 0;
}