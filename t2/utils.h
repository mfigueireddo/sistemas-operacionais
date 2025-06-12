#ifndef UTILS
#define UTILS

    #include <stdio.h>
    #include <stdlib.h>

    // Exibe mensagens para acompanhar o andamento do programa
    #if MODO_TESTE
        #define LOG(...) do { printf(__VA_ARGS__); } while (0)
    #else
        #define LOG(...) do {} while (0)
    #endif

    // Macros
    #define QTD_PROCESSOS 4
    #define QTD_PAGINAS 32
    #define MAX_PAGINAS 16
    #define forProcessos(i) for(int i=0; i<QTD_PROCESSOS; i++)
    #define forPaginas(i) for(int i=0; i<QTD_PAGINAS; i++)
    #define forMemoria(i) for(int i=0; i<MAX_PAGINAS; i++)
    #define READWRITE_MODE 0666
    #define READ_MODE (O_RDONLY | O_NONBLOCK)
    #define WRITE_MODE (O_WRONLY | O_NONBLOCK)

    // Funções de utils.c
    FILE* abreArquivoTexto(char* caminho, char modo);
    void fechaArquivoTexto(FILE* arquivo);
    int conectaPipe(char *caminho, int modo);

    // Estrutura básica de uma página
    typedef struct BasePage
    {
        int num;
        char modo;
        int processo;
        void* extra;
    } BasePage;

#endif