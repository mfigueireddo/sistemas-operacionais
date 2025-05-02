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

// Constantes do módulo
#define QTD_AERONAVES 5
#define QTD_PISTAS 4

// Funções do módulo
int buscaIndicePista(int num_pista);

// Variável com as pistas
Pista pistas[QTD_PISTAS] = { {3, 0}, {6, 0}, {18, 0}, {27, 0} };

int main(void){

    // Criando segmento de memória compartilhando
    int segmento_memoria = shmget(IPC_PRIVATE, sizeof(Aeronave)*QTD_AERONAVES, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento_memoria == -1){ perror("Erro na criação de memória compartilhada"); return 1; }

    // Ponteiro para o segmento de memória
    Aeronave *aeronaves = (Aeronave*)shmat(segmento_memoria, 0, 0);

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
    }

    // Começa pausando todas as aeronaves
    // Confere se não há aeronaves com a mesma pista de destino
    int indice_pista;
    for(int i=0; i<QTD_AERONAVES; i++){
        kill(pid[i], SIGSTOP);

        indice_pista = buscaIndicePista(aeronaves->pista_preferida);
        if (indice_pista == -1) perror("Aeronave deseja pousar em uma pista inexistente");

        // Se a pista já estiver ocupada
        if (pistas[indice_pista].estaOcupada) kill(pid[i], SIGUSR2);
        // Se a pista estiver vazia
        else pistas[indice_pista].estaOcupada = 1;

    }

    // Escalonamento Round-Robin
    int i = 0;
    float distancia_x, distancia_y;

    // !!! DAR PRIORIDADE PARA AERONAVES PRÓXIMAS DE ATERRISAR !!!
    

    while(1){

        // Confere se a aeronave já pousou
        // !!! COLOCAR LÁ PRO FUNDO !!!
        // !!! ISSO SÓ DEVE ACONTECER 1X !!!
        // !!! DEPOIS DISSO IDENTIFICAR QUE O PROCESSO JÁ FOI FINALIZADO E SÓ PULAR ELE !!!
        int status;
        if(waitpid(pid[i], status, WNOHANG) > 0){ 

            // Libera a pista
            indice_pista = buscaIndicePista(aeronaves[i].pista_preferida);
            pistas[indice_pista].estaOcupada = 0;

            // Se tiver alguma aeronave esperando, manda ela continuar
            for(int j=0; j<QTD_AERONAVES; j++){
                if(i!=j && aeronaves[i].pista_preferida == aeronaves[j].pista_preferida && aeronaves[j].status == AGUARDANDO){
                    kill(pid[j], SIGUSR1);
                    if (aeronaves[i].status != VOANDO) perror("Avião não acelerou quando solicitado"); // Confere a ação foi efetiva
                }
            }

            continue;
        }

        // Controle de colisão
        for(int j=0; j<QTD_AERONAVES; j++){
            // Se forem aeronaves diferentes e estiverem do mesmo lado
            if (i!=j && aeronaves[i].direcao == aeronaves[j].direcao) {

                distancia_x = fabs(aeronaves[j].ponto.x - aeronaves[i].ponto.x);
                distancia_y = fabs(aeronaves[j].ponto.y - aeronaves[i].ponto.y);

                // Potencial de colisão -> Ordena redução de velocidade
                if ( (distancia_x > 0.1 && distancia_x < 0.2) || (distancia_y > 0.1 && distancia_y < 0.2) ){
                    kill(pid[i], SIGUSR1);
                    if (aeronaves[i].status != AGUARDANDO) perror("Avião não desacelerou quando solicitado"); // Confere a ação foi efetiva
                }

                // Colisão eminente -> Ordena que uma das aeronaves remeta o pouso
                else if (distancia_x < 0.1 || distancia_y < 0.1){
                    kill(pid[i], SIGKILL);
                }
            }
        }

        kill(pid[i], SIGCONT);
        sleep(1); // !!! ESSE TEMPO É SUFICIENTE? !!!
        kill(pid[i], SIGSTOP);

        i = (++i) % QTD_AERONAVES;
    }

    // !!! LIMPAR O NECESSÁRIO !!!

    return 0;
}

int buscaIndicePista(int num_pista){
    for(int i=0; i<QTD_PISTAS; i++){
        if (num_pista == pistas[i].num) return i;
    }
    return -1;
}

void calculaPrioridade(Aeronave *aeronaves){

    Ponto destino = {0.5, 0.5};
    float distancia[QTD_AERONAVES], dx[QTD_AERONAVES], dy[QTD_AERONAVES];
    
    for(int i=0; i<QTD_AERONAVES; i++){

        dx[i] = aeronaves[i].ponto.x - destino.x;
        dy[i] = aeronaves[i].ponto.y - destino.y;

        distancia[i] = sqrt( pow(dx[i],2) + pow(dy[i],2) );
    }

    // Bubble sort com base na distância ao destino
    for (int i = 0; i < QTD_AERONAVES - 1; i++) {
        for (int j = 0; j < QTD_AERONAVES - 1 - i; j++) {
            if (distancia[indices[j]] > distancia[indices[j + 1]]) {
                // troca os índices
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }

}