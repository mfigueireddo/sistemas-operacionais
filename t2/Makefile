CC = gcc
CFLAGS = -DMODO_TESTE=1

all: main processos/processo1 processos/processo2 processos/processo3 processos/processo4

main: main.c utils.c
	$(CC) $(CFLAGS) main.c utils.c -o main

processos/processo1: processos/processo1.c
	$(CC) $(CFLAGS) processos/processo1.c -o processos/processo1

processos/processo2: processos/processo2.c
	$(CC) $(CFLAGS) processos/processo2.c -o processos/processo2

processos/processo3: processos/processo3.c
	$(CC) $(CFLAGS) processos/processo3.c -o processos/processo3

processos/processo4: processos/processo4.c
	$(CC) $(CFLAGS) processos/processo4.c -o processos/processo4

clean:
	rm -f main processos/processo1 processos/processo2 processos/processo3 processos/processo4
