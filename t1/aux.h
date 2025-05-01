struct Ponto{
    float x;
    float y;
};

struct Aeronave{
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
    REDIRECIONADO = 2
};