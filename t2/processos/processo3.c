#include "../utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    LOG("<> Processo 3 criado!\n");

    FILE* arquivo;
    arquivo = abreArquivoTexto("arquivos_txt/ordem_processo3.txt", 'r');

    int pipe;
    pipe = conectaPipe("pipes/pipe3", WRITE_MODE);

    sleep(6);

    char buffer[10];
    while( fgets(buffer, sizeof(buffer), arquivo) != NULL )
    {
        LOG("\n<> Processo 3 - %s", buffer);
        write(pipe, buffer, strlen(buffer));
        LOG("<> Escrita na PIPE concluída\n\n");
        sleep(3);
    }

    // Limpa a memória
    fechaArquivoTexto(arquivo);
    close(pipe);

    LOG("<> Processo 3 encerrado\n");

    return 0;
}