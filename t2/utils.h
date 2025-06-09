#ifndef UTILS
#define UTILS

    #ifndef MODO_TESTE
    #define MODO_TESTE 0
    #endif

    #define QTD_PROCESSOS 4
    #define QTD_PAGINAS 64

    #define forProcessos(i) for(int i=0; i<QTD_PROCESSOS; i++)
    #define forPaginas(i) for(int i=0; i<QTD_PAGINAS; i++)

    // Funções de utils.c
    FILE* abreArquivoTexto(char* caminho, char modo);
    void fechaArquivoTexto(FILE* arquivo);

    // Estrutura básica

#endif