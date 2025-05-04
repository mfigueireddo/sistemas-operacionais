#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include "aux.h"

// Funções do módulo
void toggle_velocidade(int sig);
void toggle_pista(int sig);
void configurar_inicialmente(struct Aeronave *aeronave, int index);

// 
typedef struct Aeronave Aeronave;

// Constantes do módulo
struct Aeronave *minha_aeronave = NULL;
float velocidade_original = 0.05;

// int flag_velocidade = 0;

int main(int argc, char *argv[]) {

    // Confere se os argumento necessários foram passados
    if (argc != 3) { fprintf(stderr, "Uso correto: %s <shm_id> <index>\n", argv[0]); exit(1); }

    // Guarda os argumentos passados
    int shm_id = atoi(argv[1]);
    int index = atoi(argv[2]);

    // Cria um ponteiro para a memória compartilhada
    struct Aeronave *memoria = ( struct Aeronave *) shmat(shm_id, NULL, 0);
    if (memoria == (void *)-1) { perror("Erro no shmat"); exit(1); }

    // Cria um ponteiro para o espaço reservado para a aeronave
    minha_aeronave = &memoria[index];

    configurar_inicialmente(minha_aeronave, index);

    // Instala tratadores de sinal
    signal(SIGUSR1, toggle_velocidade);
    signal(SIGUSR2, toggle_pista);

    // Para que a aeronave não se mova sem permissão
    sleep(6);

    while (1) {

        // Se a aeronave estiver AGUARDANDO, não anda
        if(minha_aeronave->status != VOANDO){ 
            // Evita que a aeronave ande depois de ter permissão pra voar mas antes que se confira a possibilidade de colisão
            // MUDEI AQUI AGORA
            /*
            if (flag_velocidade){
                minha_aeronave->status = VOANDO;
                flag_velocidade = !flag_velocidade;
            }
            */
            // printf("\n\n!!! TENTOU VOAR !!!\n\n");
            sleep(2); continue;
        }

        printf("\n▶️ Mudança de posição - Aeronave %d [%.2f, %.2f] -> ", minha_aeronave->id, minha_aeronave->ponto.x, minha_aeronave->ponto.y);

        minha_aeronave->ponto.x = movimentaX(minha_aeronave);
        minha_aeronave->ponto.y = movimentaY(minha_aeronave);

        printf("[%.2f, %.2f]\n", minha_aeronave->ponto.x, minha_aeronave->ponto.y);

        // Confere se chegou no destino
        if (minha_aeronave->ponto.x == 0.5 && minha_aeronave->ponto.y == 0.5) break;

        // Para que a aeronave não avance mais de 1 unidade por vez
        sleep(3);
    }

    printf("\n✅ Aeronave %d pousou na pista %d. Encerrando processo ✅\n", minha_aeronave->id, minha_aeronave->pista_preferida);
    minha_aeronave->status = FINALIZADO;

    shmdt(memoria);

    return 0;
}

void toggle_velocidade(int sig) {

    if(minha_aeronave->status == VOANDO){
        printf("\n🔁 Aeronave %d aguardando permissão para continuar. 🔁\n", minha_aeronave->id);
        minha_aeronave->status = AGUARDANDO;
    }
    else{
        printf("\n🔁 Aeronave %d continuando o trajeto 🔁\n", minha_aeronave->id);

        // MUDEI AQUI AGORA
        minha_aeronave->status = VOANDO;
        //flag_velocidade = !flag_velocidade;
    }

    // MUDEI AQUI AGORA
    sleep(1);
}

void toggle_pista(int sig) {

    printf("\n🔁 Pista da aeronave %d alterada - %d -> ", minha_aeronave->id, minha_aeronave->pista_preferida);

    minha_aeronave->pista_preferida = alteraPista(minha_aeronave->pista_preferida);

    printf("%d 🔁\n", minha_aeronave->pista_preferida);
}

void configurar_inicialmente(struct Aeronave *aeronave, int index) {

    printf("\n🔴 Criando aeronave 🔴\n");

    // Para aleatoriedade diferente entre processos
    srand(time(NULL) + index); 

    aeronave->id = index;

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

    aeronave->ponto.y = (float)(rand() % 11) / 10.0;
    aeronave->velocidade = velocidade_original;
    aeronave->delay = rand() % 3;
    aeronave->status = DELAY;

    printf("🟢 Aeronave criada com sucesso 🟢\n");
    imprimeAeronave(aeronave);
}