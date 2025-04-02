// p1.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define KEY1 1234

struct Data {
    int value;
    int seq;
};

int main() {
    int shm = shmget(KEY1, sizeof(struct Data), 0666);
    if (shm == -1) {
        perror("Erro ao acessar memória compartilhada");
        exit(1);
    }

    struct Data *m1 = (struct Data *) shmat(shm, NULL, 0);
    int seq = 0;
    srand(getpid());

    while (1) {
        sleep(rand() % 5 + 1);
        m1->value = rand() % 10;
        m1->seq = ++seq;
    }

    return 0;
}