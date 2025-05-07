#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "aux.h"

void* comandoUsuario(void* arg) {
    Aeronave* aeronaves = (Aeronave*) arg;
    char comando[50];
    int id;

    while (1) {
        printf("\n📖 Comandos disponíveis:\n");
        printf("  status         → mostra posição, direção e status de todas as aeronaves\n");
        printf("  pause <id>     → pausa a aeronave com o ID informado\n");
        printf("  resume <id>    → retoma a aeronave com o ID informado\n");
        printf("  kill <id>      → finaliza (remete) a aeronave com o ID informado\n");
        printf("  sair           → encerra a interface de comandos\n");

        printf("\n📡 Comando > ");
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = '\0'; // remove \n


        if (strncmp(comando, "status", 6) == 0) {
            printf("\n📋 Status das aeronaves:\n");
            for (int i = 0; i < QTD_AERONAVES; i++) {
                if (aeronaves[i].status != FINALIZADO && aeronaves[i].status != REMETIDA) {
                    imprimeAeronave(&aeronaves[i]);
                }
            }
        }
        else if (sscanf(comando, "pause %d", &id) == 1) {
            kill(aeronaves[id].pid, SIGSTOP);
            printf("⏸️ Aeronave %d pausada.\n", id);
        }
        else if (sscanf(comando, "resume %d", &id) == 1) {
            kill(aeronaves[id].pid, SIGCONT);
            printf("▶️ Aeronave %d retomada.\n", id);
        }
        else if (sscanf(comando, "kill %d", &id) == 1) {
            kill(aeronaves[id].pid, SIGKILL);
            aeronaves[id].status = REMETIDA;
            printf("💀 Aeronave %d finalizada.\n", id);
        }
        else if (strncmp(comando, "sair", 4) == 0) {
            printf("⛔ Encerrando interface de comandos...\n");
            break;
        }
        else {
            printf("❌ Comando inválido. Use: status | pause <id> | resume <id> | kill <id> | sair\n");
        }
    }

    return NULL;
}
