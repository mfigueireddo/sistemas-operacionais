CC = gcc
CFLAGS = -DMODO_TESTE=0 -I.

all: main processos/processo1 processos/processo2 processos/processo3 processos/processo4

main: main.c gmv.c utils.c \
      algoritmos/second_chance.c \
      algoritmos/nru.c \
      algoritmos/lru.c \
      algoritmos/working_set.c
	$(CC) $(CFLAGS) main.c gmv.c utils.c \
		algoritmos/second_chance.c \
		algoritmos/nru.c \
		algoritmos/lru.c \
		algoritmos/working_set.c \
		-o main -lpthread

processos/processo1: processos/processo1.c utils.c
	$(CC) $(CFLAGS) processos/processo1.c utils.c -o processos/processo1

processos/processo2: processos/processo2.c utils.c
	$(CC) $(CFLAGS) processos/processo2.c utils.c -o processos/processo2

processos/processo3: processos/processo3.c utils.c
	$(CC) $(CFLAGS) processos/processo3.c utils.c -o processos/processo3

processos/processo4: processos/processo4.c utils.c
	$(CC) $(CFLAGS) processos/processo4.c utils.c -o processos/processo4

clean:
	rm -f main processos/processo1 processos/processo2 processos/processo3 processos/processo4
	rm -f pipes/pipe1 pipes/pipe2 pipes/pipe3 pipes/pipe4
	rm -f arquivos_txt/ordem_processo1.txt arquivos_txt/ordem_processo2.txt arquivos_txt/ordem_processo3.txt arquivos_txt/ordem_processo4.txt 