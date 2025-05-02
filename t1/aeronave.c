#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include "aux.h"

struct Aeronave *minha_aeronave = NULL;
float velocidade_original = 0.05;
float velocidade = 0.05;

// Flags
int velocidade_reduzida = 0;
int redirecionada = 0;

void imprimeAeronave(struct Aeronave *aeronave){
    printf("Coordenadas [%f, %f]\nDireção %c\nVelocidade %f\nPista preferida %d\n Status %d\nPID %d\n", 
        aeronave->ponto.x, aeronave->ponto.y, aeronave->direcao, aeronave->velocidade, aeronave->pista_preferida, aeronave->status, aeronave->pid);
}

// Tratador de SIGUSR1 -> alterna velocidade
void toggle_velocidade(int sig) {

    velocidade_reduzida = !velocidade_reduzida;

    if(velocidade_reduzida){
        velocidade = velocidade_original/2;
        minha_aeronave->status = AGUARDANDO;
    }
    else{
        velocidade = velocidade_original;
        minha_aeronave->status = VOANDO;
    }
}

// Tratador de SIGUSR2 -> alterna pista de pouso
void toggle_pista(int sig) {
    redirecionada = !redirecionada;

    if (redirecionada) {
        if (minha_aeronave->pista_preferida == 6) minha_aeronave->pista_preferida = 27;
        else if (minha_aeronave->pista_preferida == 27) minha_aeronave->pista_preferida = 6;
        else if (minha_aeronave->pista_preferida == 18) minha_aeronave->pista_preferida = 3;
        else if (minha_aeronave->pista_preferida == 3) minha_aeronave->pista_preferida = 18;
    }
}

// Inicializa posição, direção, pista preferida e velocidade
void configurar_inicialmente(struct Aeronave *aeronave, int index) {

    srand(time(NULL) + index); // Para aleatoriedade diferente entre processos

    aeronave->pid = getpid();

    // Sorteia direção
    aeronave->direcao = (rand() % 2 == 0) ? 'W' : 'E';

    // Sorteia ponto de entrada (x fixo e y aleatório)
    if (aeronave->direcao == 'W') {
        aeronave->ponto.x = 0.0;
        aeronave->pista_preferida = (rand() % 2 == 0) ? 18 : 3;
    } else {
        aeronave->ponto.x = 1.0;
        aeronave->pista_preferida = (rand() % 2 == 0) ? 6 : 27;
    }

    aeronave->ponto.y = (float)(rand() % 100) / 100.0; // 0.00 a 0.99
    aeronave->velocidade = velocidade_original;
    aeronave->status = VOANDO;

    imprimeAeronave(aeronave);
}

int main(int argc, char *argv[]) {

    printf("Teste");
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <shm_id> <index>\n", argv[0]);
        exit(1);
    }

    int shm_id = atoi(argv[1]);
    int index = atoi(argv[2]);

    struct Aeronave *memoria = ( struct Aeronave *) shmat(shm_id, NULL, 0);
    if (memoria == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    minha_aeronave = &memoria[index];

    configurar_inicialmente(minha_aeronave, index);

    // Instala tratadores de sinal
    signal(SIGUSR1, toggle_velocidade);
    signal(SIGUSR2, toggle_pista);

    printf("[Aeronave %c - PID %d] Iniciou na direção %c. Pista preferida: %d\n",
           'A' + index, getpid(), minha_aeronave->direcao, minha_aeronave->pista_preferida);

    // Loop principal de voo
    while (1) {
        if (minha_aeronave->direcao == 'E') {
            minha_aeronave->ponto.x -= velocidade;
            if (minha_aeronave->ponto.x <= 0.5) break;
        } else {
            minha_aeronave->ponto.x += velocidade;
            if (minha_aeronave->ponto.x >= 0.5) break;
        }
        sleep(1);
    }

    printf("[Aeronave %c - PID %d] Pousou na pista %d. Encerrando processo.\n",
           'A' + index, getpid(), minha_aeronave->pista_preferida);

    // !!! ARRUMAR !!!
    minha_aeronave->status = 2; // Marca como finalizada
    shmdt(memoria);
    return 0;
}
