/*

Arquivo execução do algoritmo de busca em largura (BFS) concorrente
com mutexes e barreira de sincronização e divisão dinâmica da fila.

Entrada: ./bfsConcDin <arquivo_grafo> <nThreads> [verticeInicial]

Saída: Imprime o tempo total para calcular BFS concorrente.

*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define TAM 100000          
#define MAX_THREADS 16      

// Estruturas 

struct fila {
    int itens[TAM];         
    int frente, atras;      
    pthread_mutex_t lock;   
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

// Operações com fila

struct fila* criaFila() {
    struct fila* fila = malloc(sizeof(struct fila));
    fila->frente = -1;
    fila->atras = -1;
    pthread_mutex_init(&fila->lock, NULL); // Inicializa o mutex da fila
    return fila;
}

int ehVazio(struct fila* fila) {
    return fila->atras == -1;
}

// Enfileira valor com exclusão mútua
void enfileirar(struct fila* fila, int valor) {
    pthread_mutex_lock(&fila->lock);
    if (fila->atras < TAM - 1) {
        if (fila->frente == -1) fila->frente = 0;
        fila->itens[++fila->atras] = valor;
    }
    pthread_mutex_unlock(&fila->lock);
}

// Remove valor da fila com exclusão mútua
int tiraFila(struct fila* fila) {
    pthread_mutex_lock(&fila->lock);
    int item = -1;
    if (!ehVazio(fila)) {
        item = fila->itens[fila->frente++];
        if (fila->frente > fila->atras) fila->frente = fila->atras = -1;
    }
    pthread_mutex_unlock(&fila->lock);
    return item;
}

void liberarFila(struct fila* fila) {
    pthread_mutex_destroy(&fila->lock);
    free(fila);
}

//  Grafo 

struct no* criarNo(int v) {
    struct no* novoNo = malloc(sizeof(struct no));
    novoNo->vertice = v;
    novoNo->proximo = NULL;
    return novoNo;
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

// Adiciona aresta bidirecional (grafo não direcionado)
void adicionarAresta(struct grafo* grafo, int inicio, int destino) {
    struct no* novoNo = criarNo(destino);
    novoNo->proximo = grafo->listaAdj[inicio];
    grafo->listaAdj[inicio] = novoNo;

    novoNo = criarNo(inicio);
    novoNo->proximo = grafo->listaAdj[destino];
    grafo->listaAdj[destino] = novoNo;
}

// Lê grafo a partir de um arquivo binário 
struct grafo* lerGrafoBinario(const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "rb");
    if (!f) { perror("Erro ao abrir arquivo"); exit(EXIT_FAILURE); }
    int nVertices, nArestas;
    fread(&nVertices, sizeof(int), 1, f);
    fread(&nArestas, sizeof(int), 1, f);
    struct grafo* grafo = criarGrafo(nVertices);
    for (int i = 0; i < nArestas; i++) {
        int o, d;
        fread(&o, sizeof(int), 1, f);
        fread(&d, sizeof(int), 1, f);
        adicionarAresta(grafo, o, d);
    }
    fclose(f);
    return grafo;
}

// Liberação da memória usada pelo grafo
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

// BFS Dinâmico 

// Estrutura de argumentos para as threads
struct args {
    int id;
    struct grafo* grafo;
    struct fila* fila;
    pthread_mutex_t* mutexesVisitado;
};

// Função executada por cada thread
void* bfs_thread_dinamico(void* arg) {
    struct args* a = (struct args*)arg;
    while (1) {
        int v = tiraFila(a->fila);     // Cada thread pega um vértice da fila
        if (v == -1) break;            // Se fila estiver vazia, encerra

        struct no* temp = a->grafo->listaAdj[v];
        while (temp) {
            int adj = temp->vertice;

            // Exclusão mútua por vértice
            pthread_mutex_lock(&a->mutexesVisitado[adj]);
            if (!a->grafo->visitado[adj]) {
                a->grafo->visitado[adj] = 1;
                pthread_mutex_unlock(&a->mutexesVisitado[adj]);
                enfileirar(a->fila, adj);   // Adiciona novo vértice à fila
            } else {
                pthread_mutex_unlock(&a->mutexesVisitado[adj]);
            }

            temp = temp->proximo;
        }
    }
    return NULL;
}

//  Função Principal 

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo> <nThreads> [verticeInicial]\n", argv[0]);
        return 1;
    }

    clock_t inicio = clock();  

    const char* arquivo = argv[1];
    int nThreads = atoi(argv[2]);
    int inicioVertice = (argc >= 4) ? atoi(argv[3]) : 0;

    struct grafo* grafo = lerGrafoBinario(arquivo);

    pthread_t threads[MAX_THREADS];
    struct args a[MAX_THREADS];
    pthread_mutex_t* mutexesVisitado = malloc(grafo->nVertices * sizeof(pthread_mutex_t));
    for (int i = 0; i < grafo->nVertices; i++) 
        pthread_mutex_init(&mutexesVisitado[i], NULL);

    struct fila* fila = criaFila();

    grafo->visitado[inicioVertice] = 1;  
    enfileirar(fila, inicioVertice);

    // Cria as threads de BFS concorrente
    for (int i = 0; i < nThreads; i++) {
        a[i].id = i;
        a[i].grafo = grafo;
        a[i].fila = fila;
        a[i].mutexesVisitado = mutexesVisitado;
        pthread_create(&threads[i], NULL, bfs_thread_dinamico, &a[i]);
    }

    // Aguarda todas as threads terminarem
    for (int i = 0; i < nThreads; i++) 
        pthread_join(threads[i], NULL);

    clock_t fim = clock();  // Tempo final
    printf("Tempo total: %.3lf segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
    
    int n = grafo->nVertices;

    // Libera recursos (mutexes e memória)
    for (int i = 0; i < n; i++)
        pthread_mutex_destroy(&mutexesVisitado[i]);
    free(mutexesVisitado);

    liberarFila(fila);
    liberarGrafo(grafo);

    return 0;
}
