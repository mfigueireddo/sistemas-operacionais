#ifndef WORKING_SET_H
#define WORKING_SET_H

#include "utils.h"

extern int WS_K; // Tamanho da janela de tempo

typedef struct {
    int bit_R;
    int tempo_ultimo_acesso; // novo campo: quando foi referenciada pela última vez
} ExtraWS;

extern int tempo_global; // tempo absoluto

int select_WorkingSet(BasePage **memoria, int processo);
void atualizaContadoresWS(BasePage **memoria, int processo);

#endif
