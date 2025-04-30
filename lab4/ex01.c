#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void){

    int pid, fd[2];

    char mensagem[] = "Mensagem escrita pelo filho";
    char mensagem_copia[sizeof mensagem];

    pipe(fd);

    pid = fork();
    // Erro
    if (pid<0){
        perror("Erro no fork");
        exit(1);
    }
    // Filho
    else if (pid == 0){
        write(fd[1], mensagem, strlen(mensagem)+1);
        printf("Filho escreveu: %s\n", mensagem);
        close(fd[0]); close(fd[1]);
        exit(0);
    }

    wait(NULL);
    
    read(fd[0], mensagem_copia, sizeof mensagem_copia);
    printf("Pai leu: %s\n", mensagem_copia);

    close(fd[0]); close(fd[1]);

    return 0;
}