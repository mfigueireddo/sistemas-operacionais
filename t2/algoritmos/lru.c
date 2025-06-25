#include "lru.h"
#include <stdlib.h>

/*
 * Atualiza os contadores das páginas do processo usando a técnica de Aging.
 */
void atualizaContadoresLRU(BasePage ***tabela, int processo) {
    for (int i = 0; i < 32; i++) {
        if (tabela[processo][i]->extra != NULL) {
            ExtraLRU *extra = (ExtraLRU*) tabela[processo][i]->extra;
            extra->contador >>= 1; // desloca bits para a direita

            if (extra->bit_R) {
                extra->contador |= 0x80; // seta o bit mais significativo
                extra->bit_R = 0; // zera o bit de referência
            }
        }
    }
}

/*
 * Seleciona a página com o menor contador do processo (mais "antiga").
 */
int select_LRU(BasePage **memoria, int processo) {
    int idx_menor = -1;
    unsigned char menor_valor = 255;

    for (int i = 0; i < MAX_PAGINAS; i++) {
        if (memoria[i]->processo == processo && memoria[i]->extra != NULL) {
            ExtraLRU *extra = (ExtraLRU*) memoria[i]->extra;

            if (extra->contador < menor_valor) {
                menor_valor = extra->contador;
                idx_menor = i;
            }
        }
    }

    return idx_menor; // retorna o índice da vítima
}
