#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h> // open()

#include <errno.h>
#include <string.h>

#include "utils.h"

typedef struct BasePage BasePage;

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
    if (pipe < 0){ fprintf(stderr, "(!) Erro na abertura da PIPE 1 %s\n", strerror(errno)); exit(1); }
    return pipe;
}