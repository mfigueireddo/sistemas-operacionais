#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // sleep()

#include "../utils.h"

int main(int argc, char *argv[])
{
    #if MODO_TESTE
        printf("<> Processo 3 criado\n");
    #endif

    // Faz a ligação com o segmento de memória
    int *memoria = getMemoria(argv[1]);

    FILE* arquivo;
    arquivo = abreArquivoTexto("arquivos_txt/ordem_processo3.txt", 'r');

    sleep(6);
    char buffer[10];
    while( fgets(buffer, sizeof(buffer), arquivo) != NULL )
    {
        printf("<> Processo 3 - %s", buffer);
        sleep(2);
    }

    #if MODO_TESTE
        printf("\n<> Processo 3 encerrado\n");
    #endif

    // Limpa a memória
    fechaArquivoTexto(arquivo);

    return 0;
}