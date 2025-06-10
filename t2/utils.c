#include "utils.h"

#include <fcntl.h>
#include <string.h>

FILE* abreArquivoTexto(char* caminho, char modo)
{
    FILE* arquivo = fopen(caminho, &modo);
    if (arquivo == NULL){ fprintf(stderr, "(!) Erro na abertura de arquivo.\n"); exit(1); }
    return arquivo;
}

void fechaArquivoTexto(FILE* arquivo)
{
    fclose(arquivo);
}

int conectaPipe(char *caminho, int modo)
{
    int pipe;
    pipe = open(caminho, modo);
    if (pipe < 0){ fprintf(stderr, "(!) Erro na abertura da PIPE 1\n"); exit(1); }
    return pipe;
}