#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

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