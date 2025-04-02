#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    int tamanho = 20;
    int num_proc = 4;
    int chave = 4;
    int tam_por_proc = tamanho / num_proc;
    int segmento, *vetor;

    segmento = shmget(IPC_PRIVATE, tamanho * sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    if (segmento == -1) {
        perror("Erro ao alocar memoria compartilhada");
        exit(1);
    }
    
    vetor = (int *) shmat(segmento, 0, 0);
    srand(getpid());
    for (int i = 0; i < tamanho; i++) {
        vetor[i] = rand() % 10;
    }

    printf("Vetor:\n");
    for (int i = 0; i < tamanho; i++) {
        printf("%d ", vetor[i]);
    }
    printf("\n\n");

    for (int i = 0; i < num_proc; i++) {
        if (fork() == 0) {
            int ini = i * tam_por_proc;
            int fim = (i == num_proc - 1) ? tamanho : ini + tam_por_proc;

            for (int j = ini; j < fim; j++) {
                if (vetor[j] == chave) {
                    printf("Processo %d encontrou a chave na posicao %d\n", getpid(), j);
                }
            }

            shmdt(vetor); // Desanexa
            exit(0);
        }
    }

    for (int i = 0; i < num_proc; i++) {
        wait(NULL);
    }

    shmdt(vetor);
    shmctl(segmento, IPC_RMID, 0);

    return 0;
}
