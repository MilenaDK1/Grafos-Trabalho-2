#include "blocos.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct grafo_t {
    int vertices;
    int arestas;
    int **matriz_adj;
} grafo_t;

grafo_t *ler_grafo(void);
void imprimir_grafo(grafo_t *grafo);

grafo_t *ler_grafo(void) {
    int n;
    scanf("%d", &n); // num de vertices
    printf("n: %d\n", n);
    grafo_t *grafo = malloc(sizeof(grafo_t));
    grafo->vertices = n;
    grafo->arestas = 0;
    grafo->matriz_adj = malloc((size_t)n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        grafo->matriz_adj[i] = calloc((size_t)n, sizeof(int));
    }
    
    int v1, v2;
    while(scanf("%d %d", &v1, &v2) != EOF) {
        grafo->matriz_adj[v1 - 1][v2 - 1] = 1;
        grafo->matriz_adj[v2 - 1][v1 - 1] = 1;
        printf("v1: %d, v2: %d\n", v1, v2);
        grafo->arestas++;
    }

    return grafo;
}

void imprimir_grafo(grafo_t *grafo) {
    printf("Vertices: %d\n", grafo->vertices);
    printf("Arestas: %d\n", grafo->arestas);

    // imprime matriz de adjacencia
    for(int i = 0; i < grafo->vertices; i++) {
        for(int j = 0; j < grafo->vertices; j++) {
            printf("%d ", grafo->matriz_adj[i][j]);
        }
        printf("\n");
    }
}
    


info_t *info_blocos(unsigned int *num_blocos){
    
    grafo_t *grafo = ler_grafo();
    imprimir_grafo(grafo);
    
    return NULL;

}