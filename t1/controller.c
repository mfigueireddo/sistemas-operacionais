#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork()
#include <sys/ipc.h> // IPC_CREAT, IPC_EXCL, IPC_PRIVATE
#include <sys/shm.h> // shmget, shmat, shmdt
#include <sys/stat.h> // S_IUSR, S_IWUSR
#include <signal.h> // SIGSTOP, SIGCONT, SIGUSR1, SIGKILL, ...
#include <math.h> // sqrt

// Estruturas personalizadas do trabalho
#include "aux.h"
typedef struct Aeronave Aeronave;

#define QTD_AERONAVES 5

int main(void){

    // Criando segmento de memória compartilhando
    int segmento_memoria = shmget(IPC_PRIVATE, sizeof(Aeronave)*QTD_AERONAVES, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento_memoria == -1){ perror("Erro na criação de memória compartilhada"); return 1; }

    // Ponteiro para o segmento de memória
    Aeronave *ptr_memoria = (Aeronave*)shmat(segmento_memoria, 0, 0);

    // Criando múltiplos processos
    int pid[5];
    char str_segmento_memoria[20], str_indice_aeronave[20];
    for (int i=0; i<QTD_AERONAVES; i++){

        pid[0] = fork();
        if(pid<0){ perror("Erro no fork"); return 2; }

        else if(pid==0){ // Filho
            // int -> string
            sprintf(str_segmento_memoria, "%d", segmento_memoria);
            sprintf(str_indice_aeronave, "%d", i);

            execlp("./aeronave", "aeronave", str_segmento_memoria, str_indice_aeronave, NULL);
            exit(1);
        }
    }

    // Começa pausando todas as aeronaves
    for(int i=0; i<QTD_AERONAVES; i++) kill(pid[i], SIGSTOP);

    // Escalonamento Round-Robin
    int i = 0;
    float distancia, x, y;

    // !!! DAR PRIORIDADE PARA AERONAVES PRÓXIMAS DE ATERRISAR !!!

    while(1){

        // !!! CONFERIR SE AERONAVES QUEREM ENTRAR NA MESMA PLATAFORMA !!!

        // !!! SÓ CONFERIR PARA AERONAVES VINDO DO MESMO LADO !!!

        // Confere se a aeronave da vez está a uma distância segura das demais
        for(int j=0; j<QTD_AERONAVES; j++){
            if (i!=j) { // Se forem aeronaves diferentes

                // !!! FAZER DISTÂNCIA PARA X e Y !!!

                x = pow((ptr_memoria[j].ponto.x - ptr_memoria[i].ponto.x), 2);
                y = pow((ptr_memoria[j].ponto.y - ptr_memoria[i].ponto.y), 2);
                distancia = sqrt(x+y);

                // Possível colisão entre aeronaves
                // !!! ACHAR UMA DISTÂNCIA QUE SEJA SEGURO PARA PEDIR PARA ELAS REDUZIREM !!!
                if (distancia < ?){

                }

                // !!! PARAR O POUSO DE UMA DAS AERONAVES SE FOR IMPOSSÍVEL EVITAR COLISÃO !!!
                // !!! CONFERIR TAMBÉM SE A REDUÇÃO DE VELOCIDADE NÃO RESOLVERIA !!!
                if (distancia < 0.1){

                }

                // !!! CONFERIR SE O SIGNAL FEZ EFEITO !!!
            }
        }

        kill(pid[i], SIGCONT);
        sleep(1);
        kill(pid[i], SIGSTOP);

        // !!! CONFERIR SE A AERONAVE POUSOU !!!

        i = (++i) % QTD_AERONAVES;
    }

    return 0;
}