#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork()
#include <sys/ipc.h> // IPC_CREAT, IPC_EXCL, IPC_PRIVATE
#include <sys/shm.h> // shmget, shmat, shmdt
#include <sys/stat.h> // S_IUSR, S_IWUSR
#include <sys/wait.h> // waitpid
#include <signal.h> // SIGSTOP, SIGCONT, SIGUSR1, SIGKILL, ...
#include <math.h> // sqrt

// Estruturas personalizadas do trabalho
#include "aux.h"
typedef struct Aeronave Aeronave;
typedef struct Pista Pista;
typedef struct Ponto Ponto;

// Macros do módulo
#define QTD_AERONAVES 5
#define QTD_PISTAS 4

// Funções do módulo
int buscaIndicePista(int num_pista);
void calculaPrioridade(Aeronave *aeronaves, int *array_indices);

// Variável com as pistas
Pista pistas[QTD_PISTAS] = { {3, 0}, {6, 0}, {18, 0}, {27, 0} };

int main(void){

    // Criando segmento de memória compartilhando
    int segmento_memoria = shmget(IPC_PRIVATE, sizeof(Aeronave)*QTD_AERONAVES, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento_memoria == -1){ perror("Erro na criação de memória compartilhada"); return 1; }

    // Ponteiro para o segmento de memória
    Aeronave *aeronaves = (Aeronave*)shmat(segmento_memoria, 0, 0);
    if (aeronaves == (void *)-1) { perror("Erro no shmat"); exit(1); }

    // Criando múltiplos processos
    int pid[5];
    char str_segmento_memoria[20], str_indice_aeronave[20];
    for (int i=0; i<QTD_AERONAVES; i++){

        pid[i] = fork();
        if(pid[i]<0){ perror("Erro no fork"); return 2; }

        else if(pid[i]==0){ // Filho
            // int -> string
            sprintf(str_segmento_memoria, "%d", segmento_memoria);
            sprintf(str_indice_aeronave, "%d", i);

            // Executa o programa responsável pelas aeronaves
            execlp("./aeronave", "aeronave", str_segmento_memoria, str_indice_aeronave, NULL);
            exit(1);
        }

        // Dar tempo pras aeronaves serem criadas
        sleep(1);
    }

    // Confere se não há aeronaves com a mesma pista de destino
    int indice_pista;
    printf("\n⚠️ Ordenando a pausa de todas as aeronaves ⚠️\n");
    
    for(int i=0; i<QTD_AERONAVES; i++) kill(pid[i], SIGSTOP); // Começa pausando todas as aeronaves

    for(int i=0; i<QTD_AERONAVES; i++){

        indice_pista = buscaIndicePista(aeronaves[i].pista_preferida);
        if (indice_pista == -1) perror("Aeronave deseja pousar em uma pista inexistente");

        // Se a pista já estiver ocupada
        if (pistas[indice_pista].estaOcupada){
            printf("\n❗ Solicitando que a aeronave %d (pista %d) troque de pista ❗\n", aeronaves[i].id, aeronaves[i].pista_preferida);
            kill(pid[i], SIGCONT);
            kill(pid[i], SIGUSR2);
            sleep(1);
            kill(pid[i], SIGSTOP);
            
            // Confere se a pista foi alterada
            if (aeronaves[i].pista_preferida == pistas[indice_pista].num) perror("Avião não mudou de pista quando solicitado");
        }

        // Se a pista estiver vazia
        else pistas[indice_pista].estaOcupada = 1;
    }

    // Escalonamento Round-Robin
    int idx = 0;
    float distancia_x, distancia_y;

    // Ordem das aeronaves mais próximas ao destino
    int indices_ordenados[QTD_AERONAVES];
    for(int i=0; i<QTD_AERONAVES; i++) indices_ordenados[i] = i;

    calculaPrioridade(aeronaves, indices_ordenados);

    printf("\n⚠️ Ordem de prioridade das aeronaves: ");
    for(int i=0; i<QTD_AERONAVES; i++) printf("%d ", indices_ordenados[i]);
    printf(" ⚠️\n");

    int processos_finalizados = 0;

    sleep(1);

    while(1){

        // Pula todos os procedimentos se a aeronave tiver pousado
        if(aeronaves[indices_ordenados[idx]].status == FINALIZADO){ ++idx; idx = idx % QTD_AERONAVES; continue; }

        // Controle de colisão
        for(int j=0; j<QTD_AERONAVES; j++){

            // Se forem aeronaves diferentes, estiverem do mesmo lado e não tiverem pousado
            if (indices_ordenados[idx]!=j && aeronaves[indices_ordenados[idx]].direcao == aeronaves[j].direcao && aeronaves[j].status != FINALIZADO) {

                distancia_x = fabs(aeronaves[j].ponto.x - aeronaves[indices_ordenados[idx]].ponto.x);
                distancia_y = fabs(aeronaves[j].ponto.y - aeronaves[indices_ordenados[idx]].ponto.y);

                // Potencial de colisão -> Ordena redução de velocidade
                if ( (distancia_x > 0.1 && distancia_x < 0.2) && (distancia_y > 0.1 && distancia_y < 0.2) ){
                    printf("\n⚠️ Potencial de colisão identificado entre aeronaves %d [%.2f, %.2f] e %d [%.2f, %.2f]. Ordenando redução da aeronave %d ⚠️\n", indices_ordenados[idx], aeronaves[indices_ordenados[idx]].ponto.x, aeronaves[indices_ordenados[idx]].ponto.y, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y, indices_ordenados[idx]);
                    kill(pid[indices_ordenados[idx]], SIGCONT);
                    kill(pid[indices_ordenados[idx]], SIGUSR1);
                    sleep(1);
                    kill(pid[indices_ordenados[idx]], SIGSTOP);

                    // Confere se o avião desacelerou
                    if (aeronaves[indices_ordenados[idx]].status != AGUARDANDO) perror("Avião não desacelerou quando solicitado");
                }

                // Colisão eminente -> Ordena que uma das aeronaves remeta o pouso
                else if (distancia_x < 0.1 && distancia_y < 0.1){
                    printf("\n⚠️ Colisão eminente entre aeronaves %d [%.2f, %.2f] e %d [%.2f, %.2f]. Ordenando que a aeronave %d remeta o pouso ⚠️\n", indices_ordenados[idx], aeronaves[indices_ordenados[idx]].ponto.x, aeronaves[indices_ordenados[idx]].ponto.y, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y, indices_ordenados[idx]);
                    aeronaves[indices_ordenados[idx]].status = FINALIZADO;
                    processos_finalizados++;
                    kill(pid[indices_ordenados[idx]], SIGKILL);
                    continue;
                }
            }
        }

        kill(pid[indices_ordenados[idx]], SIGCONT);
        sleep(1); // Dá tempo do avião andar
        kill(pid[indices_ordenados[idx]], SIGSTOP);

        // Confere se a aeronave já pousou
        int status;
        if(waitpid(pid[indices_ordenados[idx]], &status, WNOHANG) > 0){ 

            aeronaves[indices_ordenados[idx]].status = FINALIZADO;
            processos_finalizados++;

            // Libera a pista
            indice_pista = buscaIndicePista(aeronaves[indices_ordenados[idx]].pista_preferida);
            pistas[indice_pista].estaOcupada = 0;

            // Se tiver alguma aeronave esperando, manda ela continuar
            for(int j=0; j<QTD_AERONAVES; j++){
                if(indices_ordenados[idx]!=j && aeronaves[indices_ordenados[idx]].pista_preferida == aeronaves[j].pista_preferida && aeronaves[j].status == AGUARDANDO){
                    kill(pid[j], SIGCONT);
                    kill(pid[j], SIGUSR1);
                    sleep(1);
                    kill(pid[j], SIGSTOP);
                    // Confere se o avião voltou a andar
                    if (aeronaves[indices_ordenados[idx]].status != VOANDO) perror("Avião não acelerou quando solicitado");
                }
            }
            
        }

        // Condição de saída
        if(processos_finalizados == QTD_AERONAVES) break;

        ++idx; idx = idx % QTD_AERONAVES;        
    }

    printf("\n⚠️ Todas as aeronaves pousaram. Encerrando programa ⚠️\n");

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

}