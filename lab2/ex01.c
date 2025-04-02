#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(void){ 

   int segmento[3], *matriz[3], pid;

   // Criando segmentos de memória
   for(int i = 0; i < 3; i++) {
       segmento[i] = shmget(IPC_PRIVATE, 9 * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
       if (segmento[i] == -1) {
           printf("Erro na criação do segmento de memória compartilhada\n");
           return 1;
       }
   }

   // Linkando os segmentos de memória
   for(int i=0; i<3; i++){
    matriz[i] = (int*)shmat(segmento[i], 0, 0);
   }

   // Salvando os valores das matrizes
   int aux[3][9]= {
    {5, 7, 9, 6, 3, 6, 3, 1, 2},
    {5, 3, 0, 6, 2, 6, 5, 7, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
   };

   // Atribuindo os valores às matrizes
   for(int i = 0; i < 9; i++) {
       matriz[0][i] = aux[0][i];
       matriz[1][i] = aux[1][i];
       matriz[2][i] = aux[2][i];
   }

   for(int i = 0; i < 3; i++) {
       pid = fork();

        // Erro
        if(pid<0){
            printf("Erro\n");
            return 2;
        } 

        // Filho
        else if(pid == 0){
            // Soma de linhas
            for(int j=i*3; j < (i+1)*3 ; j++){
                matriz[2][j] = matriz[0][j] + matriz[1][j];
            }
            return 0;
        }
   }

    // Pai espera todos os filhos
    for(int i = 0; i < 3; i++) {
        wait(NULL);
    }
 

   // Imprime o resultado final
   for(int i = 0; i < 9; i++) {
       printf("%d ", matriz[2][i]);
       if(i == 2 || i == 5) {
           printf("\n");
       }
   }

   // Desanexando e destruindo os segmentos de memória
   for(int i = 0; i < 3; i++) {
        shmdt(matriz[i]);
        shmctl(segmento[i], IPC_RMID, 0);
    }

   return 0;
}