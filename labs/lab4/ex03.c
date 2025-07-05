#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid1, pid2;

    if (pipe(fd) == -1) {
        perror("Erro ao criar o pipe");
        exit(1);
    }

    // Primeiro filho
    pid1 = fork();
    if (pid1 == -1) {
        perror("Erro no fork do ps");
        exit(2);
    }

    if (pid1 == 0) {
        // Processo filho 1 (ps)
        close(fd[0]); 
        dup2(fd[1], STDOUT_FILENO); 
        close(fd[1]);
        execlp("ps", "ps", NULL); // Executa o comando ps
        perror("Erro ao executar ps");
        exit(3);
    }

    // Segundo filho
    pid2 = fork();
    if (pid2 == -1) {
        perror("Erro no fork do wc");
        exit(4);
    }

    if (pid2 == 0) {
        // Processo filho 2 (wc)
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO); // Redireciona stdin para o pipe
        close(fd[0]); 
        execlp("wc", "wc", NULL); // Executa o comando wc
        perror("Erro ao executar wc");
        exit(5);
    }

    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);

    return 0;
}
