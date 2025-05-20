#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>

// Arquivos header
#include "aux.h"

// Estruturas personalizadas do trabalho
typedef struct Aeronave Aeronave;

// Funções do módulo
void toggle_velocidade(int sig);
void toggle_pista(int sig);
void configurar_inicialmente(struct Aeronave *aeronave, int index);

// Variáveis globais do módulo
//Ponteiro global para acessar a aeronave desse processo e velocidade padrão inicial
Aeronave *minha_aeronave = NULL; 
float velocidade_original = 0.05;

 //Função principal. Espera dois argumentos: o ID da memória compartilhada e o índice da aeronave
int main(int argc, char *argv[]) {

    // Confere se os argumento necessários foram passados. Se forem inválidos, exibe erro e finaliza
    if (argc != 3) { fprintf(stderr, "Uso correto: %s <shm_id> <index>\n", argv[0]); exit(1); }

    // Guarda os argumentos passados. Converte os argumentos recebidos (que são strings) para inteiros
    int shm_id = atoi(argv[1]);
    int index = atoi(argv[2]);

    //Acessa a memória compartilhada, criando um ponteiro, usando o ID passado
    struct Aeronave *memoria = ( struct Aeronave *) shmat(shm_id, NULL, 0);

    //Verifica erro ao mapear a memória e atribui minha_aeronave para a posição do vetor que representa esta aeronave
    if (memoria == (void *)-1) { perror("Erro no shmat"); exit(1); }
    minha_aeronave = &memoria[index];

    // Chama a função para configurar os dados iniciais dessa aeronave
    configurar_inicialmente(minha_aeronave, index);

    // Instala os tratadores de sinal
    // Registra funções que devem ser chamadas quando esses sinais forem recebidos
    signal(SIGUSR1, toggle_velocidade);
    signal(SIGUSR2, toggle_pista);

    // Tempo suficiente pra aguardar a liberação do controller. Para dar tempo do controller configurar as pistas
    sleep(6);

    // Loop principal. Esse while executa continuamente o comportamento da aeronave
    while (1) {

        //  A aeronave só se movimenta se estiver com status VOANDO. Caso contrário, ela espera 2 segundos e recomeça o loop
        if(minha_aeronave->status != VOANDO){ 
            sleep(2); continue;
        }

        //Imprime o ponto atual antes da movimentação
        printf("\n▶️ Mudança de posição - Aeronave %d [%.2f, %.2f] -> ", minha_aeronave->id, minha_aeronave->ponto.x, minha_aeronave->ponto.y);

        //Atualiza a posição X e Y da aeronave usando as funções auxiliares
        minha_aeronave->ponto.x = movimentaX(minha_aeronave);
        minha_aeronave->ponto.y = movimentaY(minha_aeronave);

        // Imprime a nova posição
        printf("[%.2f, %.2f]\n", minha_aeronave->ponto.x, minha_aeronave->ponto.y);

        // Confere se chegou no destino. Se chegou no destino, sai do loop (posição central do espaço aéreo)
        if (minha_aeronave->ponto.x == 0.5 && minha_aeronave->ponto.y == 0.5) break;

        // Delay para que a aeronave só avance 1x a cada permissão
        sleep(3);
    }

    //Mensagem de pouso + altera status da aeronave
    printf("\n✅ Aeronave %d pousou na pista %d. Encerrando processo ✅\n", minha_aeronave->id, minha_aeronave->pista_preferida);
    minha_aeronave->status = FINALIZADO;

    // Desanexa o processo da memória compartilhada
    shmdt(memoria);

    //Fim do processo da aeronave

    return 0;
}

