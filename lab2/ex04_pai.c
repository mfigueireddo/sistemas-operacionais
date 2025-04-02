// pai.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define KEY1 1234
#define KEY2 5678

struct Data {
    int value;
    int seq;
};

int main() {
    int shm1 = shmget(KEY1, sizeof(struct Data), IPC_CREAT | 0666);
    int shm2 = shmget(KEY2, sizeof(struct Data), IPC_CREAT | 0666);

    if (shm1 == -1 || shm2 == -1) {
        perror("Erro ao criar memória compartilhada");
        exit(1);
    }

    struct Data *m1 = (struct Data *) shmat(shm1, NULL, 0);
    struct Data *m2 = (struct Data *) shmat(shm2, NULL, 0);

    m1->seq = -1;
    m2->seq = -1;

    if (fork() == 0) {
        execl("./ex04_p1", "ex04_p1", NULL);
        perror("Erro ao executar p1");
        exit(1);
    }

    if (fork() == 0) {
        execl("./ex04_p2", "ex04_p2", NULL);
        perror("Erro ao executar p2");
        exit(1);
    }

    int last_seq1 = -1, last_seq2 = -1;
    while (1) {
        if (m1->seq != last_seq1 && m2->seq != last_seq2) {
            printf("Novo calculo: %d * %d = %d\n", m1->value, m2->value, m1->value * m2->value);
            last_seq1 = m1->seq;
            last_seq2 = m2->seq;
        }
        sleep(1);
    }

    return 0;
}



