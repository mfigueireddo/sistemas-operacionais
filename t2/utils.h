#ifndef UTILS
#define UTILS

    // Exibe mensagens para acompanhar o andamento do programa
    #ifndef MODO_TESTE
    #define MODO_TESTE 0
    #endif

    // Macros
    #define QTD_PROCESSOS 4
    #define QTD_PAGINAS 64
    #define MAX_PAGINAS 16
    #define forProcessos(i) for(int i=0; i<QTD_PROCESSOS; i++)
    #define forPaginas(i) for(int i=0; i<QTD_PAGINAS; i++)
    #define forMemoria(i) for(int i=0; i<16; i++)
    #define READWRITEMODE O_RDWR 
    #define READMODE O_RDONLY
    #define WRITEMODE O_WRONLY

    // Funções de utils.c
    FILE* abreArquivoTexto(char* caminho, char modo);
    void fechaArquivoTexto(FILE* arquivo);

    // Estrutura básica de uma página
    struct BasePage
    {
        int num;
        char modo;
        int processo;
        int uso;
        void* extra;
    };

#endif