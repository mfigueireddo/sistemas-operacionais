#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork()
#include <sys/ipc.h> // IPC_CREAT, IPC_EXCL, IPC_PRIVATE
#include <sys/shm.h> // shmget, shmat, shmdt
#include <sys/stat.h> // S_IUSR, S_IWUSR
#include <sys/wait.h> // waitpid
#include <signal.h> // SIGSTOP, SIGCONT, SIGUSR1, SIGKILL, ...
#include <math.h> // sqrt
#include <time.h> // time()

#include <errno.h>

// Estruturas personalizadas do trabalho
#include "aux.h"
typedef struct Aeronave Aeronave;
typedef struct Pista Pista;
typedef struct Ponto Ponto;

// Macros do módulo
#ifndef QTD_AERONAVES
#define QTD_AERONAVES 5
#endif

#define QTD_PISTAS 4

// Funções do módulo
int buscaIndicePista(int num_pista);
void calculaPrioridade(Aeronave *aeronaves, int *array_indices);
void criaAeronaves(int *segmento_memoria, int *pids);
void controlePistas(Aeronave *aeronaves, int *pids);
int controleColisao(Aeronave *aeronaves, int i, int *pids);
int controleEngavetamento(Aeronave *aeronaves, int *pids);
int verificaEntrada(Aeronave *aeronaves, int i, int *pids);

// Variáveis globais do módulo
Pista pistas[QTD_PISTAS] = { {3, 0}, {6, 0}, {18, 0}, {27, 0} };
int processos_finalizados = 0;
int bloqueados;
int indices_ordenados[QTD_AERONAVES];

int main(void){

    // Criando segmento de memória compartilhando
    int segmento_memoria = shmget(IPC_PRIVATE, sizeof(Aeronave)*QTD_AERONAVES, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento_memoria == -1){ perror("Erro na criação de memória compartilhada"); return 1; }

    // Ponteiro para o segmento de memória
    Aeronave *aeronaves = (Aeronave*)shmat(segmento_memoria, 0, 0);
    if (aeronaves == (void *)-1) { perror("Erro no shmat"); exit(1); }

    // Criando múltiplos processos
    int pids[5];
    criaAeronaves(&segmento_memoria, pids);

    // Começa pausando todas as aeronaves
    printf("\n⚠️ Ordenando a pausa de todas as aeronaves ⚠️\n");
    for(int i=0; i<QTD_AERONAVES; i++) kill(pids[i], SIGSTOP); 

    // Confere se não há aeronaves com a mesma pista de destino
    controlePistas(aeronaves, pids);

    // Ordem das aeronaves mais próximas ao destino
    for(int i=0; i<QTD_AERONAVES; i++) indices_ordenados[i] = i;
    calculaPrioridade(aeronaves, indices_ordenados);

    // Escalonamento Round-Robin
    int i, delay_check, contador = 0;

    sleep(3);

    time_t inicioVoos = time(NULL);
    time_t agora;

    while(1){

        i = indices_ordenados[contador];

        // Se a aeronave ainda não teve sua entrada no espaço aéreo permitida
        if(aeronaves[i].status == DELAY){
            agora = time(NULL);
            delay_check = (int)(agora-inicioVoos);

            // Se a aeronave puder entrar no espaço aéreo
            if (delay_check > aeronaves[i].delay){
                aeronaves[i].status = VOANDO;

                // Se a aeronave tiver entrada permitida
                if (!verificaEntrada(aeronaves, i , pids)){ printf("\n☑️ A aeronave %d teve sua entrada permitida no espaço aéreo. ☑️\n", aeronaves[i].id); }
                else{
                    processos_finalizados++; 
                    printf("\n💭 %d processos finalizados\n", processos_finalizados);
                    // Libera a pista
                    int indice_pista = buscaIndicePista(aeronaves[i].pista_preferida);
                    pistas[indice_pista].ocupacao--;
                }
            }
        }

        // Pula tudo se a aeronave não tiver que ser considerada
        if(aeronaves[i].status != VOANDO && aeronaves[i].status != AGUARDANDO ){ 
            ++contador; contador = contador % QTD_AERONAVES; 
            continue; 
        }

        if (controleColisao(aeronaves, i, pids) || controleEngavetamento(aeronaves, pids)){
            processos_finalizados++;
            printf("\n💭 %d processos finalizados\n", processos_finalizados);
            // Libera a pista
            int indice_pista = buscaIndicePista(aeronaves[i].pista_preferida);
            pistas[indice_pista].ocupacao--;
        }

        // Se a aeronave tiver permissão para andar
        if (aeronaves[i].status == VOANDO){
            // printf("\n\n!!! %d PODE VOAR !!!\n\n", aeronaves[i].id);
            kill(pids[i], SIGCONT);
            sleep(1); // Dá tempo da aeronave andar
            kill(pids[i], SIGSTOP);
        }

        // Se a aeronave tiver pousado
        if(aeronaves[i].status == FINALIZADO){ 

            processos_finalizados++; 
            printf("\n💭 %d processos finalizados\n", processos_finalizados);

            // Libera a pista
            int indice_pista = buscaIndicePista(aeronaves[i].pista_preferida);
            pistas[indice_pista].ocupacao--;
            
        }

        // Condição de saída
        if(processos_finalizados == QTD_AERONAVES) break;

        ++contador; contador = contador % QTD_AERONAVES;
        
        if(contador == 0) calculaPrioridade(aeronaves, indices_ordenados);

    }

    printf("\n🥳 Todas as aeronaves pousaram. Encerrando programa. 🥳\n");

    // Libera as áreas de memória
    shmdt(aeronaves);
    shmctl(segmento_memoria, IPC_RMID, NULL);

    return 0;
}

