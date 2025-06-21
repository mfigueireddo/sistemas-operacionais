#ifndef SECOND_CHANCE_H
#define SECOND_CHANCE_H

#include "utils.h"

// Struct usada em BasePage->extra
typedef struct {
    int bit_R;
} ExtraSecondChance;

// Função pública usada pelo GMV
int select_SecondChance(BasePage **memoria_ram);

#endif
