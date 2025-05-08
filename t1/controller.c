#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork()
#include <sys/ipc.h> // IPC_CREAT, IPC_EXCL, IPC_PRIVATE
#include <sys/shm.h> // shmget(), shmat(), shmdt()
#include <sys/stat.h> // S_IUSR, S_IWUSR
#include <sys/wait.h> // waitpid()
#include <signal.h> // SIGSTOP, SIGCONT, SIGUSR1, SIGKILL, ...
#include <math.h> // sqrt()
#include <time.h> // time()
#include <string.h> // strcspn()

// Interface
#include <pthread.h>
pthread_t controller_thread;
int flag_interface = 0;
int flag_fecha_thread = 0;

// Arquivos header
#include "aux.h"

// Estruturas personalizadas do trabalho
typedef struct Aeronave Aeronave;
typedef struct Pista Pista;
typedef struct Ponto Ponto;

// Funções do módulo
int buscaIndicePista(int num_pista);
void calculaPrioridade(Aeronave *aeronaves, int *array_indices);
void criaAeronaves(int *segmento_memoria, int *pids);
void controlePistas(Aeronave *aeronaves, int *pids);
int controleColisao(Aeronave *aeronaves, int i, int *pids);
int controleEngavetamento(Aeronave *aeronaves, int *pids);
int verificaEntrada(Aeronave *aeronaves, int i, int *pids);
void formalizaPouso(Aeronave* aeronave);
void imprimeResultados(void);
void* interface(void* arg);

// Variáveis globais do módulo
static Aeronave *aeronaves = NULL;
static int pids[QTD_AERONAVES];
static Pista pistas[QTD_PISTAS] = { {3, 0}, {6, 0}, {18, 0}, {27, 0} };
static int processos_finalizados = 0;
static int bloqueados;
static int indices_ordenados[QTD_AERONAVES];

int main(void){

    //printf("Entre com CTRL+\\ para abrir o terminal.\n");

    // Criando segmento de memória compartilhando
    int segmento_memoria = shmget(IPC_PRIVATE, sizeof(Aeronave)*QTD_AERONAVES, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento_memoria == -1){ perror("Erro na criação de memória compartilhada"); return 1; }

    // Ponteiro para o segmento de memória
    aeronaves = (Aeronave*)shmat(segmento_memoria, 0, 0);
    if (aeronaves == (void *)-1) { perror("Erro no shmat"); exit(1); }

    // Criando múltiplos processos (aeronaves)
    criaAeronaves(&segmento_memoria, pids);

    // Inicia thread de interface de usuário
    pthread_create(&controller_thread, NULL, interface, NULL);

    // Começa pausando todas as aeronaves
    printf("\n⚠️ Ordenando a pausa de todas as aeronaves ⚠️\n");
    for(int i=0; i<QTD_AERONAVES; i++) kill(pids[i], SIGSTOP); 

    // Aciona a interface quando o usuário entra com CTRL+\ .
    //signal(SIGQUIT, interface);
    //signal(SIGINT, interface);

    // Confere se não há aeronaves com a mesma pista de destino
    controlePistas(aeronaves, pids);

    // Ordem de execução dos processos com base na distância da aeornave ao destino
    for(int i=0; i<QTD_AERONAVES; i++) indices_ordenados[i] = i;
    calculaPrioridade(aeronaves, indices_ordenados);

    // Escalonamento Round-Robin
    int i, contador = 0;

    // Efeito unicamente visual: para dar uma pausa antes da enxurrada de impressões
    sleep(3);

    // Gerenciamento do momento de ativação de uma aeronave
    int delay_check;
    time_t inicioVoos = time(NULL);
    time_t agora;

    while(1){

        if(flag_interface) continue;

        // Itera sobre a ordem de prioridade e não sobre o ID das aeronaves
        i = indices_ordenados[contador];

        // Se a aeronave ainda não teve sua entrada no espaço aéreo permitida
        if(aeronaves[i].status == DELAY){

            // Calcula quanto tempo se passou desde que o programa foi iniciado
            agora = time(NULL);
            delay_check = (int)(agora-inicioVoos);

            // Se a aeronave puder entrar no espaço aéreo (momento de ativação ultrapassado)
            if (delay_check > aeronaves[i].delay){

                aeronaves[i].status = VOANDO;

                // Se a aeronave tiver entrada permitida
                if (!verificaEntrada(aeronaves, i , pids)){ printf("\n☑️ A aeronave %d teve sua entrada permitida no espaço aéreo. ☑️\n", aeronaves[i].id); }
                
                // Se a aeronave não tiver a entrada permitida (colisão eminente)
                else{ formalizaPouso(&aeronaves[i]); }
            }
        }

        // Pula tudo se a aeronave não estiver ativamente no espaço aéreo (andando ou parada)
        if(aeronaves[i].status != VOANDO && aeronaves[i].status != AGUARDANDO ){ 
            ++contador; contador = contador % QTD_AERONAVES; 
            continue; 
        }

        // Se a aeronave tiver sido remetida pelos algoritmos de controle
        if (controleColisao(aeronaves, i, pids) || controleEngavetamento(aeronaves, pids)){ formalizaPouso(&aeronaves[i]); }

        // Se a aeronave tiver permissão para continuar
        if (aeronaves[i].status == VOANDO){
            // printf("\nTESTE: Enviando sinal para processo continuar\n");
            //imprimeAeronave(&aeronaves[i]);
            //if (kill(pids[i], 0) == 0) printf("Processo vivo\n");
            kill(pids[i], SIGCONT);
            sleep(1); // Dá um tempo para aeronave.c aplicar as mudanças de posição
            kill(pids[i], SIGSTOP);
        }

        // Se a aeronave tiver pousado com sucesso
        if(aeronaves[i].status == FINALIZADO){ formalizaPouso(&aeronaves[i]); }

        // Condição de saída
        if(processos_finalizados == QTD_AERONAVES) break;

        ++contador; contador = contador % QTD_AERONAVES;
    
        // Se todas as aeronaves tiverem tido "sua vez", calcula novamente a ordem de prioridade
        if(contador == 0) calculaPrioridade(aeronaves, indices_ordenados);

    }

    flag_fecha_thread = 1;
    printf("\n🥳 Todas as aeronaves pousaram. Encerrando programa. 🥳\n");
    imprimeResultados();

    // Libera o necessário
    shmdt(aeronaves);
    shmctl(segmento_memoria, IPC_RMID, NULL);

    return 0;
}