int buscaIndicePista(int num_pista){
    for(int i=0; i<QTD_PISTAS; i++){
        if (num_pista == pistas[i].num) return i;
    }
    return -1;
}

void calculaPrioridade(Aeronave *aeronaves, int *array_indices){

    Ponto destino = {0.5, 0.5};
    float distancia[QTD_AERONAVES], dx[QTD_AERONAVES], dy[QTD_AERONAVES];
    
    for(int i=0; i<QTD_AERONAVES; i++){

        dx[i] = aeronaves[i].ponto.x - destino.x;
        dy[i] = aeronaves[i].ponto.y - destino.y;

        distancia[i] = sqrt( pow(dx[i],2) + pow(dy[i],2) );
    }

    // Bubble sort com base na distância ao destino
    int temp;
    for (int i = 0; i < QTD_AERONAVES - 1; i++) {
        for (int j = 0; j < QTD_AERONAVES - 1 - i; j++) {

            if (distancia[array_indices[j]] > distancia[array_indices[j + 1]]) {
                temp = array_indices[j];
                array_indices[j] = array_indices[j + 1];
                array_indices[j + 1] = temp;
            }

        }
    }

    printf("\n🕐 Ordem de prioridade das aeronaves: ");
    for(int i=0; i<QTD_AERONAVES; i++) printf("%d ", array_indices[i]);
    printf(" 🕐\n");

}

void criaAeronaves(int *segmento_memoria, int *pids){
    char str_segmento_memoria[20], str_indice_aeronave[20];

    for (int i=0; i<QTD_AERONAVES; i++){

        pids[i] = fork();
        if(pids[i]<0){ perror("Erro no fork"); exit(1); }

        else if(pids[i]==0){ // Filho
            // int -> string
            sprintf(str_segmento_memoria, "%d", *segmento_memoria);
            sprintf(str_indice_aeronave, "%d", i);

            // Executa o programa responsável pelas aeronaves
            execlp("./aeronave", "aeronave", str_segmento_memoria, str_indice_aeronave, NULL);
            exit(1);
        }

        // Dar tempo pras aeronaves serem criadas
        sleep(1);
    }
}

