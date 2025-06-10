/*

Arquivo execução do algoritmo de busca em largura (BFS) sequencial.

Entrada: ./bfsSeq <arquivo_grafo> [vertice_inicial] [IMPRIME]

Saída: Imprime o tempo total para calcular BFS sequencial 
e imprime os vértices visitados se solicitado.

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define TAM 10000 

// Estrutura da fila circular usada na BFS
struct fila {
    int itens[TAM];
    int frente;
    int atras;
    int tamanho;
};

// Nó da lista de adjacência
struct no {
    int vertice;
    struct no* proximo;
};

// Estrutura do grafo com lista de adjacência
struct grafo {
    int nVertices;
    struct no** listaAdj;
    int* visitado;
};

// Cria um novo nó da lista de adjacência
struct no* criarNo(int v) {
    struct no* novoNo = malloc(sizeof(struct no));
    novoNo->vertice = v;
    novoNo->proximo = NULL;
    return novoNo;
}

// Inicializa fila circular
struct fila* criaFila() {
    struct fila* fila = malloc(sizeof(struct fila));
    fila->frente = 0;
    fila->atras = 0;
    fila->tamanho = 0;
    return fila;
}

// Cria um grafo com N vértices e inicializa estruturas
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

// Adiciona aresta bidirecional ao grafo (grafo não direcionado)
void adicionarAresta(struct grafo* grafo, int inicio, int destino) {
    struct no* novoNo = criarNo(destino);
    novoNo->proximo = grafo->listaAdj[inicio];
    grafo->listaAdj[inicio] = novoNo;

    // Adiciona também o inverso, pois se trata de um grafo não direcionado
    novoNo = criarNo(inicio);
    novoNo->proximo = grafo->listaAdj[destino];
    grafo->listaAdj[destino] = novoNo;
}

// Verifica se a fila está vazia
int ehVazio(struct fila* fila) {
    return fila->tamanho == 0;
}

// Coloca valor na fila circular
void enfileirar(struct fila* fila, int valor) {
    if (fila->tamanho == TAM) {
        fprintf(stderr, "Erro: fila circular cheia ao enfileirar %d\n", valor);
        return;
    }
    fila->itens[fila->atras] = valor;
    fila->atras = (fila->atras + 1) % TAM;
    fila->tamanho++;
}

// Retira elemento da frente da fila
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

// Libera memória alocada para o grafo
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

// Algoritmo de busca em largura (BFS) 
void bfs(struct grafo* grafo, int verticeInicio, int imprimir) {
    struct fila* fila = criaFila();
    grafo->visitado[verticeInicio] = 1;
    enfileirar(fila, verticeInicio);

    while (!ehVazio(fila)) {
        int verticeAtual = tiraFila(fila);
        if (imprimir) {
            printf("visitado %d\n", verticeAtual);
        }

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

// Lê o grafo de um arquivo binário no formato: 
// nVertices, nArestas, [origem, destino]
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
        printf("Uso: %s <arquivo_grafo> [vertice_inicial] [imprime]\n", argv[0]);
        return 1;
    }

    const char* nomeArquivo = argv[1];
    int verticeInicial = 0;
    int imprimir = 0;

    // Se o segundo argumento não começa com "-" assume que é o vértice inicial
    if (argc >= 3 && argv[2][0] != '-') {
        verticeInicial = atoi(argv[2]);
    }

    // Verifica se "imprime" foi passado como argumento
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "imprime") == 0) {
            imprimir = 1;
        }
    }

    struct grafo* grafo = lerGrafoBinario(nomeArquivo);

    if (grafo->nVertices > TAM) {
        fprintf(stderr, "Erro: número de vértices (%d) excede capacidade da fila (%d)\n", grafo->nVertices, TAM);
        liberarGrafo(grafo);
        return 1;
    }

    bfs(grafo, verticeInicial, imprimir);

    tempoFinal = clock();
    double tempoTotal = (double)(tempoFinal - tempoInicio) / CLOCKS_PER_SEC;
    printf("Tempo total para calcular BFS sequencial: %.3lf segundos\n", tempoTotal);

    liberarGrafo(grafo);
    return 0;
}