// Retorna o índice do vetor pistas onde está uma pista passada como parâmetro
int buscaIndicePista(int num_pista){
    for(int i=0; i<QTD_PISTAS; i++){
        if (num_pista == pistas[i].num) return i;
    }
    return -1;
}

// Ordena array_indices com base na distância das aeronaves à pista de pouso
void calculaPrioridade(Aeronave *aeronaves, int *array_indices){

    Ponto destino = {0.5, 0.5};
    float distancia[QTD_AERONAVES], dx[QTD_AERONAVES], dy[QTD_AERONAVES];
    
    // Calcula a distância de cada aeronave à pista de pouso
    for(int i=0; i<QTD_AERONAVES; i++){

        dx[i] = aeronaves[i].ponto.x - destino.x;
        dy[i] = aeronaves[i].ponto.y - destino.y;

        distancia[i] = sqrt( pow(dx[i],2) + pow(dy[i],2) );
    }

    // Bubble sort com base na distância
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

        // Erro
        if(pids[i]<0){ perror("Erro no fork"); exit(1); }

        // Filho
        else if(pids[i]==0){ 

            // Transforma de inteiro para string
            sprintf(str_segmento_memoria, "%d", *segmento_memoria);
            sprintf(str_indice_aeronave, "%d", i);

            // Executa o programa responsável pelas aeronaves
            execlp("./aeronave", "aeronave", str_segmento_memoria, str_indice_aeronave, NULL);
            exit(1);
        }

        // Dá um tempo entre a criação de uma aeronave e outra (apenas para melhor visualização)
        sleep(1);
    }
}

