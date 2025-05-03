#include <stdio.h>
#include "aux.h"

typedef struct Aeronave Aeronave;

float movimentaX(Aeronave *aeronave){

    float novo_x = aeronave->ponto.x;

    if (aeronave->direcao == 'E' && aeronave->ponto.x > 0.5){
        novo_x -= aeronave->velocidade;
        if (novo_x < 0.5) novo_x = 0.5;
    }

    else if(aeronave->direcao == 'W' && aeronave->ponto.x < 0.5){
        novo_x += aeronave->velocidade;
        if (novo_x > 0.5) novo_x = 0.5;
    }

    return novo_x;
}

float movimentaY(Aeronave *aeronave){

    float novo_y = aeronave->ponto.y;

    if (aeronave->ponto.y < 0.5){
        novo_y += aeronave->velocidade;
        if (novo_y > 0.5) novo_y = 0.5;
    }
    else if (aeronave->ponto.y > 0.5){
        novo_y -= aeronave->velocidade;
        if (novo_y < 0.5) novo_y = 0.5;
    }

    return novo_y;

}

void imprimeAeronave(struct Aeronave *aeronave){
    printf("ID %d Coordenadas [%.2f, %.2f] Direção %c Velocidade %.1f Pista preferida %d Status %s Delay %d PID %d\n", aeronave->id, aeronave->ponto.x, aeronave->ponto.y, aeronave->direcao, aeronave->velocidade, aeronave->pista_preferida, stringStatus(aeronave->status), aeronave->delay, aeronave->pid);
}

const char* stringStatus(int status){

    switch (status) {
        case VOANDO:
            return "VOANDO";
        case AGUARDANDO:
            return "AGUARDANDO";
        case FINALIZADO:
            return "FINALIZADO";
        case REMETIDA:
            return "REMETIDA";
        case DELAY:
            return "DELAY";
        default:
            return "DESCONHECIDO";
    }
}