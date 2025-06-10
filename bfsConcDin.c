/*





IMPLEMENTAÇÃO BFS CONCORRENTE
DIVISÃO DE TAREFAS DINÂMICA





*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define TAM 100000
#define MAX_THREADS 16

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

struct fila* criaFila() {
    struct fila* fila = malloc(sizeof(struct fila));
    fila->frente = -1;
    fila->atras = -1;
    pthread_mutex_init(&fila->lock, NULL);
    return fila;
}

int ehVazio(struct fila* fila) {
    return fila->atras == -1;
}

void enfileirar(struct fila* fila, int valor) {
    pthread_mutex_lock(&fila->lock);
    if (fila->atras < TAM - 1) {
        if (fila->frente == -1) fila->frente = 0;
        fila->itens[++fila->atras] = valor;
    }
    pthread_mutex_unlock(&fila->lock);
}

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

void adicionarAresta(struct grafo* grafo, int inicio, int destino) {
    struct no* novoNo = criarNo(destino);
    novoNo->proximo = grafo->listaAdj[inicio];
    grafo->listaAdj[inicio] = novoNo;

    novoNo = criarNo(inicio);
    novoNo->proximo = grafo->listaAdj[destino];
    grafo->listaAdj[destino] = novoNo;
}

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

// ---------------------- BFS Dinâmico ----------------------
struct args {
    int id;
    struct grafo* grafo;
    struct fila* fila;
    pthread_mutex_t* mutexesVisitado;
};

void* bfs_thread_dinamico(void* arg) {
    struct args* a = (struct args*)arg;
    while (1) {
        int v = tiraFila(a->fila);
        if (v == -1) break;  // Fila vazia

        struct no* temp = a->grafo->listaAdj[v];
        while (temp) {
            int adj = temp->vertice;
            pthread_mutex_lock(&a->mutexesVisitado[adj]);
            if (!a->grafo->visitado[adj]) {
                a->grafo->visitado[adj] = 1;
                pthread_mutex_unlock(&a->mutexesVisitado[adj]);
                enfileirar(a->fila, adj);
                //printf("visitado %d pela thread %d\n", adj, a->id);
            } else {
                pthread_mutex_unlock(&a->mutexesVisitado[adj]);
            }
            temp = temp->proximo;
        }
    }
    return NULL;
}

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
    for (int i = 0; i < grafo->nVertices; i++) pthread_mutex_init(&mutexesVisitado[i], NULL);

    struct fila* fila = criaFila();
    grafo->visitado[inicioVertice] = 1;
    enfileirar(fila, inicioVertice);

    for (int i = 0; i < nThreads; i++) {
        a[i].id = i;
        a[i].grafo = grafo;
        a[i].fila = fila;
        a[i].mutexesVisitado = mutexesVisitado;
        pthread_create(&threads[i], NULL, bfs_thread_dinamico, &a[i]);
    }

    for (int i = 0; i < nThreads; i++) pthread_join(threads[i], NULL);

    clock_t fim = clock();
    printf("Tempo total: %.3lf segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
    
    int n = grafo->nVertices;
    
    for (int i = 0; i < n; i++)
        pthread_mutex_destroy(&mutexesVisitado[i]);
    free(mutexesVisitado);

    liberarFila(fila);
    liberarGrafo(grafo);

    return 0;
}