// Garante que as aeronaves fiquem na pista mais vazia possível
void controlePistas(Aeronave *aeronaves, int *pids){

    int indice_pista, indice_pista_secundaria, pista_secundaria;

    for(int i=0; i<QTD_AERONAVES; i++){

        // Procura o índice da pista de preferência da aeronave no vetor pistas
        indice_pista = buscaIndicePista(aeronaves[i].pista_preferida);
        if (indice_pista == -1) perror("Aeronave deseja pousar em uma pista inexistente");

        // Se a pista já estiver ocupada
        if (pistas[indice_pista].ocupacao != 0){

            // Procura uma pista alterntiva
            pista_secundaria = alteraPista(aeronaves[i].pista_preferida);
            indice_pista_secundaria = buscaIndicePista(pista_secundaria);

            // Se a outra pista tiver menos ocupada, envia um sinal solicitando a troca
            if (pistas[indice_pista_secundaria].ocupacao < pistas[indice_pista].ocupacao){
                printf("\n❗ Solicitando que a aeronave %d (pista %d) troque de pista ❗\n", aeronaves[i].id, aeronaves[i].pista_preferida);
                kill(pids[i], SIGCONT);
                kill(pids[i], SIGUSR2);
                sleep(1); // Dá um tempo para aeronave.c aplicar a mudança de pista
                kill(pids[i], SIGSTOP);

                // Confere se a pista foi alterada
                if (aeronaves[i].pista_preferida != pistas[indice_pista_secundaria].num) perror("Avião não mudou de pista quando solicitado");
            
                pistas[indice_pista_secundaria].ocupacao++;
            }

            // Se a outra pista não estiver menos ocupada, a aeronave continua na mesma pista
            else{
                pistas[indice_pista].ocupacao++;
            }
        }

        // Se a pista estiver vazia, a aeronave continua na nela
        else{
            pistas[indice_pista].ocupacao++;
        }
    }
}

// Confere se a aeronave no índice i tem risco de colidir com outras aeronaves
// Retornos -> 0 (aeronave não foi finalizada) 1 (aeronave foi finalizada)
int controleColisao(Aeronave *aeronaves, int i, int *pids){

    float distancia_x, distancia_y, x_projetado, y_projetado;

    // Guarda quantas aeronaves estão voando na mesma direção
    int voando_mesma_direcao = 0;

    // Guarda de quantas aeronaves a aeronave "principal" não tem risco de colidir
    int livre_de_colisao = 0;

    for(int j=0; j<QTD_AERONAVES; j++){

        /*
        Não confere se há risco de colisão se
        - forem a mesma aeronave
        - a outra aeronave não estiver ativamente no espaço aéreo (voando ou parada)
        - as aeronaves estiverem em lados diferentes
        */
        if (i == j || (aeronaves[j].status != VOANDO && aeronaves[j].status != AGUARDANDO) || aeronaves[i].direcao != aeronaves[j].direcao) continue;
        
        voando_mesma_direcao++;

        // Calcula a distância entre as aeronaves baseado na posição atual de ambas
        distancia_x = fabs(aeronaves[j].ponto.x - aeronaves[i].ponto.x);
        distancia_y = fabs(aeronaves[j].ponto.y - aeronaves[i].ponto.y);

        // Identifica colisão eminente entre aeronaves e ordena que a aeronave "principal" seja remetida
        if (distancia_x < 0.1 && distancia_y < 0.1){
            printf("\n🚫 Colisão eminente entre aeronaves %d [%.2f, %.2f] e %d [%.2f, %.2f]. Ordenando que a aeronave %d remeta o pouso 🚫\n", i, aeronaves[i].ponto.x, aeronaves[i].ponto.y, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y, i);
            
            kill(pids[i], SIGKILL);
            aeronaves[i].status = REMETIDA;

            return 1;
        }

        // Projeta a posição futura da aeronave "principal"
        x_projetado = movimentaX(&aeronaves[i]);
        y_projetado = movimentaY(&aeronaves[i]);

        // Calcula a distância entre as aeronaves baseado na posição futura da "principal"
        distancia_x = fabs(aeronaves[j].ponto.x - x_projetado);
        distancia_y = fabs(aeronaves[j].ponto.y - y_projetado);

        // Identifica potencial de colisão futura e ordena que a aeronave "principal" reduza sua velocidade
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

        // Se não houver risco de colisão com uma aeronave
        else if (!(distancia_x < 0.1 && distancia_y < 0.1) && aeronaves[i].status == AGUARDANDO){ 
            livre_de_colisao++; 
        }
    }

    // Se não tiverem mais aeronaves do mesmo lado do espaço aéreo e aeronave estiver parada, ordena que ela continue
    if(voando_mesma_direcao == 0 && aeronaves[i].status == AGUARDANDO){
        printf("\n🆗 Não há mais potencial de colisão para a aeronave %d. Ordenando aumento da velocidade 🆗\n", aeronaves[i].id);
        
        kill(pids[i], SIGCONT);
        kill(pids[i], SIGUSR1);
        sleep(1);
        kill(pids[i], SIGSTOP);

        // Confere se o avião acelerou
        if (aeronaves[i].status != VOANDO) perror("Avião não acelerou quando solicitado"); 
    }

    // Se a aeronave não for colidir nenhuma outra voando na mesma direção, ordena que ela continue
    else if (livre_de_colisao == voando_mesma_direcao && voando_mesma_direcao > 0){
        printf("\n⚠️ Não há mais potencial de colisão para a aeronave %d. Ordenando aumento da velocidade ⚠️\n", aeronaves[i].id);
                
        kill(pids[i], SIGCONT);
        kill(pids[i], SIGUSR1);
        sleep(1);
        kill(pids[i], SIGSTOP);

        // Confere se o avião acelerou
        if (aeronaves[i].status != VOANDO) perror("Avião não acelerou quando solicitado");
    }

    return 0;
}

