#include <stdio.h>
#include <stdlib.h>

#include <sys/shm.h> // shmat()

#include "../utils.h"

int segmento_memoria;

int main(int argc, char *argv[])
{
    #if MODO_TESTE
        printf("<> Processo 4 criado\n");
    #endif
    
    // Faz a ligação com o segmento de memória
    segmento_memoria = atoi(argv[1]);
    int *memoria = (int*)shmat(segmento_memoria, NULL, 0);
    if (memoria == (void*)-1){ fprintf(stderr, "(!) Erro ao estabelecer ligação entre o processo 4 e o segmento de memória compartilhada.\n"); exit(1); }

    return 0;
}