#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // sleep()
#include <fcntl.h>
#include <string.h>

#include "../utils.h"

typedef struct BasePage BasePage;

int main(int argc, char *argv[])
{
    #if MODO_TESTE
        printf("<> Processo 4 criado\n");
    #endif

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

    #if MODO_TESTE
        printf("\n<> Processo 4 encerrado\n");
    #endif

    return 0;
}