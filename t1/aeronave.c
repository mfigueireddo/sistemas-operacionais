#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include "aux.h"

// Funções do módulo
void imprimeAeronave(struct Aeronave *aeronave);
void toggle_velocidade(int sig);
void toggle_pista(int sig);
void configurar_inicialmente(struct Aeronave *aeronave, int index);

// Constantes do módulo
struct Aeronave *minha_aeronave = NULL;
float velocidade_original = 0.05;
float velocidade = 0.05;

// Flags do módulo
int velocidade_reduzida = 0;
int redirecionada = 0;

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

    // !!! NÃO ANDA EM Y? !!!
    while (1) {

        // Se a aeronave estiver AGUARDANDO, não anda
        if(minha_aeronave->status != VOANDO){ printf("\n▶️ Aeronave %d não andou porque está esperando permissão\n", minha_aeronave->id); sleep(3); }

        printf("\n▶️ Mudança de posição - Aeronave %d [%f, %f] -> ", minha_aeronave->id, minha_aeronave->ponto.x, minha_aeronave->ponto.y);

        if (minha_aeronave->direcao == 'E') {
            minha_aeronave->ponto.x -= velocidade;
            printf("[%f, %f]\n", minha_aeronave->ponto.x, minha_aeronave->ponto.y);

            if (minha_aeronave->ponto.x <= 0.5) break;
        } 
        
        else {
            minha_aeronave->ponto.x += velocidade;
            printf("[%f, %f]\n", minha_aeronave->ponto.x, minha_aeronave->ponto.y);
            
            if (minha_aeronave->ponto.x >= 0.5) break;
        }

        // Para que a aeronave não avance mais de 1 unidade por vez
        sleep(3);
    }

    printf("\n⏹️ Aeronave %d pousou na pista %d. Encerrando processo ⏹️\n", minha_aeronave->id, minha_aeronave->pista_preferida);
    minha_aeronave->status = FINALIZADO;

    shmdt(memoria);

    return 0;
}

void imprimeAeronave(struct Aeronave *aeronave){
    printf("ID %d Coordenadas [%.2f, %.2f] Direção %c Velocidade %.1f Pista preferida %d Status %d PID %d\n", aeronave->id, aeronave->ponto.x, aeronave->ponto.y, aeronave->direcao, aeronave->velocidade, aeronave->pista_preferida, aeronave->status, aeronave->pid);
}

void toggle_velocidade(int sig) {

    velocidade_reduzida = !velocidade_reduzida;

    if(velocidade_reduzida){
        printf("\n🔁 Aeronave %d aguardando permissão para continuar 🔁\n", minha_aeronave->id);
        printf("\n🔁 Velocidade da aeronave %d alterada - %f -> ", minha_aeronave->id, minha_aeronave->velocidade);

        velocidade = 0;
        minha_aeronave->status = AGUARDANDO;
    }
    else{
        printf("\n🔁 Aeronave %d continuando o trajeto 🔁\n", minha_aeronave->id);
        printf("\n🔁 Velocidade da aeronave %d alterada - %f -> ", minha_aeronave->id, minha_aeronave->velocidade);

        velocidade = velocidade_original;
        minha_aeronave->status = VOANDO;
    }

    printf("%f 🔁\n", minha_aeronave->velocidade);

    // Avião espera um pouco pra andar se ele teve que mudar a velocidade
    sleep(5);
}

void toggle_pista(int sig) {

    redirecionada = !redirecionada;

    printf("\n🔁 Pista da aeronave %d alterada - %d -> ", minha_aeronave->id, minha_aeronave->pista_preferida);

    if (redirecionada) {
        if (minha_aeronave->pista_preferida == 6) minha_aeronave->pista_preferida = 27;
        else if (minha_aeronave->pista_preferida == 27) minha_aeronave->pista_preferida = 6;
        else if (minha_aeronave->pista_preferida == 18) minha_aeronave->pista_preferida = 3;
        else if (minha_aeronave->pista_preferida == 3) minha_aeronave->pista_preferida = 18;
    }

    printf("%d🔁\n", minha_aeronave->pista_preferida);

    // Avião espera um pouco pra andar se ele teve que mudar de pista
    sleep(5);
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
    aeronave->status = VOANDO;

    printf("🟢 Aeronave criada com sucesso 🟢\n");
    imprimeAeronave(aeronave);
}