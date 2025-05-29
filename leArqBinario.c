#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *arquivo;
    int nVertices, nArestas;
    int origem, destino;

    if (argc < 2) {
        printf("Uso: %s <arquivoGrafoBinario>\n", argv[0]);
        return 1;
    }

    // Abre o arquivo para leitura binária
    arquivo = fopen(argv[1], "rb");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return 2;
    }

    // Lê o número de vértices e arestas
    fread(&nVertices, sizeof(int), 1, arquivo);
    fread(&nArestas, sizeof(int), 1, arquivo);

    printf("Número de vértices: %d\n", nVertices);
    printf("Número de arestas: %d\n", nArestas);
    printf("Lista de arestas:\n");

    // Lê e imprime cada par (origem, destino)
    for (int i = 0; i < nArestas; i++) {
        fread(&origem, sizeof(int), 1, arquivo);
        fread(&destino, sizeof(int), 1, arquivo);
        printf("%d %d\n", origem, destino);
    }

    fclose(arquivo);
    return 0;
}