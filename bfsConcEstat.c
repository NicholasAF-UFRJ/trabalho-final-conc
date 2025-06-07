/*






IMPLEMENTAÇÃO BFS CONCORRENTE
DIVISÃO DE TAREFAS ESTÁTICA






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

// ---------- Fila ----------
struct fila* criaFila() {
    struct fila* fila = malloc(sizeof(struct fila));
    fila->frente = -1;
    fila->atras = -1;
    return fila;
}

int ehVazio(struct fila* fila) {
    return fila->atras == -1;
}

void enfileirar(struct fila* fila, int valor) {
    if (fila->atras >= TAM - 1) return;
    if (fila->frente == -1) fila->frente = 0;
    fila->atras++;
    fila->itens[fila->atras] = valor;
}

int tiraFila(struct fila* fila) {
    int item = -1;
    if (!ehVazio(fila)) {
        item = fila->itens[fila->frente];
        fila->frente++;
        if (fila->frente > fila->atras) fila->frente = fila->atras = -1;
    }
    return item;
}

void liberarFila(struct fila* fila) {
    free(fila);
}

// ---------- Grafo ----------
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

// ---------- Sincronização ----------
int nThreads;
int bloqueadas = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void barreira(int nthreads) {
    pthread_mutex_lock(&mutex);
    if (bloqueadas == (nthreads - 1)) {
        bloqueadas = 0;
        pthread_cond_broadcast(&cond);
    } else {
        bloqueadas++;
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

// ---------- BFS Concorrente ----------
struct thread_args {
    int id;
    struct grafo* grafo;
    struct fila** filaAtualPtr;
    struct fila* filaProxima;
    int* nivelAtual;
    pthread_mutex_t* mutexesVisitado;
    pthread_mutex_t* mutexFila;
};

void* bfs_thread(void* arg) {
    struct thread_args* args = (struct thread_args*) arg;
    int id = args->id;
    struct grafo* grafo = args->grafo;

    while (1) {
        struct fila* filaAtual = *args->filaAtualPtr;

        int tamFila = (filaAtual->frente == -1) ? 0 : (filaAtual->atras - filaAtual->frente + 1);

        if (tamFila <= 0) break;

        int porThread = (tamFila + nThreads - 1) / nThreads;
        int ini = filaAtual->frente + id * porThread;
        int fim = ini + porThread;
        if (fim > filaAtual->atras + 1) fim = filaAtual->atras + 1;

        for (int i = ini; i < fim; i++) {
            if (i > filaAtual->atras) break;

            int v = filaAtual->itens[i];
            if (v < 0 || v >= grafo->nVertices) {
                fprintf(stderr, "[ERRO] Vértice inválido: %d\n", v);
                continue;
            }
            struct no* temp = grafo->listaAdj[v];
            while (temp) {
                int adj = temp->vertice;
                pthread_mutex_lock(&args->mutexesVisitado[adj]);
                if (!grafo->visitado[adj]) {
                    grafo->visitado[adj] = 1;
                    pthread_mutex_unlock(&args->mutexesVisitado[adj]);

                    printf("visitado %d pela thread %d\n", adj, id);

                    pthread_mutex_lock(args->mutexFila);
                    enfileirar(args->filaProxima, adj);
                    pthread_mutex_unlock(args->mutexFila);
                } else {
                    pthread_mutex_unlock(&args->mutexesVisitado[adj]);
                }
                temp = temp->proximo;
            }
        }

        barreira(nThreads);

        // Apenas a thread 0 troca as filas
        if (id == 0) {
            if (ehVazio(args->filaProxima)) {
                (*args->filaAtualPtr)->frente = (*args->filaAtualPtr)->atras = -1;
            } else {
                struct fila* temp = *args->filaAtualPtr;
                *args->filaAtualPtr = args->filaProxima;
                args->filaProxima = temp;
                args->filaProxima->frente = args->filaProxima->atras = -1;
            }
        }

        barreira(nThreads);

        if (ehVazio(*args->filaAtualPtr)) break;
    }

    return NULL;
}


// ---------- MAIN ----------
int main(int argc, char* argv[]) {

    clock_t tempoInicio, tempoFinal;
    tempoInicio = clock();

    if (argc < 3) {
        printf("Uso: %s <arquivo_grafo> <nThreads> [verticeInicial]\n", argv[0]);
        return 1;
    }

    const char* arquivo = argv[1];
    nThreads = atoi(argv[2]);
    int verticeInicial = (argc >= 4) ? atoi(argv[3]) : 0;

    if (nThreads < 1 || nThreads > MAX_THREADS) {
        printf("Número de threads deve estar entre 1 e %d\n", MAX_THREADS);
        return 1;
    }

    struct grafo* grafo = lerGrafoBinario(arquivo);
    
    pthread_mutex_t* mutexesVisitado = malloc(grafo->nVertices * sizeof(pthread_mutex_t));
    for (int i = 0; i < grafo->nVertices; i++) pthread_mutex_init(&mutexesVisitado[i], NULL);
    
    grafo->visitado[verticeInicial] = 1;

    struct fila* filaAtualPtr = criaFila();
    struct fila* filaProxima = criaFila();
    enfileirar(filaAtualPtr, verticeInicial);

    pthread_t threads[MAX_THREADS];
    struct thread_args args[MAX_THREADS];
    pthread_mutex_t mutexFila = PTHREAD_MUTEX_INITIALIZER;

    for (int i = 0; i < nThreads; i++) {
        args[i].id = i;
        args[i].grafo = grafo;
        args[i].filaAtualPtr = &filaAtualPtr;
        args[i].filaProxima = filaProxima;
        args[i].mutexesVisitado = mutexesVisitado;
        args[i].mutexFila = &mutexFila;
        pthread_create(&threads[i], NULL, bfs_thread, &args[i]);
    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    tempoFinal = clock();
    double tempoTotal = (double)(tempoFinal - tempoInicio) / CLOCKS_PER_SEC;
    printf("Tempo total para calcular BFS concorrente com %d: %.3lf segundos\n", nThreads, tempoTotal);    

    if (filaAtualPtr == filaProxima) {
        liberarFila(filaAtualPtr);
    } else {
        liberarFila(filaAtualPtr);
        liberarFila(filaProxima);
    }

    int n = grafo->nVertices;

    liberarGrafo(grafo);

    for (int i = 0; i < n; i++)
        pthread_mutex_destroy(&mutexesVisitado[i]);
    free(mutexesVisitado);

    return 0;
}