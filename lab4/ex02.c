#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void){

    int fd_entrada = open("entrada.txt", O_RDONLY);
    if (fd_entrada<0){ perror("Erro na abertura do arquivo de entrada"); exit(1); }
    
    int fd_saida = open("saida.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_saida<0){ perror("Erro na abertura do arquivo de saída"); exit(1); }

    /*
    close(0); // Fechando a entrada padrão stdin
    close(1); // Fechando a entrada padrão stdout
    */

    // Agora a entrada padrão passa a ser nosso arquivo entrada.txt
    if (dup2(fd_entrada, 0)<0){ perror("Erro na duplicação do file descriptor de entrada"); exit(1); }
    close(fd_entrada);
    
    // Agora a saída padrão passa a ser nosso arquivo saida.txt
    if (dup2(fd_saida, 1)<0){ perror("Erro na duplicação do file descriptor de saída"); exit(1); }
    close(fd_saida);

    char temp[100];
    ssize_t bytes_lidos;
    
    while ((bytes_lidos = read(0, temp, sizeof(temp))) > 0) { // Lê entrada.txt linha por linha
        write(1, temp, bytes_lidos); // Escreve no saida.txt linha por linha
    }

    return 0;
}