#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h> // O_WRONLY
#include <unistd.h> // open(), read(), close(), unlink()
#include <sys/wait.h> // wait()

#define FIFO "fifo_name"
#define OPEN_MODE O_RDONLY
#define QTD_PROCESSOS 2

int main(void)
{
    // Criar uma FIFO
    if(mkfifo(FIFO, S_IRUSR | S_IWUSR) != 0){ fprintf(stderr, "Erro na criação da FIFO\n"); }

    // Cria 2 processos-filho
    int pid;
    for(int i=0; i<QTD_PROCESSOS; i++)
    {
        pid = fork();
        if (pid == 0) // Filho
        {
            execl("./ex02-filho", "ex02-filho", NULL);
            return 0;
        }
    }

    // Espera os filhos terminarem
    wait(NULL);

    // Abre a FIFO no modo leitura
    int fifo;
    if ((fifo = open(FIFO, OPEN_MODE)) < 0){ fprintf(stderr, "Erro na abertura da FIFO\n"); return 1; }

    // Lê as mensagens dos processos-filho e as exibe na tela
    char mensagem[100]; int size;
    while((size = read(fifo, &mensagem, sizeof(mensagem))) > 0) 
    {
        mensagem[size] = '\0';
        printf("Lido: %s\n", mensagem);
    } 

    // Fecha a FIFO
    close(fifo);

    // Deleta a FIFO
    if (unlink(FIFO) != 0){ fprintf(stderr, "Erro ao deletar a FIFO"); }

    return 0;
}