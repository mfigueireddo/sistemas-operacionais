#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define TAMANHO 256

void converter_para_maiusculas(char *mensagem) {
    for (int i = 0; mensagem[i]; i++) {
        mensagem[i] = toupper((unsigned char)mensagem[i]);
    }
}

int main() {
    char mensagem[TAMANHO];

    int entrada = open("/tmp/fila_entrada", O_RDONLY);
    int saida = open("/tmp/fila_saida", O_WRONLY);

    if (entrada < 0 || saida < 0) {
        perror("Erro ao abrir as filas");
        exit(1);
    }

    while (1) {
        read(entrada, mensagem, TAMANHO);
        converter_para_maiusculas(mensagem);
        write(saida, mensagem, strlen(mensagem));
    }

    close(entrada);
    close(saida);
    return 0;
}
