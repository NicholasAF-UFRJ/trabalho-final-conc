CC = gcc
CFLAGS = -Wall -O2

# Alvos (executáveis)
all: geraGrafo leBin bfsSeq bfsConcEstat bfsConcDin

geraGrafo: geraGrafo.c
	$(CC) $(CFLAGS) -o geraGrafo geraGrafo.c

leBin: leBin.c
	$(CC) $(CFLAGS) -o leBin leBin.c

bfsSeq: bfsSeq.c
	$(CC) $(CFLAGS) -o bfsSeq bfsSeq.c

bfsConcEstat: bfsConcEstat.c
	$(CC) $(CFLAGS) -o bfsConcEstat bfsConcEstat.c

bfsConcDin: bfsConcDin.c
	$(CC) $(CFLAGS) -o bfsConcDin bfsConcDin.c



clean:
	rm -f geraGrafo leBin bfsSeq bfsConcEstat bfsConcDin *.o
