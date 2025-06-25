#ifndef NRU_H
#define NRU_H

#include "utils.h"

typedef struct {
    int bit_R; // Referenciado
    int bit_M; // Modificado
} ExtraNRU;

int select_NRU(BasePage **memoria);

void atualizaBitsNRU(BasePage ***tabelas_processos); // (opcional) para limpar bits R periodicamente

#endif
