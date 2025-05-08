#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>

// Arquivos header
#include "aux.h"

// Estruturas personalizadas do trabalho
typedef struct Aeronave Aeronave;

// Funções do módulo
void toggle_velocidade(int sig);
void toggle_pista(int sig);
void configurar_inicialmente(struct Aeronave *aeronave, int index);

// Variáveis globais do módulo
static Aeronave *minha_aeronave = NULL;
static float velocidade_original = 0.05;

int main(int argc, char *argv[]) {

    // Confere se os argumento necessários foram passados
    if (argc != 3) { fprintf(stderr, "Uso correto: %s <shm_id> <index>\n", argv[0]); exit(1); }

    // Guarda os argumentos passados
    int shm_id = atoi(argv[1]);
    int index = atoi(argv[2]);

    // Cria um ponteiro para a memória compartilhada
    struct Aeronave *memoria = ( struct Aeronave *) shmat(shm_id, NULL, 0);
    if (memoria == (void *)-1) { perror("Erro no shmat"); exit(1); }
    minha_aeronave = &memoria[index];

    configurar_inicialmente(minha_aeronave, index);

    // Instala os tratadores de sinal
    signal(SIGUSR1, toggle_velocidade);
    signal(SIGUSR2, toggle_pista);

    // Tempo suficiente pra aguardar a liberação do controller
    sleep(6);

    while (1) {

        printf("\nArigato");

        // Inibe qualquer movimento caso a aeronave não tenha permissão do controller
        if(minha_aeronave->status != VOANDO){ 
            sleep(2); continue;
        }

        printf("\n▶️ Mudança de posição - Aeronave %d [%.2f, %.2f] -> ", minha_aeronave->id, minha_aeronave->ponto.x, minha_aeronave->ponto.y);

        minha_aeronave->ponto.x = movimentaX(minha_aeronave);
        minha_aeronave->ponto.y = movimentaY(minha_aeronave);

        printf("[%.2f, %.2f]\n", minha_aeronave->ponto.x, minha_aeronave->ponto.y);

        // Confere se chegou no destino
        if (minha_aeronave->ponto.x == 0.5 && minha_aeronave->ponto.y == 0.5) break;

        // Delay para que a aeronave só avance 1x a cada permissão
        sleep(3);
    }

    printf("\n✅ Aeronave %d pousou na pista %d. Encerrando processo ✅\n", minha_aeronave->id, minha_aeronave->pista_preferida);
    minha_aeronave->status = FINALIZADO;

    // Libera o necessário
    shmdt(memoria);

    return 0;
}

// Handler de velocidade
void toggle_velocidade(int sig) {

    // Se a aeronave estiver voando, ela passa a fica parada
    if(minha_aeronave->status == VOANDO){
        printf("\n🔁 Aeronave %d aguardando permissão para continuar. 🔁\n", minha_aeronave->id);
        minha_aeronave->status = AGUARDANDO;
    }
    // Se a aeronave estiver parada, ela passa a voar
    else{
        printf("\n🔁 Aeronave %d continuando o trajeto 🔁\n", minha_aeronave->id);
        minha_aeronave->status = VOANDO;
    }

    // Tempo para que a aeronave não ande sem que o controller faça um controle de colisão e engavetamento antes
    sleep(1);
}

// Handler de pista
void toggle_pista(int sig) {

    printf("\n🔁 Pista da aeronave %d alterada (%d -> ", minha_aeronave->id, minha_aeronave->pista_preferida);

    minha_aeronave->pista_preferida = alteraPista(minha_aeronave->pista_preferida);

    printf("%d) 🔁\n", minha_aeronave->pista_preferida);
}

void configurar_inicialmente(struct Aeronave *aeronave, int index) {

    printf("\n🔴 Criando aeronave 🔴\n");

    // Garante aleatoridade entre diferentes processos
    srand(time(NULL) + index); 

    aeronave->id = index;
    aeronave->pid = getpid();
    aeronave->direcao = (rand() % 2 == 0) ? 'W' : 'E'; // Sorteia direção

    // Sorteia ponto de entrada (x fixo e y aleatório)
    if (aeronave->direcao == 'W') {
        aeronave->ponto.x = 0.0;
        aeronave->pista_preferida = (rand() % 2 == 0) ? 18 : 3;
    } else {
        aeronave->ponto.x = 1.0;
        aeronave->pista_preferida = (rand() % 2 == 0) ? 6 : 27;
    }
    aeronave->ponto.y = (float)(rand() % 11) / 10.0; // Sorteia ponto Y entre 0 e 1

    aeronave->velocidade = velocidade_original;
    aeronave->delay = rand() % 3; // Sorteia delay entre 0 e 2 (inclusos)
    aeronave->status = DELAY;

    printf("🟢 Aeronave criada com sucesso 🟢\n");
    imprimeAeronave(aeronave);
}