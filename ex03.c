#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void imprime(int *vetor, int tam);
void ordena(int *vetor, int tam);

int vetor[] = {1, 36, 5, 20, 2};

int main(void){

    imprime(vetor,5);

    int pid;
    pid= fork();

    if(pid<0){
        printf("deu erro");
    }
    else if(pid==0){
        ordena(vetor, 5);
        imprime(vetor, 5);
        exit(1);
    }
    else{
        waitpid(pid, NULL, 0);
        imprime(vetor,5);
    }
    return 0;

}

void imprime(int *vetor, int tam){
    for(int i=0; i<tam; i++){
        printf("%d ", vetor[i]);
    }
    printf("\n");
}

void ordena(int *vetor, int tam){
    for(int i=0; i<tam-1; i++){
        for(int j=0; j<tam-1-i; j++){
            if(vetor[j]>vetor[j+1]){
                int temp = vetor[j];
                vetor[j] = vetor[j+1];
                vetor[j+1] = temp;
            }
        }
    }
}