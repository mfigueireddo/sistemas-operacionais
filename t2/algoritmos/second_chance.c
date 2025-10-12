#include "second_chance.h"

static int ponteiro_SC = 0;

int select_SecondChance(BasePage **memoria_ram)
{
    int tentativas = 0;

    while (tentativas < MAX_PAGINAS)
    {
        BasePage *pagina = memoria_ram[ponteiro_SC];
        ExtraSecondChance *extra = (ExtraSecondChance *)pagina->extra;

        if (extra == NULL || extra->bit_R == 0) {
            int escolhido = ponteiro_SC;
            ponteiro_SC = (ponteiro_SC + 1) % MAX_PAGINAS;
            return escolhido;
        } else {
            extra->bit_R = 0;
            ponteiro_SC = (ponteiro_SC + 1) % MAX_PAGINAS;
            tentativas++;
        }
    }

    return ponteiro_SC;
}
