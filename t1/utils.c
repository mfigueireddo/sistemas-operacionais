#include <stdio.h>
#include "aux.h"

typedef struct Aeronave Aeronave;

//Função que calcula o novo valor do eixo X (horizontal) da aeronave com base em sua direção e velocidade
float movimentaX(Aeronave *aeronave){

    //Inicializa novo_x com a posição atual da aeronave
    float novo_x = aeronave->ponto.x;

    //Se a aeronave estiver vindo do leste ('E') e ainda não tiver chegado ao ponto central (0.5):
    // Reduz sua posição X (vai se aproximando do centro)
    // Se passar do centro (menor que 0.5), força o valor a ser exatamente 0.5, o destino
    if (aeronave->direcao == 'E' && aeronave->ponto.x > 0.5){
        novo_x -= aeronave->velocidade;
        if (novo_x < 0.5) novo_x = 0.5;
    }

    //Se a aeronave estiver vindo do oeste ('W') e ainda não tiver chegado ao centro:
    // Aumenta sua posição X
    // Se ultrapassar o centro, força o valor a ser 0.5
    else if(aeronave->direcao == 'W' && aeronave->ponto.x < 0.5){
        novo_x += aeronave->velocidade;
        if (novo_x > 0.5) novo_x = 0.5;
    }

    //Retorna a nova coordenada X calculada da aeronave
    return novo_x;
}


float movimentaY(Aeronave *aeronave){

    //Inicializa novo_y com a posição Y atual da aeronave
    float novo_y = aeronave->ponto.y;

    //Se o Y atual for menor que o ponto central (0.5):
    // Aumenta o valor de Y
    // Se ultrapassar 0.5, limita para 0.5
    if (aeronave->ponto.y < 0.5){
        novo_y += aeronave->velocidade;
        if (novo_y > 0.5) novo_y = 0.5;
    }

    //Se o Y atual for maior que o ponto central (0.5):
    // Diminui o valor de Y
    // Se for menor que 0.5, corrige para 0.5
    else if (aeronave->ponto.y > 0.5){
        novo_y -= aeronave->velocidade;
        if (novo_y < 0.5) novo_y = 0.5;
    }

    //Retorna a nova posição no eixo Y após o movimento.
    return novo_y;

}

//Função que imprime todos os dados relevantes de uma aeronave
void imprimeAeronave(struct Aeronave *aeronave){
    printf("ID %d Coordenadas [%.2f, %.2f] Direção %c Velocidade %.2f Pista preferida %d Status %s Delay %d PID %d\n", aeronave->id, aeronave->ponto.x, aeronave->ponto.y, aeronave->direcao, aeronave->velocidade, aeronave->pista_preferida, stringStatus(aeronave->status), aeronave->delay, aeronave->pid);
}

//Função que traduz o valor numérico do status da aeronave para uma string legível
const char* stringStatus(int status){

    //Cada case retorna a string correspondente ao status definido no enum do aux.h
    // Se for um status desconhecido, retorna "DESCONHECIDO"
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

//Essa função retorna uma pista alternativa com base na atual
// Serve para balancear as aeronaves entre pistas paralelas
//6 <-> 27 (paralelas em uma direção)
// 18 <-> 3 (paralelas na outra direção)
int alteraPista(int pista){
    if (pista == 6) return 27;
    else if (pista == 27) return 6;
    else if (pista == 18) return 3;
    else if (pista == 3) return 18;
    return -1;
}