#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ARESTAS(v) ((v) * ((v) - 1) / 2)

typedef struct {
    int origem;
    int destino;
} Aresta;

int aresta_existe(Aresta* arestas, int count, int origem, int destino) {
    for (int i = 0; i < count; i++) {
        if ((arestas[i].origem == origem && arestas[i].destino == destino) ||
            (arestas[i].origem == destino && arestas[i].destino == origem)) {
            return 1;
        }
    }
    return 0;
}

void gerar_grafo_binario(const char* nome_arquivo, int nVertices) {
    FILE* f = fopen(nome_arquivo, "wb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }

    int maxArestas = MAX_ARESTAS(nVertices);
    int minArestas = nVertices; // pelo menos conexo
    int nArestas = minArestas + rand() % (maxArestas - minArestas + 1);

    Aresta* arestas = malloc(nArestas * sizeof(Aresta));
    int count = 0;

    while (count < nArestas) {
        int origem = rand() % nVertices;
        int destino = rand() % nVertices;

        if (origem != destino && !aresta_existe(arestas, count, origem, destino)) {
            arestas[count].origem = origem;
            arestas[count].destino = destino;
            count++;
        }
    }

    // Escreve vértices e arestas no arquivo binário
    fwrite(&nVertices, sizeof(int), 1, f);
    fwrite(&nArestas, sizeof(int), 1, f);

    for (int i = 0; i < nArestas; i++) {
        fwrite(&arestas[i].origem, sizeof(int), 1, f);
        fwrite(&arestas[i].destino, sizeof(int), 1, f);
    }

    fclose(f);
    free(arestas);

    printf("Grafo com %d vértices e %d arestas gerado em '%s'\n", nVertices, nArestas, nome_arquivo);
}

int main(int argc, char *argv[]) {

    clock_t tempoInicio, tempoFinal;
    tempoInicio = clock();

    if (argc < 2) {
        printf("Uso: %s <nVertices>\n", argv[0]);
        return 1;
    }

    int nVertices = atoi(argv[1]);
    if (nVertices < 2) {
        printf("Número de vértices deve ser >= 2\n");
        return 1;
    }

    srand(time(NULL)); // Semente aleatória

    gerar_grafo_binario("grafo.bin", nVertices);

    tempoFinal = clock();
    double tempoTotal = (double)(tempoFinal - tempoInicio) / CLOCKS_PER_SEC;
    printf("Tempo total para gerar grafo: %.3lf segundos\n", tempoTotal);

    return 0;
}