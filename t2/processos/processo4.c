#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> // sleep()

#include "../utils.h"

typedef struct BasePage BasePage;

int main(int argc, char *argv[])
{
    #if MODO_TESTE
        printf("<> Processo 4 criado\n");
    #endif

    FILE* arquivo;
    arquivo = abreArquivoTexto("arquivos_txt/ordem_processo4.txt", 'r');

    // Faz a conexão com a pipe que o liga ao GMV

    sleep(6);
    char buffer[10];
    
    while( fgets(buffer, sizeof(buffer), arquivo) != NULL )
    {
        printf("<> Processo 4 - %s", buffer);

        sleep(2);
    }

    // Limpa a memória
    fechaArquivoTexto(arquivo);

    #if MODO_TESTE
        printf("\n<> Processo 4 encerrado\n");
    #endif

    return 0;
}