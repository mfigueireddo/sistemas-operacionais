#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h> // O_WRONLY
#include <unistd.h> // read(), close()
#include <string.h> // strcmp()

#define FIFO "fifo_name"
#define OPEN_MODE O_WRONLY

int main(void)
{
    // Abrir a FIFO
    int fifo;
    if ((fifo = open(FIFO, OPEN_MODE)) < 0){ fprintf(stderr, "Erro na abertura da FIFO\n"); return 1; }

    // Loop que lê os dados da FIFO e escreve na tela
    char mensagem[100];
    do
    {
        printf("Insira uma mensagem: ");
        scanf("%s", mensagem);
        write(fifo, mensagem, strlen(mensagem));
    } while(strcmp(mensagem, "stop") != 0);

    // Fecha a FIFO
    close(fifo);

    return 0;
}