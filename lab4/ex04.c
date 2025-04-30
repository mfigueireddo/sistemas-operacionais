#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void){

    int pid_leitura1, pid_leitura2, pid_escrita, fd[2];

    if (pipe(fd) == -1){ perror("Erro na criação da pipe"); exit(1); }

    // Filho de leitura 1
    pid_leitura1 = fork();
    if (pid_leitura1 == 0){
        char temp[100];
        ssize_t bytes_lidos;
        close(fd[1]);
        while(1){
            bytes_lidos = read(fd[0], temp, sizeof(temp)-1);
            if (bytes_lidos > 0){ temp[bytes_lidos] = '\0'; printf("Filho de leitura 1 leu %s\n", temp); }
            sleep(4);
        }
        exit(0);
    }

    // Filho de leitura 2
    pid_leitura2 = fork();
    if (pid_leitura2 == 0){
        char temp[100];
        ssize_t bytes_lidos;
        close(fd[1]);
        while(1){
            bytes_lidos = read(fd[0], temp, sizeof(temp)-1);
            if (bytes_lidos > 0){ temp[bytes_lidos] = '\0'; printf("Filho de leitura 2 leu %s\n", temp); }
            sleep(4);
        }
        exit(0);
    }

    // Filho de escrita
    pid_escrita = fork();
    if (pid_escrita == 0){
        char mensagem[100] = "Sequência de a's: ";
        close(fd[0]);
        while(1){
            strcat(mensagem, "a");
            write(fd[1], mensagem, sizeof(mensagem));
            printf("\nFilho de escrita escreveu %s\n", mensagem);
            sleep(2);
        }
        exit(0);
    }

    // Pai
    close(fd[0]); close(fd[1]);
    wait(NULL);

    return 0;
}