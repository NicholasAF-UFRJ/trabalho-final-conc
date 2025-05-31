CC = gcc
CFLAGS = -Wall -O2

# Alvos (executáveis)
all: geraGrafo leArqBinario bfsSeq bfsConcEstat bfsConcDin

geraGrafo: geraGrafo.c
	$(CC) $(CFLAGS) -o geraGrafo geraGrafo.c

leArqBinario: leArqBinario.c
	$(CC) $(CFLAGS) -o leArqBinario leArqBinario.c

bfsSeq: bfsSeq.c
	$(CC) $(CFLAGS) -o bfsSeq bfsSeq.c

bfsConcEstat: bfsConcEstat.c
	$(CC) $(CFLAGS) -o bfsConcEstat bfsConcEstat.c

bfsConcDin: bfsConcDin.c
	$(CC) $(CFLAGS) -o bfsConcDin bfsConcDin.c

clean:
	rm -f geraGrafo leArqBinario bfsSeq bfsConcEstat bfsConcDin *.o
