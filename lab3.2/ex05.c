#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void erro_divisao(int sign);

int main(void){

    signal(SIGFPE, erro_divisao);

    int nums[2], resultado;

    printf("Insira um número: ");
    scanf("%d", &nums[0]);

    printf("Insira outro número: ");
    scanf("%d", &nums[1]);

    // Soma
    resultado = nums[0]+nums[1];
    printf("\nSoma: %d", resultado);

    // Subtração
    resultado = nums[0]-nums[1];
    printf("\nSubtração: %d", resultado);

    // Multiplicação
    resultado = nums[0]*nums[1];
    printf("\nMultiplicação: %d", resultado);

    // Divisão
    resultado = nums[0]/nums[1];
    printf("\nDivisão: %d\n", resultado);

    return 0;
}

void erro_divisao(int sign){
    printf("\nErro: divisão por 0\n");
    exit(1);
}