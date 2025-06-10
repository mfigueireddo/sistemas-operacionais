#include "../utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    #if MODO_TESTE
        printf("<> Processo 1 criado\n");
    #endif

    FILE* arquivo;
    arquivo = abreArquivoTexto("arquivos_txt/ordem_processo1.txt", 'r');

    int pipe;
    pipe = conectaPipe("pipes/pipe1", WRITE_MODE);

    sleep(6);

    char buffer[10];
    while( fgets(buffer, sizeof(buffer), arquivo) != NULL )
    {
        printf("<> Processo 1 - %s", buffer);
        write(pipe, buffer, strlen(buffer));
        sleep(3);
    }

    // Limpa a memória
    fechaArquivoTexto(arquivo);
    close(pipe);

    #if MODO_TESTE
        printf("\n<> Processo 1 encerrado\n");
    #endif

    return 0;
}