#include <stdio.h>
#include <stdlib.h>

#include <sys/shm.h> // shmat()

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

int* getMemoria(char *segmento_memoria)
{
    int shm;
    shm = atoi(segmento_memoria);
    int *memoria = (int*)shmat(shm, NULL, 0);
    if (memoria == (void*)-1){ fprintf(stderr, "(!) Erro ao estabelecer ligação entre o processo 1 e o segmento de memória compartilhada\n"); exit(1); }
    return memoria;
}