// Confere se as aeronaves estão "engavetadas" (paradas mas caso se movimentem irão colidir)
// Retornos -> 0 (nenhuma aeronave remetida) 1 (alguma aeronave foi remetida)
int controleEngavetamento(Aeronave *aeronaves, int *pids){

    // Guarda a quantidade de aeronaves que estão paradas porque se bateriam caso houvesse movimento
    int bloqueados = 0;
    float distancia_x, distancia_y;

    for (int i = 0; i < QTD_AERONAVES; i++) {

        // Descarta a aeronave se ela não estiver espernado liberação
        if (aeronaves[i].status != AGUARDANDO) continue;

        for (int j = 0; j < QTD_AERONAVES; j++) {

            // Descarta a comparação com outra aeronave se ela não estiver nem voando, nem parada
            if (i == j || (aeronaves[j].status != VOANDO && aeronaves[j].status != AGUARDANDO) ) continue;

            // Projeta a posição futura da aeronave "principal"
            float x_projetado = movimentaX(&aeronaves[i]);
            float y_projetado = movimentaY(&aeronaves[i]);

            // Calcula a distância entre as aeronaves baseado na posição futura da "principal"
            distancia_x = fabs(aeronaves[j].ponto.x - x_projetado);
            distancia_y = fabs(aeronaves[j].ponto.y - y_projetado);

            // Identifica potencial de colisão futura e "guarda" esse fato
            if (distancia_x < 0.1 && distancia_y < 0.1) { bloqueados++; break; }
        }
    }

    // Se só falterem essas aeronaves pousarem
    if (bloqueados > 1 && bloqueados + processos_finalizados == QTD_AERONAVES){
        printf("\n☢️ Emergência: aviões engavetados. Será necessário que 1 deles remeta o pouso. ☢️\n");
        // Ordena que a primeira aeronave encontrada remeta o pouso
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

// Verifia se uma aeronave vai bater com alguma outra se ela entrar no espaço aéreo
// Retornos -> 0 (nenhuma aeronave remetida) 1 (alguma aeronave foi remetida)
int verificaEntrada(Aeronave *aeronaves, int i, int *pids){

    float distancia_x, distancia_y;

    for(int j=0; j<QTD_AERONAVES; j++){

        /*
        Não confere se há risco de colisão se
        - forem a mesma aeronave
        - a outra aeronave não estiver ativamente no espaço aéreo (voando ou parada)
        - as aeronaves estiverem em lados diferentes
        */
       if (i == j || (aeronaves[j].status != VOANDO && aeronaves[j].status != AGUARDANDO) || aeronaves[i].direcao != aeronaves[j].direcao) continue;

        // Calcula a distância entre as aeronaves baseado na posição atual de ambas
        distancia_x = fabs(aeronaves[j].ponto.x - aeronaves[i].ponto.x);
        distancia_y = fabs(aeronaves[j].ponto.y - aeronaves[i].ponto.y);

        // Identifica colisão eminente entre aeronaves e ordena que a aeronave que ainda não entrou no espaço aéreo seja remetida
        if (distancia_x < 0.1 && distancia_y < 0.1){
            printf("\n🚫 Entrada negada a aeronave %d [%.2f, %.2f]. Risco de colisão com e %d [%.2f, %.2f] 🚫\n", i, aeronaves[i].ponto.x, aeronaves[i].ponto.y, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y);
            
            kill(pids[i], SIGKILL);
            aeronaves[i].status = REMETIDA;

            return 1;
        }
    }

    return 0;
} 

// Executa os procedimentos necessários quando uma aeronave pousa
void formalizaPouso(Aeronave* aeronave){
    processos_finalizados++; 
    printf("\n💭 %d processos finalizados\n", processos_finalizados);
    int indice_pista = buscaIndicePista(aeronave->pista_preferida);
    pistas[indice_pista].ocupacao--;   
}

void imprimeResultados(void){
    int finalizado, remetida;
    finalizado = remetida = 0;

    for(int i=0; i<QTD_AERONAVES; i++){
        if(aeronaves[i].status == FINALIZADO) finalizado++;
        else if(aeronaves[i].status == REMETIDA) remetida++;
    }

    printf(">> %d pousaram com sucesso\n", finalizado);
    printf(">> %d foram remetidas\n", remetida);
}

void* interface(void* arg) {

    char comando[50];
    int id;

    while (1) {

        if (flag_fecha_thread) break;

        if (!flag_interface){
            char entrada = getchar();
    
            /*
            // Limpa o buffer caso ainda haja '\n' sobrando
            while (entrada != '\n' && entrada != EOF) {
                entrada = getchar();
            }
            */

            if (entrada == '\n') {
                flag_interface = 1;
            }
        }

        if (flag_interface){
            printf("\n📖 Comandos disponíveis:\n");
            printf("  status          → mostra todas as informações das aeronaves\n");
            printf("  iniciar <id>    → inicia o vôo de uma aeronave\n");
            printf("  pausar <id>     → pausa o vôo de uma aeronave\n");
            printf("  retomar <id>    → retoma o vôo de uma aeronave\n");
            printf("  finalizar <id>  → finaliza o vôo de uma aeronave\n");
            printf("  sair            → encerra a interface de comandos\n");

            // Obtém o comando do usuário
            printf("\n📡 Comando > ");
            fgets(comando, sizeof(comando), stdin);
            comando[strcspn(comando, "\n")] = '\0'; // remove \n

            // Status
            if (strncmp(comando, "status", 6) == 0) {
                printf("\n📋 Status das aeronaves:\n");
                for (int i = 0; i < QTD_AERONAVES; i++) { 
                    imprimeAeronave(&aeronaves[i]); 
                }
            }
            // Iniciar
            else if (sscanf(comando, "iniciar %d", &id) == 1) {
                if (aeronaves[id].status != VOANDO){ 
                    aeronaves[id].status = VOANDO; 
                    printf("▶️ Aeronave %d iniciada.\n", id); 
                }
                else { 
                    printf("▶️ Atenção! A aeronave %d já havia sido iniciada.\n", id); 
                }
            }
            // Pausar
            else if (sscanf(comando, "pausar %d", &id) == 1) {
                if(aeronaves[id].status != AGUARDANDO){ 
                    aeronaves[id].status = AGUARDANDO; 
                    printf("⏸️ Aeronave %d pausada.\n", id); 
                }
                else { 
                    printf("⏸️ Atenção! A aeronave %d já estava pausada.\n", id); 
                }
            }
            // Retomar
            else if (sscanf(comando, "retomar %d", &id) == 1) {
                if (aeronaves[id].status != VOANDO){ 
                    aeronaves[id].status = VOANDO; 
                    printf("▶️ Aeronave %d retomada.\n", id); 
                }
                else {
                    printf("▶️ Atenção! A aeronave %d já estava em execução.\n", id); 
                }
            }
            // Finalizar
            else if (sscanf(comando, "finalizar %d", &id) == 1) {
                if (aeronaves[id].status == FINALIZADO){ printf("💀 Atenção! Não é possível finalizar a aeronave %d porque ela já pousou.\n", id); }
                else if(aeronaves[id].status != REMETIDA){ 
                    kill(aeronaves[id].pid, SIGKILL);
                    aeronaves[id].status = REMETIDA;
                    formalizaPouso(&aeronaves[id]);
                    printf("💀 Aeronave %d finalizada.\n", id);
                }
                else{
                    printf("💀 Atenção! A aeronave %d já foi finalizada.\n", id);
                }
            }
            // Sair
            else if (strncmp(comando, "sair", 4) == 0) {
                printf("⛔ Encerrando interface de comandos...\n");
                flag_interface = 0;
                continue;
            }
            // Default
            else {
                printf("❌ Comando inválido. Use: status | iniciar <id> | pausar <id> | retomar <id> | finalizar <id> | sair\n");
            }
        }
    }

    return NULL;
}