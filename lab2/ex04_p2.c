// p2.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define KEY2 5678

struct Data {
    int value;
    int seq;
};

int main() {
    int shm = shmget(KEY2, sizeof(struct Data), 0666);
    if (shm == -1) {
        perror("Erro ao acessar memória compartilhada");
        exit(1);
    }

    struct Data *m2 = (struct Data *) shmat(shm, NULL, 0);
    int seq = 0;
    srand(getpid());

    while (1) {
        sleep(rand() % 5 + 1);
        m2->value = rand() % 10;
        m2->seq = ++seq;
    }

    return 0;
}
