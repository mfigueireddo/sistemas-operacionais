CC = gcc
CFLAGS = -DMODO_TESTE=0

# Arquivos com main
MAIN_SRCS = main.c processos/processo1.c processos/processo2.c processos/processo3.c processos/processo4.c
MAIN_OBJS = $(MAIN_SRCS:.c=.o)

# Arquivo sem main
UTILS_SRC = utils.c
UTILS_OBJ = $(UTILS_SRC:.c=.o)

TARGETS = $(MAIN_SRCS:.c=)

all: $(TARGETS)

# Compilar utils.o
$(UTILS_OBJ): $(UTILS_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar .o dos arquivos com main
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Linkar executáveis com utils.o junto
%: %.o $(UTILS_OBJ)
	$(CC) $^ -o $@

# Exclui todos os .exe e .o com "make clean"
clean:
	rm -f $(TARGETS) $(MAIN_OBJS) $(UTILS_OBJ)

.PHONY: all clean