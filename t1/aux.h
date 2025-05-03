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
};

enum StatusAeronave{
    VOANDO = 0,
    AGUARDANDO = 1,
    FINALIZADO = 2
};

// !!! FAZER FUNÇÃO PARA PEGAR O ESTADO COM BASE NO NÚMERO !!!