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

void gerar_arvore(const char* nome_arquivo, int nVertices) {
    FILE* f = fopen(nome_arquivo, "wb");
    if (!f) { perror("Erro ao abrir arquivo"); exit(EXIT_FAILURE); }

    Aresta* arestas = malloc((nVertices - 1) * sizeof(Aresta));

    for (int i = 1; i < nVertices; i++) {
        int pai = rand() % i;
        arestas[i - 1].origem = pai;
        arestas[i - 1].destino = i;
    }

    fwrite(&nVertices, sizeof(int), 1, f);
    int nArestas = nVertices - 1;
    fwrite(&nArestas, sizeof(int), 1, f);

    for (int i = 0; i < nArestas; i++) {
        fwrite(&arestas[i].origem, sizeof(int), 1, f);
        fwrite(&arestas[i].destino, sizeof(int), 1, f);
    }

    free(arestas);
    fclose(f);
}

void gerar_arvore_binaria(const char* nome_arquivo, int nVertices) {
    FILE* f = fopen(nome_arquivo, "wb");
    if (!f) { perror("Erro ao abrir arquivo"); exit(EXIT_FAILURE); }

    Aresta* arestas = malloc((nVertices - 1) * sizeof(Aresta));
    int count = 0;

    for (int i = 1; i < nVertices; i++) {
        int pai = (i - 1) / 2; // regra da árvore binária
        arestas[count++] = (Aresta){pai, i};
    }

    fwrite(&nVertices, sizeof(int), 1, f);
    int nArestas = nVertices - 1;
    fwrite(&nArestas, sizeof(int), 1, f);

    for (int i = 0; i < nArestas; i++) {
        fwrite(&arestas[i].origem, sizeof(int), 1, f);
        fwrite(&arestas[i].destino, sizeof(int), 1, f);
    }

    free(arestas);
    fclose(f);
}

void gerar_grafo_conexo(const char* nome_arquivo, int nVertices) {
    FILE* f = fopen(nome_arquivo, "wb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }

    int maxArestas = MAX_ARESTAS(nVertices);
    int minArestas = nVertices;
    int nArestas = minArestas + rand() % (maxArestas - minArestas + 1);

    Aresta* arestas = malloc(nArestas * sizeof(Aresta));
    int count = 0;

    // Primeiro conecta os vértices como uma árvore básica
    for (int i = 1; i < nVertices; i++) {
        arestas[count].origem = i;
        arestas[count].destino = rand() % i;
        count++;
    }

    // Agora adiciona arestas extras aleatórias
    while (count < nArestas) {
        int origem = rand() % nVertices;
        int destino = rand() % nVertices;
        if (origem != destino && !aresta_existe(arestas, count, origem, destino)) {
            arestas[count++] = (Aresta){origem, destino};
        }
    }

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

void gerar_arvore_larga(const char* nome_arquivo, int nVertices, int filhosPorNo) {
    FILE* f = fopen(nome_arquivo, "wb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }

    Aresta* arestas = malloc((nVertices - 1) * sizeof(Aresta));
    int arestaCount = 0;

    int atual = 1; // começa com o vértice 0 como raiz
    for (int i = 0; i < nVertices && atual < nVertices; i++) {
        for (int j = 0; j < filhosPorNo && atual < nVertices; j++) {
            arestas[arestaCount].origem = i;
            arestas[arestaCount].destino = atual;
            arestaCount++;
            atual++;
        }
    }

    fwrite(&nVertices, sizeof(int), 1, f);
    fwrite(&arestaCount, sizeof(int), 1, f);

    for (int i = 0; i < arestaCount; i++) {
        fwrite(&arestas[i].origem, sizeof(int), 1, f);
        fwrite(&arestas[i].destino, sizeof(int), 1, f);
    }

    fclose(f);
    free(arestas);
    printf("Grafo árvore larga gerado com %d vértices e %d arestas em '%s'\n", nVertices, arestaCount, nome_arquivo);
}

int main(int argc, char *argv[]) {

    clock_t tempoInicio, tempoFinal;
    tempoInicio = clock();

    if (argc < 3) {
        printf("Uso: %s <tipo> <nVertices> [filhosPorNo]\n", argv[0]);
        printf("Tipos: 1 - árvore | 2 - árvore binária | 3 - conexo | 4 - árvore larga\n");
        return 1;
    }
    
    int tipo = atoi(argv[1]);

    int nVertices = atoi(argv[2]);
    if (nVertices < 2) {
        printf("Número de vértices deve ser >= 2\n");
        return 1;
    }

    int filhosPorNo = (argc >= 4) ? atoi(argv[3]) : 10;

    srand(time(NULL)); // Semente aleatória

    switch (tipo) {
        case 1:
            gerar_arvore("grafoArvore.bin", nVertices);
            break;
        case 2:
            gerar_arvore_binaria("grafoArvoreBin.bin", nVertices);
            break;
        case 3:
            gerar_grafo_conexo("grafoConexo.bin", nVertices);
            break;
        case 4:
            gerar_arvore_larga("grafoLargo.bin", nVertices, filhosPorNo);
            break;
        default:
            printf("Tipo inválido. Use 1 (árvore), 2 (árvore binária) ou 3 (conexo)\n");
            return 1;
    }

    tempoFinal = clock();
    double tempoTotal = (double)(tempoFinal - tempoInicio) / CLOCKS_PER_SEC;
    printf("Tempo total para gerar grafo: %.3lf segundos\n", tempoTotal);

    return 0;
}