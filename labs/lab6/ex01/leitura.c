#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h> // O_WRONLY
#include <unistd.h> // read(), close(), unlink()
#include <string.h> // strcmp()

#define FIFO "fifo_name"
#define OPEN_MODE O_RDONLY

int main(void)
{
    // Criar uma FIFO
    if(mkfifo(FIFO, S_IRUSR | S_IWUSR) != 0){ fprintf(stderr, "Erro na criação da FIFO\n"); }

    // Abrir a FIFO
    int fifo;
    if ((fifo = open(FIFO, OPEN_MODE)) < 0){ fprintf(stderr, "Erro na abertura da FIFO\n"); return 1; }

    // Loop que lê os dados da FIFO e escreve na tela
    char mensagem[100]; int size;
    while((size = read(fifo, &mensagem, sizeof(mensagem))) > 0) 
    {
        mensagem[size] = '\0';
        if (strcmp(mensagem, "stop") == 0){ break; }
        printf("Lido: %s\n", mensagem);
    }

    // Fecha a FIFO
    close(fifo);

    // Deleta a FIFO
    if (unlink(FIFO) != 0){ fprintf(stderr, "Erro ao deletar a FIFO"); }

    return 0;
}