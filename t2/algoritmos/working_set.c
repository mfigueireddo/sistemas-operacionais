#include "working_set.h"
#include <limits.h>
#include <stdlib.h>

int WS_K = 5;
int tempo_global = 0; // tempo global avança a cada acesso

/*
 * Atualiza os bits R e timestamps das páginas do processo atual
 */
void atualizaContadoresWS(BasePage **tabela, int processo) {
    for (int i = 0; i < 32; i++) {
        if (tabela[processo][i].extra != NULL) {
            ExtraWS *extra = (ExtraWS*)tabela[processo][i].extra;

            if (extra->bit_R) {
                extra->tempo_ultimo_acesso = tempo_global;
                extra->bit_R = 0; // limpa R após atualizar
            }
        }
    }
}


/*
 * Seleciona uma página para substituição com base no Working Set (local)
 */
int select_WorkingSet(BasePage **memoria, int processo) {
    int candidato_idx = -1;
    int mais_antigo = INT_MAX;

    for (int i = 0; i < MAX_PAGINAS; i++) {
        if (memoria[i]->processo == processo && memoria[i]->extra != NULL) {
            ExtraWS *extra = (ExtraWS*)memoria[i]->extra;

            int idade = tempo_global - extra->tempo_ultimo_acesso;

            if (idade >= WS_K) {
                return i; // está fora do Working Set
            }

            if (extra->tempo_ultimo_acesso < mais_antigo) {
                mais_antigo = extra->tempo_ultimo_acesso;
                candidato_idx = i;
            }
        }
    }

    if (candidato_idx != -1)
        return candidato_idx;

    // fallback global
    for (int i = 0; i < MAX_PAGINAS; i++) {
        if (memoria[i]->extra != NULL) return i;
    }

    return 0;
}

