#include "nru.h"
#include <stdlib.h>
#include <time.h>

#define CLASSES 4

/*
 * Função auxiliar opcional: usada para resetar o bit R de tempos em tempos.
 */
void atualizaBitsNRU(BasePage ***tabelas_processos) {
    for(int p=0; p<4; p++)
    {
        for (int i = 0; i < 32; i++) {
            if (tabelas_processos[p][i]->extra != NULL) {
                ExtraNRU *extra = (ExtraNRU*)tabelas_processos[p][i]->extra;
                extra->bit_R = 0;
            }
        }
    }
}

/*
 * Função principal: implementa o algoritmo de substituição NRU.
 */
int select_NRU(BasePage **memoria) {
    int melhores_indices[CLASSES] = {-1, -1, -1, -1}; // guarda o primeiro índice encontrado para cada classe

    for (int i = 0; i < MAX_PAGINAS; i++) {
        if (memoria[i]->extra != NULL) {
            ExtraNRU *extra = (ExtraNRU*)memoria[i]->extra;

            int R = extra->bit_R;
            int M = (memoria[i]->modo == 'W') ? 1 : 0;

            int classe = 2 * R + M; // calcula a classe (0 a 3)

            // se for o primeiro da classe, salva o índice
            if (melhores_indices[classe] == -1)
                melhores_indices[classe] = i;
        }
    }

    // procura a melhor classe disponível (menor número)
    for (int c = 0; c < CLASSES; c++) {
        if (melhores_indices[c] != -1) {
            return melhores_indices[c];
        }
    }

    // fallback de segurança
    return 0;
}
