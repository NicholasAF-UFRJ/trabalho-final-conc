# trabalho-final-conc
Repositório para o trabalho final da disciplina de Programação Concorrente de 2025.1.


Consiste na implementação do algoritmo de busca em largura (Breadth First Search - BFS) em C, de forma sequencial e concorrente. 

Passo a passo:

1) Compilar todos os programas:

```bash
make
```

2) Execução (exemplos):

```bash
# BFS sequencial
./bfsSeq arvore

# BFS alocação estática de tarefas/thread com 4 threads
./bfsConcEstat arvore 4

# BFS alocação dinâmica de tarefas/thread com 4 threads
./bfsConcDin arvore 4
```

3) Limpar binários e executáveis:

```bash
make clean
```