#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_ARESTAS(v) ((v) * ((v) - 1) / 2)

typedef struct {
    int origem;
    int destino;
} Aresta;

// Função auxiliar para salvar o grafo
void salvar_grafo(const char* nome_arquivo, Aresta* arestas, int nVertices, int nArestas) {
    FILE* f = fopen(nome_arquivo, "wb");
    if (!f) { perror("Erro ao abrir arquivo"); exit(EXIT_FAILURE); }

    fwrite(&nVertices, sizeof(int), 1, f);
    fwrite(&nArestas, sizeof(int), 1, f);
    for (int i = 0; i < nArestas; i++) {
        fwrite(&arestas[i].origem, sizeof(int), 1, f);
        fwrite(&arestas[i].destino, sizeof(int), 1, f);
    }
    fclose(f);
}

void gerar_arvore(const char* nome_arquivo, int nVertices) {
    Aresta* arestas = malloc((nVertices - 1) * sizeof(Aresta));
    for (int i = 1; i < nVertices; i++) {
        int pai = rand() % i;
        arestas[i - 1] = (Aresta){pai, i};
    }
    salvar_grafo(nome_arquivo, arestas, nVertices, nVertices - 1);
    free(arestas);
}

void gerar_arvore_binaria(const char* nome_arquivo, int nVertices) {
    Aresta* arestas = malloc((nVertices - 1) * sizeof(Aresta));
    for (int i = 1; i < nVertices; i++) {
        arestas[i - 1] = (Aresta){(i - 1) / 2, i};
    }
    salvar_grafo(nome_arquivo, arestas, nVertices, nVertices - 1);
    free(arestas);
}

void gerar_arvore_larga(const char* nome_arquivo, int nVertices, int filhosPorNo) {
    Aresta* arestas = malloc((nVertices - 1) * sizeof(Aresta));
    int atual = 1, count = 0;
    for (int i = 0; i < nVertices && atual < nVertices; i++) {
        for (int j = 0; j < filhosPorNo && atual < nVertices; j++) {
            arestas[count++] = (Aresta){i, atual++};
        }
    }
    salvar_grafo(nome_arquivo, arestas, nVertices, count);
    free(arestas);
}

void gerar_grafo_conexo(const char* nome_arquivo, int nVertices) {
    int maxArestas = MAX_ARESTAS(nVertices);
    int minArestas = nVertices;
    int nArestas = minArestas + rand() % (maxArestas - minArestas + 1);

    Aresta* arestas = malloc(nArestas * sizeof(Aresta));
    bool* adj = calloc(nVertices * nVertices, sizeof(bool));

    int count = 0;
    for (int i = 1; i < nVertices; i++) {
        int destino = rand() % i;
        arestas[count] = (Aresta){i, destino};
        adj[i * nVertices + destino] = true;
        adj[destino * nVertices + i] = true;
        count++;
    }

    while (count < nArestas) {
        int a = rand() % nVertices;
        int b = rand() % nVertices;
        if (a != b && !adj[a * nVertices + b]) {
            arestas[count++] = (Aresta){a, b};
            adj[a * nVertices + b] = true;
            adj[b * nVertices + a] = true;
        }
    }

    salvar_grafo(nome_arquivo, arestas, nVertices, nArestas);
    free(arestas);
    free(adj);
}

void gerar_grafo_pequeno(const char* nome_arquivo) {
    int nVertices = 6;
    Aresta arestas[] = {
        {0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 5}
    };
    salvar_grafo(nome_arquivo, arestas, nVertices, 5);
}

void gerar_grafo_com_ciclo(const char* nome_arquivo, int nVertices) {
    Aresta* arestas = malloc((nVertices + 1) * sizeof(Aresta));
    for (int i = 1; i < nVertices; i++) {
        arestas[i - 1] = (Aresta){i - 1, i};
    }
    arestas[nVertices - 1] = (Aresta){nVertices - 1, 0};
    salvar_grafo(nome_arquivo, arestas, nVertices, nVertices);
    free(arestas);
}

void gerar_grafo_isolado(const char* nome_arquivo, int nVertices) {
    Aresta arestas[] = {{0, 1}, {2, 3}};
    salvar_grafo(nome_arquivo, arestas, nVertices, 2);
}

void gerar_grafo_completo(const char* nome_arquivo, int nVertices) {
    int nArestas = MAX_ARESTAS(nVertices);
    Aresta* arestas = malloc(nArestas * sizeof(Aresta));
    int count = 0;
    for (int i = 0; i < nVertices; i++) {
        for (int j = i + 1; j < nVertices; j++) {
            arestas[count++] = (Aresta){i, j};
        }
    }
    salvar_grafo(nome_arquivo, arestas, nVertices, count);
    free(arestas);
}

void gerar_grafo_linear(const char* nome_arquivo, int nVertices) {
    Aresta* arestas = malloc((nVertices - 1) * sizeof(Aresta));
    for (int i = 0; i < nVertices - 1; i++) {
        arestas[i] = (Aresta){i, i + 1};
    }
    salvar_grafo(nome_arquivo, arestas, nVertices, nVertices - 1);
    free(arestas);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <tipo> <nVertices> [filhosPorNo]\n", argv[0]);
        printf("Tipos: 1-Arvore | 2-Binaria | 3-Conexo | 4-Larga | 5-Pequeno | 6-Ciclo | 7-Isolado | 8-Completo | 9-Linear\n");
        return 1;
    }

    int tipo = atoi(argv[1]);
    int nVertices = atoi(argv[2]);
    const char* nomeArquivo;
    int filhosPorNo = (argc >= 4) ? atoi(argv[3]) : 10;

    srand(time(NULL));

    switch (tipo) {
        case 1: nomeArquivo = "arvore"; gerar_arvore(nomeArquivo, nVertices); break;
        case 2: nomeArquivo = "arvoreBinaria"; gerar_arvore_binaria(nomeArquivo, nVertices); break;
        case 3: nomeArquivo = "grafoConexo"; gerar_grafo_conexo(nomeArquivo, nVertices); break;
        case 4: nomeArquivo = "arvoreLarga"; gerar_arvore_larga(nomeArquivo, nVertices, filhosPorNo); break;
        case 5: nomeArquivo = "grafoPequeno"; gerar_grafo_pequeno(nomeArquivo); break;
        case 6: nomeArquivo = "grafoCiclo"; gerar_grafo_com_ciclo(nomeArquivo, nVertices); break;
        case 7: nomeArquivo = "grafoIsolado"; gerar_grafo_isolado(nomeArquivo, nVertices); break;
        case 8: nomeArquivo = "grafoCompleto"; gerar_grafo_completo(nomeArquivo, nVertices); break;
        case 9: nomeArquivo = "grafoLinear"; gerar_grafo_linear(nomeArquivo, nVertices); break;
        default: printf("Tipo invalido.\n"); return 1;
    }

    printf("Grafo gerado em '%s' com tipo %d.\n", nomeArquivo, tipo);
    return 0;
}