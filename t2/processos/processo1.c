#include <stdio.h>

#include "../utils.h"

int main(void)
{
    #if MODO_TESTE
        printf("! Processo 1 criado\n");
    #endif
    
    return 0;
}