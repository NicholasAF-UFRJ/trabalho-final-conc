/*

    HEADER

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#define IMPRIME
#define TAM 10000 

struct fila {
    int itens[TAM];
    int frente;
    int atras;
    int tamanho;
};

struct no {
    int vertice;
    struct no* proximo;
};

struct grafo {
    int nVertices;
    struct no** listaAdj;
    int* visitado;
};

struct no* criarNo(int v) {
    struct no* novoNo = malloc(sizeof(struct no));
    novoNo->vertice = v;
    novoNo->proximo = NULL;
    return novoNo;
}

struct fila* criaFila() {
    struct fila* fila = malloc(sizeof(struct fila));
    fila->frente = 0;
    fila->atras = 0;
    fila->tamanho = 0;
    return fila;
}

struct grafo* criarGrafo(int vertices) {
    struct grafo* grafo = malloc(sizeof(struct grafo));
    grafo->nVertices = vertices;
    grafo->listaAdj = malloc(vertices * sizeof(struct no*));
    grafo->visitado = malloc(vertices * sizeof(int));
    for (int i = 0; i < vertices; i++) {
        grafo->listaAdj[i] = NULL;
        grafo->visitado[i] = 0;
    }
    return grafo;
}

void adicionarAresta(struct grafo* grafo, int inicio, int destino) {
    struct no* novoNo = criarNo(destino);
    novoNo->proximo = grafo->listaAdj[inicio];
    grafo->listaAdj[inicio] = novoNo;

    novoNo = criarNo(inicio);
    novoNo->proximo = grafo->listaAdj[destino];
    grafo->listaAdj[destino] = novoNo;
}

int ehVazio(struct fila* fila) {
    return fila->tamanho == 0;
}

void enfileirar(struct fila* fila, int valor) {
    if (fila->tamanho == TAM) {
        fprintf(stderr, "Erro: fila circular cheia ao enfileirar %d\n", valor);
        return;
    }
    fila->itens[fila->atras] = valor;
    fila->atras = (fila->atras + 1) % TAM;
    fila->tamanho++;
}

int tiraFila(struct fila* fila) {
    if (fila->tamanho == 0) {
        fprintf(stderr, "Erro: tentativa de tirar elemento de fila vazia\n");
        return -1;
    }
    int valor = fila->itens[fila->frente];
    fila->frente = (fila->frente + 1) % TAM;
    fila->tamanho--;
    return valor;
}

void liberarGrafo(struct grafo* grafo) {
    for (int i = 0; i < grafo->nVertices; i++) {
        struct no* atual = grafo->listaAdj[i];
        while (atual) {
            struct no* temp = atual;
            atual = atual->proximo;
            free(temp);
        }
    }
    free(grafo->listaAdj);
    free(grafo->visitado);
    free(grafo);
}

void bfs(struct grafo* grafo, int verticeInicio) {
    struct fila* fila = criaFila();
    grafo->visitado[verticeInicio] = 1;
    enfileirar(fila, verticeInicio);

    while (!ehVazio(fila)) {
        int verticeAtual = tiraFila(fila);
        #ifdef IMPRIME
            printf("visitado %d\n", verticeAtual);
        #endif

        struct no* temp = grafo->listaAdj[verticeAtual];
        while (temp) {
            int adj = temp->vertice;
            if (!grafo->visitado[adj]) {
                grafo->visitado[adj] = 1;
                enfileirar(fila, adj);
            }
            temp = temp->proximo;
        }
    }
    free(fila);
}

struct grafo* lerGrafoBinario(const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "rb");
    if (!f) {
        perror("Erro ao abrir arquivo binário");
        exit(EXIT_FAILURE);
    }

    int nVertices, nArestas;
    fread(&nVertices, sizeof(int), 1, f);
    fread(&nArestas, sizeof(int), 1, f);

    struct grafo* grafo = criarGrafo(nVertices);
    for (int i = 0; i < nArestas; i++) {
        int origem, destino;
        if (fread(&origem, sizeof(int), 1, f) != 1 || fread(&destino, sizeof(int), 1, f) != 1) {
            fprintf(stderr, "Erro ao ler aresta %d do arquivo\n", i);
            exit(EXIT_FAILURE);
        }
        adicionarAresta(grafo, origem, destino);
    }

    fclose(f);
    return grafo;
}

int main(int argc, char *argv[]) {

    clock_t tempoInicio, tempoFinal;
    tempoInicio = clock();

    if (argc < 2) {
        printf("Uso: %s <arquivo_grafo> [vertice_inicial]\n", argv[0]);
        return 1;
    }

    const char* nomeArquivo = argv[1];
    int verticeInicial = (argc >= 3) ? atoi(argv[2]) : 0;

    struct grafo* grafo = lerGrafoBinario(nomeArquivo);

    if (grafo->nVertices > TAM) {
        fprintf(stderr, "Erro: número de vértices (%d) excede capacidade da fila (%d)\n", grafo->nVertices, TAM);
        liberarGrafo(grafo);
        return 1;
    }
    
    bfs(grafo, verticeInicial);

    tempoFinal = clock();
    double tempoTotal = (double)(tempoFinal - tempoInicio) / CLOCKS_PER_SEC;
    printf("Tempo total para calcular BFS sequencial: %.3lf segundos\n", tempoTotal);

    liberarGrafo(grafo);

    return 0;
}