struct Ponto{
    float x;
    float y;
};

struct Pista{
    int num;
    int estaOcupada;
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

float movimentaX(struct Aeronave *aeronave);
float movimentaY(struct Aeronave *aeronave);
void imprimeAeronave(struct Aeronave *aeronave);
const char* stringStatus(int status);