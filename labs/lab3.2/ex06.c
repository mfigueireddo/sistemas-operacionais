#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void liga(int sign);
void desliga(int sign);
float calcula(double time);

time_t begin, end;
int andamento = 0;

int main(void){

    signal(SIGUSR1, liga);
    signal(SIGUSR2, desliga);
    
    while(1){
        pause();
    }

    return 0;
}

void liga(int sign){

    if (andamento) printf("Você já está em uma ligação.\n");

    printf("Ligação iniciada\n");
    begin = time(NULL);
    andamento = 1;

}

void desliga(int sign){

    end = time(NULL);
    double time = difftime(end, begin);
    float price = calcula(time);   

    printf("Ligação encerrada. Duração: %.0f segundos\n", time);
    printf("O preço da ligação foi de R$%.2f\n", price/100);

    andamento = 0;
    exit(1);
}

float calcula(double time){

    if (time <= 60){
        return time*2;
    }

    else{
        time -= 60;
        return 120 + time;
    }

}