// Handler de velocidade, ou seja, será chamada automaticamente quando o processo receber um sinal (SIGUSR1).
// Essa função altera entre os estados VOANDO e AGUARDANDO da aeronave, simulando "pausa e retomada" de voo
//O parâmetro sig representa o número do sinal recebido (mas não é usado dentro da função)
void toggle_velocidade(int sig) {

    //Verifica se a aeronave está atualmente no estado VOANDO. Se sim, ela será "pausada"
    if(minha_aeronave->status == VOANDO){
        printf("\n🔁 Aeronave %d aguardando permissão para continuar. 🔁\n", minha_aeronave->id);
        minha_aeronave->status = AGUARDANDO;
    }
    // Se a aeronave estiver parada, ela passa a voar
    else{
        printf("\n🔁 Aeronave %d continuando o trajeto 🔁\n", minha_aeronave->id);
        minha_aeronave->status = VOANDO;
    }

    // Tempo para que a aeronave não ande sem que o controller faça um controle de colisão e engavetamento antes
    sleep(1);
}

// Handler de pista, neste caso, associado ao SIGUSR2
// Essa função troca a pista preferida da aeronave usando alteraPista(). Serve para balancear carga nas pistas
void toggle_pista(int sig) {

    //Mostra qual é a pista atual da aeronave e que ela será alterada
    printf("\n🔁 Pista da aeronave %d alterada (%d -> ", minha_aeronave->id, minha_aeronave->pista_preferida);

    //Chama a função alteraPista() passando a pista atual
    //Essa função devolve a pista alternativa (ex: se for 6 retorna 27)
    //Atualiza a pista preferida da aeronave com esse novo valor
    minha_aeronave->pista_preferida = alteraPista(minha_aeronave->pista_preferida);

    //Finaliza a mensagem no terminal informando a nova pista após a troca
    printf("%d) 🔁\n", minha_aeronave->pista_preferida);
}


// Essa função inicializa todos os dados da aeronave
// Essa função é chamada no início da execução de cada processo de aeronave
// Ela recebe um ponteiro para a estrutura Aeronave da memória compartilhada e o índice (posição no vetor).
void configurar_inicialmente(struct Aeronave *aeronave, int index) {

    printf("\n🔴 Criando aeronave 🔴\n");

    // Garante aleatoridade entre diferentes processos
    // Inicializa a semente do gerador de números aleatórios com base no tempo atual + índice da aeronave
    // Isso evita que todas as aeronaves recebam os mesmos valores aleatórios
    srand(time(NULL) + index); 

    // Define o ID da aeronave com o índice dela no vetor de memória compartilhada
    aeronave->id = index;
    // Salva o PID (Process ID) do processo que representa a aeronave. Isso será usado para enviar sinais mais tarde
    aeronave->pid = getpid();
    //orteia a direção da aeronave: 'W' (vindo da esquerda) ou 'E' (vindo da direita).
    aeronave->direcao = (rand() % 2 == 0) ? 'W' : 'E'; 

    // Sorteia ponto de entrada (x fixo e y aleatório)
    // Define a coordenada x inicial: 
    // Se vier do oeste ('W'), começa em x = 0.0 (esquerda da tela). Se vier do leste ('E'), começa em x = 1.0 (direita da tela)
    // Também sorteia a pista preferida da aeronave, compatível com o lado de entrada
    if (aeronave->direcao == 'W') {
        aeronave->ponto.x = 0.0;
        aeronave->pista_preferida = (rand() % 2 == 0) ? 18 : 3;
    } else {
        aeronave->ponto.x = 1.0;
        aeronave->pista_preferida = (rand() % 2 == 0) ? 6 : 27;
    }

    //Sorteia a coordenada y inicial (altura), variando entre 0.0 e 1.0 com passo de 0.1
    aeronave->ponto.y = (float)(rand() % 11) / 10.0; 

    // Define a velocidade padrão da aeronave (0.05, declarada como variável global)
    aeronave->velocidade = velocidade_original;

    // Sorteia delay entre 0 e 2 (inclusos)
    aeronave->delay = rand() % 3; 

    //icializa o status da aeronave como DELAY, ou seja, ela ainda não está no espaço aéreo
    aeronave->status = DELAY;

    //Mensagens informando sucesso na criação e exibindo os dados da aeronave na tela
    printf("🟢 Aeronave criada com sucesso 🟢\n");
    imprimeAeronave(aeronave);
}