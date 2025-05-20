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
#include <pthread.h> // pthread

// Arquivo header
#include "aux.h"

// Estruturas personalizadas do trabalho
typedef struct Aeronave Aeronave;
typedef struct Pista Pista;
typedef struct Ponto Ponto;

// Funções do módulo
int buscaIndicePista(int num_pista);
void calculaPrioridade();
void criaAeronaves();
void controlePistas();
int controleColisao(int i);
int controleEngavetamento();
int verificaEntrada(int i);
void formalizaFim(Aeronave* aeronave);
void imprimeResultados();
void* interface(void* arg);

// Variáveis globais do módulo
Aeronave *aeronaves = NULL;
int pids[QTD_AERONAVES];
Pista pistas[QTD_PISTAS] = { {3, 0}, {6, 0}, {18, 0}, {27, 0} };
int processos_finalizados = 0;
int bloqueados;
int indices_ordenados[QTD_AERONAVES];
pthread_t controller_thread;
int flag_interface = 0;
int flag_fecha_thread = 0;
int segmento_memoria;

int main(void){

    printf("🌐 Entre com ENTER para abrir o terminal.\n\n");

    //Cria um segmento de memória compartilhada, reservado para armazenar um array de Aeronave
    segmento_memoria = shmget(IPC_PRIVATE, sizeof(Aeronave)*QTD_AERONAVES, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    //Se shmget falhar, imprime um erro e encerra o programa com código de saída 1.
    if (segmento_memoria == -1){ perror("Erro na criação de memória compartilhada"); return 1; }

    // Ponteiro para o segmento de memória
    // Conecta o processo ao segmento de memória compartilhada criado anteriormente. Retorna um ponteiro para a área
    aeronaves = (Aeronave*)shmat(segmento_memoria, 0, 0);
    // Verifica se o shmat falhou. Se sim, imprime erro e encerra.
    if (aeronaves == (void *)-1) { perror("Erro no shmat"); exit(1); }

    // Criando múltiplos processos (aeronaves)
    criaAeronaves();

    // Cria uma thread separada que executa a função interface
    // Responsável por capturar comandos do usuário (como “pausar”, “retomar” ou “status”).
    pthread_create(&controller_thread, NULL, interface, NULL);

    // Após a criação das aeronaves, o controller pausa todas elas enviando o sinal SIGSTOP para cada processo.
    printf("\n⚠️ Ordenando a pausa de todas as aeronaves ⚠️\n");
    for(int i=0; i<QTD_AERONAVES; i++) kill(pids[i], SIGSTOP); 

    //Verifica se há conflitos de pista entre as aeronaves. 
    //Se duas aeronaves querem a mesma pista, uma delas pode receber um sinal para trocar (via SIGUSR2)
    controlePistas();

    // Ordem de execução dos processos com base na distância da aeornave ao destino
    // Inicializa o vetor de prioridade (indices_ordenados) com os índices das aeronaves
    //Em seguida, a função calculaPrioridade() 
    //ordena esses índices com base na distância de cada aeronave até o ponto central de pouso (0.5, 0.5)
    for(int i=0; i<QTD_AERONAVES; i++) indices_ordenados[i] = i;
    calculaPrioridade();

    // Escalonamento Round-Robin
    int i, contador = 0;

    // Efeito unicamente visual: para dar uma pausa antes da enxurrada de impressões
    sleep(3);

    // Gerenciamento do momento de ativação de uma aeronave
    // Variáveis para medir o tempo desde o início da simulação
    // Cada aeronave tem um delay aleatório antes de poder iniciar seu voo.
    int delay_check;
    time_t inicioVoos = time(NULL);
    time_t agora;

    // Loop principal. Roda até todas as aeronaves pousarem ou forem remetidas
    while(1){

        // Se a interface estiver sendo usada (flag ligada), o controlador espera e não interfere.
        if(flag_interface) continue;

        // Pega o índice da aeronave a ser processada com base na ordem de prioridade.
        i = indices_ordenados[contador];

        // Se a aeronave ainda não teve sua entrada no espaço aéreo permitida
        if(aeronaves[i].status == DELAY){

            // Calcula quanto tempo se passou desde que o programa foi iniciado
            agora = time(NULL);
            delay_check = (int)(agora-inicioVoos);

            // Se o tempo de espera da aeronave tiver sido ultrapassado, ela entra oficialmente no espaço aéreo
            if (delay_check > aeronaves[i].delay){

                aeronaves[i].status = VOANDO;

                // Se a aeronave tiver entrada permitida
                if (!verificaEntrada(i)){ printf("\n☑️ A aeronave %d teve sua entrada permitida no espaço aéreo. ☑️\n", aeronaves[i].id); }
                
                // Se a aeronave não tiver a entrada permitida (colisão eminente)
                else{ formalizaFim(&aeronaves[i]); }
            }
        }

        // Se a aeronave não está voando nem aguardando (por exemplo, já pousou), ela é ignorada nesta rodada
        if(aeronaves[i].status != VOANDO && aeronaves[i].status != AGUARDANDO ){ 
            ++contador; contador = contador % QTD_AERONAVES; 
            continue; 
        }

        // Executa os algoritmos de segurança. Se houver colisão iminente ou engavetamento, a aeronave é removida da simulação
        if (controleColisao(i) || controleEngavetamento()){ formalizaFim(&aeronaves[i]); }

        // Se a aeronave tiver permissão para continuar
        // Dá permissão para a aeronave se mover (SIGCONT), espera 1s para ela mudar de posição, e em seguida pausa (SIGSTOP)
        if (aeronaves[i].status == VOANDO){
            kill(pids[i], SIGCONT);
            sleep(1); // Dá um tempo para aeronave.c aplicar as mudanças de posição
            kill(pids[i], SIGSTOP);
        }

        // Se a aeronave tiver pousado com sucesso
        // Se a aeronave chegou ao ponto central, o controlador atualiza os contadores e libera a pista
        if(aeronaves[i].status == FINALIZADO){ formalizaFim(&aeronaves[i]); }

        // Se todas as aeronaves pousaram ou foram removidas, o loop termina
        if(processos_finalizados == QTD_AERONAVES) break;

        //Avança para a próxima aeronave no vetor de prioridade. Se chegar ao fim, volta para o começo
        ++contador; contador = contador % QTD_AERONAVES;
    
        // Se todas as aeronaves tiverem tido "sua vez", calcula novamente a ordem de prioridade com base na nova posição.
        if(contador == 0) calculaPrioridade();

    }

    //Encerra a simulação, imprime quantas aeronaves pousaram com sucesso e quantas foram remetidas.
    flag_fecha_thread = 1;
    printf("\n🥳 Todas as aeronaves pousaram. Encerrando programa. 🥳\n");
    imprimeResultados();

    // Desanexa a memória compartilhada e em seguida remove o segmento da memória do sistema.
    shmdt(aeronaves);
    shmctl(segmento_memoria, IPC_RMID, NULL);

    return 0;
}

// Função que recebe um número de pista (num_pista) e retorna o índice correspondente no vetor global pistas
int buscaIndicePista(int num_pista){
    //Percorre o vetor pistas
    for(int i=0; i<QTD_PISTAS; i++){
        //Se encontrar uma pista com o número igual ao parâmetro, retorna o índice i
        if (num_pista == pistas[i].num) return i;
    }
    return -1;
}

// Essa função ordena o vetor indices_ordenados com base na distância de cada aeronave até o destino final (ponto [0.5, 0.5]).
void calculaPrioridade(){
    // Define o destino padrão para pouso das aeronaves.
    Ponto destino = {0.5, 0.5};
    float distancia[QTD_AERONAVES], dx[QTD_AERONAVES], dy[QTD_AERONAVES];
    
    // Calcula a distância de cada aeronave à pista de pouso
    // Loop para calcular distância para cada aeronave.
    for(int i=0; i<QTD_AERONAVES; i++){
        
        // Calcula diferença de posição no eixo X e no eixo Y
        dx[i] = aeronaves[i].ponto.x - destino.x;
        dy[i] = aeronaves[i].ponto.y - destino.y;

        // Usa a fórmula da distância euclidiana para medir a distância total.
        distancia[i] = sqrt( pow(dx[i],2) + pow(dy[i],2) );
    }

    // Bubble sort com base na distância
    // Ordenar indices_ordenados de forma crescente
    int temp;
    for (int i = 0; i < QTD_AERONAVES - 1; i++) {
        for (int j = 0; j < QTD_AERONAVES - 1 - i; j++) {
            //Troca as posições se uma aeronave estiver mais distante que a seguinte.
            if (distancia[indices_ordenados[j]] > distancia[indices_ordenados[j + 1]]) {
                temp = indices_ordenados[j];
                indices_ordenados[j] = indices_ordenados[j + 1];
                indices_ordenados[j + 1] = temp;
            }

        }
    }

    //Imprime a ordem final
    printf("\n🕐 Ordem de prioridade das aeronaves: ");
    for(int i=0; i<QTD_AERONAVES; i++) printf("%d ", indices_ordenados[i]);
    printf(" 🕐\n");
}

//Declara strings para passar como argumentos ao processo aeronave
void criaAeronaves(){
    char str_segmento_memoria[20], str_indice_aeronave[20];

    //Cria todos os processos de aeronaves
    for (int i=0; i<QTD_AERONAVES; i++){

        // Cria um processo filho. O PID é salvo no vetor pids
        pids[i] = fork();

        //  Se fork() falhar, imprime erro e encerra o programa
        if(pids[i]<0){ perror("Erro no fork"); exit(1); }

        // Filho
        else if(pids[i]==0){ 

            // Converte os argumentos segmento_memoria e i para strings
            sprintf(str_segmento_memoria, "%d", segmento_memoria);
            sprintf(str_indice_aeronave, "%d", i);

            //Substitui o processo filho por um novo executável: aeronave, passando os argumentos. Se falhar, sai com erro.
            execlp("./aeronave", "aeronave", str_segmento_memoria, str_indice_aeronave, NULL);
            exit(1);
        }

        // Dá um tempo entre a criação de uma aeronave e outra (apenas para melhor visualização)
        sleep(1);
    }
}

// Garante que as aeronaves fiquem na pista mais vazia possível
void controlePistas(){

    //Variáveis para armazenar os índices e número das pistas principais/secundárias
    int indice_pista, indice_pista_secundaria, pista_secundaria;

    //Processa cada aeronave
    for(int i=0; i<QTD_AERONAVES; i++){

        // Procura o índice da pista de preferência da aeronave no vetor pistas
        indice_pista = buscaIndicePista(aeronaves[i].pista_preferida);
        if (indice_pista == -1) perror("Aeronave deseja pousar em uma pista inexistente");

        // Se a pista já estiver ocupada
        // Verifica se já tem outra aeronave querendo a mesma pista.
        if (pistas[indice_pista].ocupacao != 0){

            // Procura uma pista alterntiva
            pista_secundaria = alteraPista(aeronaves[i].pista_preferida);
            indice_pista_secundaria = buscaIndicePista(pista_secundaria);

            // Se a outra pista tiver menos ocupada, envia um sinal solicitando a troca para ela
            if (pistas[indice_pista_secundaria].ocupacao < pistas[indice_pista].ocupacao){
                //Envia sinais para que a aeronave retome e altere a pista. Pausa após para não correr risco de conflito.
                printf("\n❗ Solicitando que a aeronave %d (pista %d) troque de pista ❗\n", aeronaves[i].id, aeronaves[i].pista_preferida);
                kill(pids[i], SIGCONT);
                kill(pids[i], SIGUSR2);
                sleep(1); // Dá um tempo para aeronave.c aplicar a mudança de pista
                kill(pids[i], SIGSTOP);

                // Garante que a troca foi feita corretamente
                if (aeronaves[i].pista_preferida != pistas[indice_pista_secundaria].num) perror("Avião não mudou de pista quando solicitado");
                //Atualiza ocupação da pista secundária
                pistas[indice_pista_secundaria].ocupacao++;
            }

            // Se a outra pista não estiver menos ocupada, a aeronave continua na mesma pista
            else{
                pistas[indice_pista].ocupacao++;
            }
        }

        // Se a pista estiver vazia desde o início, mantém nela e marca como ocupada
        else{
            pistas[indice_pista].ocupacao++;
        }
    }
}

// Confere se a aeronave no índice i tem risco de colidir com outras aeronaves
//Se houver colisão eminente, a aeronave é "remetida" (finalizada)
// Retornos -> 0 (aeronave não foi finalizada) 1 (aeronave foi finalizada)
int controleColisao(int i){

   //Declara variáveis para armazenar distâncias entre aeronaves e posições projetadas (para prever colisões). 
    float distancia_x, distancia_y, x_projetado, y_projetado;

    // Guarda quantas aeronaves estão voando na mesma direção
    int voando_mesma_direcao = 0;

    // Guarda de quantas aeronaves a aeronave "principal" não tem risco de colidir
    int livre_de_colisao = 0;

    // Loop para comparar a aeronave i com todas as outras j
    for(int j=0; j<QTD_AERONAVES; j++){

        /*
        Não confere se há risco de colisão se
        - forem a mesma aeronave
        - a outra aeronave não estiver ativamente no espaço aéreo (voando ou parada)
        - as aeronaves estiverem em lados diferentes
        */
        if (i == j || (aeronaves[j].status != VOANDO && aeronaves[j].status != AGUARDANDO) || aeronaves[i].direcao != aeronaves[j].direcao) continue;
        
        //Conta aeronave válida (mesma direção e ativa)
        voando_mesma_direcao++;

        // Calcula a distância entre as aeronaves baseado na posição atual de ambas
        distancia_x = fabs(aeronaves[j].ponto.x - aeronaves[i].ponto.x);
        distancia_y = fabs(aeronaves[j].ponto.y - aeronaves[i].ponto.y);

        // Identifica colisão eminente entre aeronaves e ordena que a aeronave "principal" seja remetida
        if (distancia_x < 0.1 && distancia_y < 0.1){
            //Imprime aviso, finaliza o processo i, marca status como remetido e retorna 1
            printf("\n🚫 Colisão eminente entre aeronaves %d [%.2f, %.2f] e %d [%.2f, %.2f]. Ordenando que a aeronave %d remeta o pouso 🚫\n", i, aeronaves[i].ponto.x, aeronaves[i].ponto.y, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y, i);

            kill(pids[i], SIGKILL);
            aeronaves[i].status = REMETIDA;

            return 1;
        }

        // Projeta a posição futura da aeronave "principal". Calcula onde a aeronave i vai estar no próximo passo
        x_projetado = movimentaX(&aeronaves[i]);
        y_projetado = movimentaY(&aeronaves[i]);

        // Calcula a distância entre as aeronaves baseado na posição futura da "principal". Calcula a distância prevista para o futuro
        distancia_x = fabs(aeronaves[j].ponto.x - x_projetado);
        distancia_y = fabs(aeronaves[j].ponto.y - y_projetado);

        // Identifica potencial de colisão futura e ordena que a aeronave "principal" reduza sua velocidade
        //Se a distância futura indicar colisão, e a aeronave estiver voando:
        if (distancia_x < 0.1 && distancia_y < 0.1 && aeronaves[i].status == VOANDO){
            // Envia sinais para "pausar" o avião (mudar status), verifica se funcionou.
            printf("\n⚠️ Potencial de colisão identificado entre aeronaves %d [%.2f, %.2f]->[%.2f, %.2f] e %d [%.2f, %.2f]. Ordenando redução da aeronave %d ⚠️\n", i, aeronaves[i].ponto.x, aeronaves[i].ponto.y, x_projetado, y_projetado, j, aeronaves[j].ponto.x, aeronaves[j].ponto.y, i);

            kill(pids[i], SIGCONT);
            kill(pids[i], SIGUSR1); // pausa aeronave
            sleep(1);
            kill(pids[i], SIGSTOP);

            // Confere se o avião desacelerou
            if (aeronaves[i].status != AGUARDANDO) perror("Aeronave não desacelerou quando solicitado");

            break;
        }

        // Se não houver risco de colisão com uma aeronave contabiliza como "livre"
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
// Essa função detecta se várias aeronaves estão paradas porque não conseguem se mover sem colidir
// Se isso acontecer, uma delas será remetida para resolver o bloqueio
// Retornos -> 0 (nenhuma aeronave remetida) 1 (alguma aeronave foi remetida)
int controleEngavetamento(){

    // Guarda a quantidade de aeronaves que estão paradas porque se bateriam caso houvesse movimento
    int bloqueados = 0;
    float distancia_x, distancia_y;

    //Analisa apenas as aeronaves paradas aguardando
    for (int i = 0; i < QTD_AERONAVES; i++) {

        // Descarta a aeronave se ela não estiver espernado liberação
        if (aeronaves[i].status != AGUARDANDO) continue;

        //Compara com outras que estejam voando ou aguardando também
        for (int j = 0; j < QTD_AERONAVES; j++) {

            // Descarta a comparação com outra aeronave se ela não estiver nem voando, nem parada
            if (i == j || (aeronaves[j].status != VOANDO && aeronaves[j].status != AGUARDANDO) ) continue;

            // Projeta a posição futura da aeronave "principal". Calcula onde i vai estar se ela se mover
            float x_projetado = movimentaX(&aeronaves[i]);
            float y_projetado = movimentaY(&aeronaves[i]);

            // Calcula a distância entre as aeronaves baseado na posição futura da "principal"
            // Se tiver risco de colisão, conta como bloqueada e sai do loop interno.
            distancia_x = fabs(aeronaves[j].ponto.x - x_projetado);
            distancia_y = fabs(aeronaves[j].ponto.y - y_projetado);

            // Identifica potencial de colisão futura e "guarda" esse fato
            if (distancia_x < 0.1 && distancia_y < 0.1) { bloqueados++; break; }
        }
    }

    //  Se só faltarem essas aeronaves bloqueadas pousarem, então há um "engarrafamento".
    if (bloqueados > 1 && bloqueados + processos_finalizados == QTD_AERONAVES){
        printf("\n☢️ Emergência: aviões engavetados. Será necessário que 1 deles remeta o pouso. ☢️\n");
        // A primeira aeronave aguardando é escolhida para ser "remetida" e liberar espaço
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

// Verifia se a aeronave de indice i vai bater com alguma outra se ela entrar no espaço aéreo
// Retornos -> 0 (nenhuma aeronave remetida) 1 (alguma aeronave foi remetida)
int verificaEntrada(int i){

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
            //Mata o processo da aeronave e marca ela como remetida, ou seja, não pôde entrar no espaço aéreo
            kill(pids[i], SIGKILL);
            aeronaves[i].status = REMETIDA;

            return 1;
        }
    }

    return 0;
} 

// Executa os procedimentos necessários quando uma aeronave pousa
void formalizaFim(Aeronave* aeronave){
    //Encrementa o contador global de quantas aeronaves já terminaram (pousaram ou foram remetidas).
    processos_finalizados++; 
    //Imprime o número atualizado.
    printf("\n💭 %d processos finalizados\n", processos_finalizados);
    //Localiza o índice da pista que a aeronave utilizou e reduz a contagem de aeronaves nessa pista, liberando-a.
    int indice_pista = buscaIndicePista(aeronave->pista_preferida);
    pistas[indice_pista].ocupacao--;   
}

void imprimeResultados(){
    //Inicializa os contadores das duas categorias de fim de voo: pouso e remessa.
    int finalizado, remetida;
    finalizado = remetida = 0;
    //Percorre todas as aeronaves e contabiliza quantas: pousaram com sucesso (FINALIZADO) e quantas foram removidas por risco (REMETIDA
    for(int i=0; i<QTD_AERONAVES; i++){
        if(aeronaves[i].status == FINALIZADO) finalizado++;
        else if(aeronaves[i].status == REMETIDA) remetida++;
    }

    printf(">> %d pousaram com sucesso\n", finalizado);
    printf(">> %d foram remetidas\n", remetida);
}

//Essa função roda em uma thread separada e oferece uma interface interativa via terminal para o usuário
void* interface(void* arg)

    //texto digitado pelo usuário e variável auxiliar id
    char comando[50];
    int id;

    //Loop infinito que só termina quando a flag de finalização for ativada
    while (1) {

        // Se todas as aeronaves tiverem pousado, quebra o loop e termina a thread
        if (flag_fecha_thread) break;

        // Se o usuário não tiver ordenado a exibição da interface. Aguarda o usuário apertar ENTER para iniciar a interface.
        if (!flag_interface){
            char entrada = getchar();
            if (entrada == '\n') { flag_interface = 1; }
        }

        // Se o usuário tiver ordenado a exibição da interface
        if (flag_interface){

            sleep(1); // Espera que as ações do loop principal sejam concluídas

            printf("\n📖 Comandos disponíveis:\n");
            printf("  status          → mostra todas as informações das aeronaves\n");
            printf("  iniciar <id>    → inicia o vôo de uma aeronave\n");
            printf("  pausar <id>     → pausa o vôo de uma aeronave\n");
            printf("  retomar <id>    → retoma o vôo de uma aeronave\n");
            printf("  finalizar <id>  → finaliza o vôo de uma aeronave\n");
            printf("  sair            → encerra a interface de comandos\n");

            // Lê o comando digitado e remove o \n no final
            printf("\n📡 Comando > ");
            fgets(comando, sizeof(comando), stdin);
            comando[strcspn(comando, "\n")] = '\0'; // remove \n

            // Status: Mostra todas as informações de todas as aeronaves.
            if (strncmp(comando, "status", 6) == 0) {
                printf("\n📋 Status das aeronaves:\n");
                for (int i = 0; i < QTD_AERONAVES; i++) { 
                    imprimeAeronave(&aeronaves[i]); 
                }
            }
            // Inicia id: Ativa uma aeronave manualmente se ainda não estiver voando
            else if (sscanf(comando, "iniciar %d", &id) == 1) {
                if (aeronaves[id].status != VOANDO){ 
                    aeronaves[id].status = VOANDO; 
                    printf("▶️ Aeronave %d iniciada.\n", id); 
                }
                else { 
                    printf("▶️ Atenção! A aeronave %d já havia sido iniciada.\n", id); 
                }
            }
            // Pausar: funciona igual ao iniciar, mas alteram o status da aeronave para AGUARDANDO (pausar) ou VOANDO (retomar)
            else if (sscanf(comando, "pausar %d", &id) == 1) {
                if(aeronaves[id].status != AGUARDANDO){ 
                    aeronaves[id].status = AGUARDANDO; 
                    printf("⏸️ Aeronave %d pausada.\n", id); 
                }
                else { 
                    printf("⏸️ Atenção! A aeronave %d já estava pausada.\n", id); 
                }
            }
            // Retomar: funciona igual ao iniciar, mas alteram o status da aeronave para AGUARDANDO (pausar) ou VOANDO (retomar).
            else if (sscanf(comando, "retomar %d", &id) == 1) {
                if (aeronaves[id].status != VOANDO){ 
                    aeronaves[id].status = VOANDO; 
                    printf("▶️ Aeronave %d retomada.\n", id); 
                }
                else {
                    printf("▶️ Atenção! A aeronave %d já estava em execução.\n", id); 
                }
            }
            // Finalizar id: Força o encerramento de uma aeronave, matando seu processo e atualizando o status
            else if (sscanf(comando, "finalizar %d", &id) == 1) {
                if (aeronaves[id].status == FINALIZADO){ printf("💀 Atenção! Não é possível finalizar a aeronave %d porque ela já pousou.\n", id); }
                else if(aeronaves[id].status != REMETIDA){ 
                    kill(aeronaves[id].pid, SIGKILL);
                    aeronaves[id].status = REMETIDA;
                    formalizaFim(&aeronaves[id]);
                    printf("💀 Aeronave %d finalizada.\n", id);
                }
                else{
                    printf("💀 Atenção! A aeronave %d já foi finalizada.\n", id);
                }
            }
            // Sair: Fecha a interface e volta o programa ao estado automático
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