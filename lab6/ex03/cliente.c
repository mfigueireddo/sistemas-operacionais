#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define TAMANHO 256

int main() {
    char mensagem[TAMANHO];

    int entrada = open("/tmp/fila_entrada", O_WRONLY);
    int saida = open("/tmp/fila_saida", O_RDONLY);

    if (entrada < 0 || saida < 0) {
        perror("Erro ao abrir as filas");
        exit(1);
    }

    while (1) {
        printf("Digite uma mensagem: ");
        fgets(mensagem, TAMANHO, stdin);

        write(entrada, mensagem, strlen(mensagem));
        read(saida, mensagem, TAMANHO);

        printf("Resposta do servidor: %s\n", mensagem);
    }

    close(entrada);
    close(saida);
    return 0;
}
