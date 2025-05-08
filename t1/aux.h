struct Ponto{
    float x;
    float y;
};

struct Pista{
    int num;
    int ocupacao;
};

struct Aeronave{
    int id;
    struct Ponto ponto;
    char direcao; // W ou E
    float velocidade;
    int pista_preferida;
    int status;
    int pid;
    int delay;
};

enum StatusAeronave{
    VOANDO = 0,
    AGUARDANDO = 1,
    FINALIZADO = 2,
    REMETIDA = 3,
    DELAY = 4
};

// Funções utilizadas por mais de um módulo
float movimentaX(struct Aeronave *aeronave);
float movimentaY(struct Aeronave *aeronave);
void imprimeAeronave(struct Aeronave *aeronave);
const char* stringStatus(int status);
int alteraPista(int pista);

// Macros
#ifndef QTD_AERONAVES
#define QTD_AERONAVES 5
#endif

#define QTD_PISTAS 4