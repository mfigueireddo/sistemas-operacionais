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

    // Imprime a chave
    printf("Chave a ser buscada: %d\n", chave);

    // Cria uma SHM
    segmento = shmget(IPC_PRIVATE, tamanho * sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    if (segmento == -1) {
        perror("Erro ao alocar memoria compartilhada");
        exit(1);
    }
    
    // Cria um ponteiro para a SHM
    vetor = (int *)shmat(segmento, 0, 0);

    // Cria um número randômico com base no pid
    srand(getpid());

    // Armazena números randômicos na SHM
    for (int i = 0; i < tamanho; i++) {
        vetor[i] = rand() % 10;
    }

    // Imprime os valores gerados
    printf("Vetor: ");
    for (int i = 0; i < tamanho; i++) {
        printf("%d ", vetor[i]);
    }
    printf("\n\n");

    for (int i = 0; i < num_proc; i++) { // Cria 4 processos

        // Processo-filho
        if (fork() == 0) {
            int ini = i * tam_por_proc; // delimita o início da SHM a ser lida pelo processo
            int fim = (i == num_proc - 1) ? tamanho : ini + tam_por_proc; // delimita o fim da SHM a ser lida pelo processo

            for (int j = ini; j < fim; j++) { // Percorre a sua respectiva SHM
                if (vetor[j] == chave) { 
                    printf("Processo %d encontrou a chave na posicao %d\n", getpid(), j);
                }
            }

            shmdt(vetor); // Desanexa a SHM
            exit(0);
        }
    }

    // Processo-pai
    for (int i = 0; i < num_proc; i++) {
        wait(NULL);
    }

    // Desanexa a SHM
    shmdt(vetor);
    // Exclui a SHM
    shmctl(segmento, IPC_RMID, 0);

    return 0;
}
