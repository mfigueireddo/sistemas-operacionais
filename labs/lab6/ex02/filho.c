#include <stdio.h>
#include <sys/stat.h> // FIFO
#include <fcntl.h> // O_WRONLY
#include <unistd.h> // open(), write(), close()
#include <string.h> // strlen()

#define FIFO "fifo_name"
#define OPEN_MODE O_WRONLY
#define QTD_PROCESSOS 2

int main(void)
{
    // Abre a FIFO no modo escrita
    int fifo;
    if ((fifo = open(FIFO, OPEN_MODE)) < 0){ fprintf(stderr, "Erro na abertura da FIFO\n"); return 1; }

    // Lê as mensagens dos processos-filho e as exibe na tela
    char mensagem[100];
    sprintf(mensagem, "Mensagem do filho %d", getpid());
    write(fifo, mensagem, strlen(mensagem));

    // Fecha a FIFO
    close(fifo);

    return 0;
}