void controlePistas(Aeronave *aeronaves, int *pids){

    int indice_pista, indice_pista_secundaria, pista_secundaria;

    for(int i=0; i<QTD_AERONAVES; i++){

        indice_pista = buscaIndicePista(aeronaves[i].pista_preferida);
        if (indice_pista == -1) perror("Aeronave deseja pousar em uma pista inexistente");

        // Se a pista já estiver ocupada
        if (pistas[indice_pista].ocupacao != 0){

            // Procura uma pista alterntiva
            pista_secundaria = alteraPista(aeronaves[i].pista_preferida);
            indice_pista_secundaria = buscaIndicePista(pista_secundaria);

            // Se a outra pista tiver menos ocupada
            if (pistas[indice_pista_secundaria].ocupacao < pistas[indice_pista].ocupacao){
                printf("\n❗ Solicitando que a aeronave %d (pista %d) troque de pista ❗\n", aeronaves[i].id, aeronaves[i].pista_preferida);
                kill(pids[i], SIGCONT);
                kill(pids[i], SIGUSR2);
                sleep(1);
                kill(pids[i], SIGSTOP);

                // Confere se a pista foi alterada
                if (aeronaves[i].pista_preferida != pistas[indice_pista_secundaria].num) perror("Avião não mudou de pista quando solicitado");
            
                pistas[indice_pista_secundaria].ocupacao++;
            }
            // Se não, fica nela mesmo
            else pistas[indice_pista].ocupacao++;
        }
        // Se a pista estiver vazia
        else pistas[indice_pista].ocupacao++;
    }
}

int controleColisao(Aeronave *aeronaves, int i, int *pids){

    float distancia_x, distancia_y, x_projetado, y_projetado;
    int voando_mesma_direcao = 0, livre_de_colisao = 0;

    for(int j=0; j<QTD_AERONAVES; j++){

        // Se forem aeronaves diferentes, estiverem do mesmo lado e não tiverem pousado
        if (i == j || (aeronaves[j].status != VOANDO && aeronaves[j].status != AGUARDANDO) || aeronaves[i].direcao != aeronaves[j].direcao) continue;
        
        // Guarda quantas aeronaves estão voando na mesma direção
        voando_mesma_direcao++;

        distancia_x = fabs(aeronaves[j].ponto.x - aeronaves[i].ponto.x);
        distancia_y = fabs(aeronaves[j].ponto.y - aeronaves[i].ponto.y);

        // Colisão eminente -> Ordena que uma das aeronaves remeta o pouso
        if (distancia_x < 0.1 && distancia_y < 0.1){
            printf("\n🚫 Colisão eminente entre aeronaves %d [%.2f, %.2f] e %d [%.2f, %.2f]. Ordenando que a aeronave %d remeta o pouso 🚫\n", i, aeronaves[i].ponto.x, aeronaves[i].ponto.y, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y, i);
            kill(pids[i], SIGKILL);
            aeronaves[i].status = REMETIDA;
            return 1;
        }

        // Projeta a próxima posição da aeronave
        x_projetado = movimentaX(&aeronaves[i]);
        y_projetado = movimentaY(&aeronaves[i]);

        distancia_x = fabs(aeronaves[j].ponto.x - x_projetado);
        distancia_y = fabs(aeronaves[j].ponto.y - y_projetado);

        // Potencial de colisão -> Ordena redução de velocidade
        if (distancia_x < 0.1 && distancia_y < 0.1 && aeronaves[i].status == VOANDO){
            printf("\n⚠️ Potencial de colisão identificado entre aeronaves %d [%.2f, %.2f]->[%.2f, %.2f] e %d [%.2f, %.2f]. Ordenando redução da aeronave %d ⚠️\n", i, aeronaves[i].ponto.x, aeronaves[i].ponto.y, x_projetado, y_projetado, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y, i);
            kill(pids[i], SIGCONT);
            kill(pids[i], SIGUSR1);
            sleep(1);
            kill(pids[i], SIGSTOP);

            // Confere se o avião desacelerou
            if (aeronaves[i].status != AGUARDANDO) perror("Aeronave não desacelerou quando solicitado");

            break;
        }
        // Não há mais potencial de colisão com uma das possíveis aeronaves
        else if (!(distancia_x < 0.1 && distancia_y < 0.1) && aeronaves[i].status == AGUARDANDO){ 
            // printf("\n\n---> %d[%.2f, %.2f]->[%.2f, %.2f] livre de colisão com %d[%.2f, %.2f]\n\n", aeronaves[i].id, aeronaves[i].ponto.x, aeronaves[i].ponto.y, x_projetado, y_projetado, aeronaves[j].id, aeronaves[j].ponto.x, aeronaves[j].ponto.y);
            livre_de_colisao++; 
        }
    }

    // Se não tiverem mais aeronaves do mesmo lado do espaço aéreo
    if(voando_mesma_direcao == 0 && aeronaves[i].status == AGUARDANDO){
        printf("\n🆗 Não há mais potencial de colisão para a aeronave %d. Ordenando aumento da velocidade 🆗\n", aeronaves[i].id);
        
        // printf("\n\n!!! PERMISSÃO DADA POR 1 !!!\n\n");

        kill(pids[i], SIGCONT);
        kill(pids[i], SIGUSR1);
        sleep(1);
        kill(pids[i], SIGSTOP);

        // Confere se o avião acelerou
        if (aeronaves[i].status != VOANDO) perror("Avião não acelerou quando solicitado"); 
    }
    // Se a aeronave não for colidir com uma voando na mesma direção -> Ordena aceleração
    else if (livre_de_colisao == voando_mesma_direcao && voando_mesma_direcao > 0){
        printf("\n⚠️ Não há mais potencial de colisão para a aeronave %d. Ordenando aumento da velocidade ⚠️\n", aeronaves[i].id);
        
        // printf("\n\n!!! Permissão concedida para voo. %d livre de colisão e %d na mesma direção !!!\n\n", livre_de_colisao, voando_mesma_direcao);
        
        kill(pids[i], SIGCONT);
        kill(pids[i], SIGUSR1);
        sleep(1);
        kill(pids[i], SIGSTOP);
        // Confere se o avião acelerou
        if (aeronaves[i].status != VOANDO) perror("Avião não acelerou quando solicitado");
    }

    return 0;
}

