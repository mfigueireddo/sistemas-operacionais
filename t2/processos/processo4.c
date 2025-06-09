#include <stdio.h>
#include <stdlib.h>

#include <sys/shm.h> // shmat()

#include "../utils.h"

// Variáveis globais do módulo
int segmento_memoria;

int main(int argc, char *argv[])
{
    #if MODO_TESTE
        printf("\n<> Processo 4 criado\n");
    #endif
    
    // Faz a ligação com o segmento de memória
    segmento_memoria = atoi(argv[1]);
    int *memoria = (int*)shmat(segmento_memoria, NULL, 0);
    if (memoria == (void*)-1){ fprintf(stderr, "(!) Erro ao estabelecer ligação entre o processo 4 e o segmento de memória compartilhada\n"); exit(1); }
   
    #if MODO_TESTE
        printf("\n<> Processo 4 encerrado\n");
    #endif

    return 0;
}