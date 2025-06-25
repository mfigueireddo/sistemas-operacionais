#ifndef LRU_H
#define LRU_H

#include "utils.h"

typedef struct {
    unsigned char contador; // 8 bits
    int bit_R; // Referenciado (usado no aging)
} ExtraLRU;

int select_LRU(BasePage **memoria, int processo); // LRU é local
void atualizaContadoresLRU(BasePage ***tabela, int processo);

#endif