int controleEngavetamento(Aeronave *aeronaves, int *pids){
    int bloqueados = 0;
    float distancia_x, distancia_y;

    for (int i = 0; i < QTD_AERONAVES; i++) {

        if (aeronaves[i].status != AGUARDANDO) continue;

        for (int j = 0; j < QTD_AERONAVES; j++) {
            if (i == j || (aeronaves[j].status != VOANDO && aeronaves[j].status != AGUARDANDO) ) continue;

            float x_projetado = movimentaX(&aeronaves[i]);
            float y_projetado = movimentaY(&aeronaves[i]);

            distancia_x = fabs(aeronaves[j].ponto.x - x_projetado);
            distancia_y = fabs(aeronaves[j].ponto.y - y_projetado);

            if (distancia_x < 0.1 && distancia_y < 0.1) { bloqueados++; break; }
        }
    }

    if (bloqueados > 1 && bloqueados + processos_finalizados == QTD_AERONAVES){
        printf("\n☢️ Emergência: aviões engavetados. Será necessário que 1 deles remeta o pouso. ☢️\n");
        for (int i = 0; i < QTD_AERONAVES; i++) {
            if (aeronaves[i].status == AGUARDANDO) {
                printf("\n🚫 Ordenando que a aeronave %d remeta o pouso 🚫\n", i);
                kill(pids[i], SIGKILL);
                aeronaves[i].status = REMETIDA;
                return 1;
            }
        }
    }

    return 0;
}

int verificaEntrada(Aeronave *aeronaves, int i, int *pids){
    float distancia_x, distancia_y;

    for(int j=0; j<QTD_AERONAVES; j++){

        // Se forem aeronaves diferentes, estiverem do mesmo lado e não tiverem pousado
        if (i == j || (aeronaves[j].status != VOANDO && aeronaves[j].status != AGUARDANDO) ) continue;

        distancia_x = fabs(aeronaves[j].ponto.x - aeronaves[i].ponto.x);
        distancia_y = fabs(aeronaves[j].ponto.y - aeronaves[i].ponto.y);

        // Colisão eminente -> Ordena que uma das aeronaves remeta o pouso
        if (distancia_x < 0.1 && distancia_y < 0.1){
            printf("\n🚫 Entrada negada a aeronave %d [%.2f, %.2f]. Risco de colisão com e %d [%.2f, %.2f] 🚫\n", i, aeronaves[i].ponto.x, aeronaves[i].ponto.y, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y);
            kill(pids[i], SIGKILL);
            aeronaves[i].status = REMETIDA;
            return 1;
        }
    }

    return 0;
} 