#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAXFILA 8
#define MAXITENS 64

int buffer[MAXFILA];
int idx_consumidor = 0, idx_produtor = 0, cont_buffer = 0;
int cont_produtos = 0, cont_consumidos = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t flag_buffer_vazio = PTHREAD_COND_INITIALIZER;
pthread_cond_t flag_buffer_cheio = PTHREAD_COND_INITIALIZER;

void* produtor(void* arg);
void* consumidor(void* arg);

int main() 
{
    srand(time(NULL));
    pthread_t thread_produtor, thread_consumidor;

    pthread_create(&thread_produtor, NULL, produtor, NULL);
    pthread_create(&thread_consumidor, NULL, consumidor, NULL);

    pthread_join(thread_produtor, NULL);
    pthread_join(thread_consumidor, NULL);

    printf("Programa encerrado.\n");
    return 0;
}

void* produtor(void* arg) 
{
    while (cont_produtos < MAXITENS) 
    {

        // Restringe acesso à área crítica
        pthread_mutex_lock(&mutex);
        
        // Aguarda o esvaziamento do buffer
        while (cont_buffer == MAXFILA) { pthread_cond_wait(&flag_buffer_cheio, &mutex); }

        // Gera um número aleatório
        int random_data = rand() % 64 + 1;

        // Coloca o número no buffer
        buffer[idx_produtor] = random_data;

        // Atualiza variáveis auxiliares
        idx_produtor = (idx_produtor + 1) % MAXFILA;
        cont_buffer++;
        cont_produtos++;

        printf("Produtor\n");
        printf("> Número gerado: %d\n", random_data);
        printf("> Buffer atual: %d de no máximo %d no buffer\n\n", cont_buffer, MAXFILA);
        
        // Avisa o consumidor que há algo no buffer
        pthread_cond_signal(&flag_buffer_vazio);

        // Libera acesso à área crítica
        pthread_mutex_unlock(&mutex);
        
        // Produz a cada 1 segundo
        sleep(1);
    }

    return NULL;
}

void* consumidor(void* arg)
{
    while (cont_consumidos < MAXITENS) {

        // Restringe acesso à área crítica
        pthread_mutex_lock(&mutex);
        
        // Aguarda que até que haja algo no buffer
        while (cont_buffer == 0) { pthread_cond_wait(&flag_buffer_vazio, &mutex); }

        // Gera um número aleatório
        int random_data = buffer[idx_consumidor];

        // Atualiza variáveis auxiliares
        idx_consumidor = (idx_consumidor + 1) % MAXFILA;
        cont_buffer--;
        cont_consumidos++;

        printf("Consumidor\n");
        printf("> Número processado: %d\n", random_data);
        printf("> Buffer atual: %d de no máximo %d no buffer\n\n", cont_buffer, MAXFILA);
        
        // Avisa o produtor que o buffer foi esvaziado (mesmo que parcialmente)
        pthread_cond_signal(&flag_buffer_cheio);

        // Libera acesso à área crítica
        pthread_mutex_unlock(&mutex);
        
        // Consome a cada 2 segundos
        sleep(2);
    }

    return NULL;
}