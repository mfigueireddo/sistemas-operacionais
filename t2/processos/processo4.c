#include "../utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    LOG("<> Processo 4 criado!\n");

    FILE* arquivo;
    arquivo = abreArquivoTexto("arquivos_txt/ordem_processo4.txt", 'r');

    int pipe;
    pipe = conectaPipe("pipes/pipe4", WRITE_MODE);

    sleep(6);

    char buffer[10];
    while( fgets(buffer, sizeof(buffer), arquivo) != NULL )
    {
        printf("<> Processo 4 - %s", buffer);
        write(pipe, buffer, strlen(buffer));
        sleep(3);
    }

    // Limpa a memória
    fechaArquivoTexto(arquivo);
    close(pipe);

    LOG("\n<> Processo 4 encerrado\n");

    return 0;
}