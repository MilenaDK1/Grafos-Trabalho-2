#include "blocos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct vertice_t {
    int label;
    char estado;

    int component; 
    int pre;
    int pos;
    struct vertice_t *pai;

    int lv;
    int low;
    int eh_articulacao;
    int qtd_filhos;

} vertice_t;

typedef struct grafo_t {
    size_t n_vertices;
    size_t m_arestas;
    int **matriz_adj;
    vertice_t **vertices;
    size_t comp_num;
    int tstmp;
} grafo_t;

typedef struct bloco_t {
    vertice_t **vertices;
    size_t n;
} bloco_t;

// macro para calcular o menor de dois valores
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

grafo_t *ler_grafo(void);
void imprimir_grafo(grafo_t *grafo);
void dfs(grafo_t *grafo);
void dfsAux(grafo_t *grafo, vertice_t *r);
void encontraArticulacoes(grafo_t *grafo);
void lowPoint(grafo_t *grafo);
void lowPointAux(grafo_t *grafo, vertice_t *r);
void decomposeDif(grafo_t *grafo);
void decomposeAux(grafo_t *grafo, vertice_t *r, int c);
int comparePostOrderR(const void *a, const void *b);
bloco_t *separaBlocos(grafo_t *grafo);
void printBlocks(bloco_t *bls, size_t n);

grafo_t *ler_grafo(void) {
    int n;
    scanf("%d", &n); // num de vertices
    printf("n: %d\n", n);
    grafo_t *grafo = malloc(sizeof(grafo_t));
    grafo->n_vertices = (size_t)n;
    grafo->m_arestas = 0;
    grafo->matriz_adj = malloc((size_t)n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        grafo->matriz_adj[i] = calloc((size_t)n, sizeof(int));
    }
    
    // preenche matriz de adjacencia
    int v1, v2;
    while(scanf("%d %d", &v1, &v2) != EOF) {
        grafo->matriz_adj[v1 - 1][v2 - 1] = 1;
        grafo->matriz_adj[v2 - 1][v1 - 1] = 1;
        printf("v1: %d, v2: %d\n", v1, v2);
        grafo->m_arestas++;
    }

    // cria a lista de vertices
    grafo->vertices = malloc(sizeof(vertice_t *) * grafo->n_vertices);
    if (!grafo->vertices)
    {
        perror("Falha ao alocar memória");
        exit(EXIT_FAILURE);
    }
    
    for (size_t i = 0; i < grafo->n_vertices; i++){
        grafo->vertices[i] = malloc(sizeof(vertice_t));
        if (!grafo->vertices[i])
        {
            perror("Falha ao alocar memória");
            exit(EXIT_FAILURE);
        }
        grafo->vertices[i]->label = (int)i;
        grafo->vertices[i]->eh_articulacao = 0;
    }

    return grafo;
}

void imprimir_grafo(grafo_t *grafo) {
    printf("Vertices: %ld\n", grafo->n_vertices);
    printf("Arestas: %ld\n", grafo->m_arestas);

    // imprime matriz de adjacencia
    for(size_t i = 0; i < grafo->n_vertices; i++) {
        for(size_t j = 0; j < grafo->n_vertices; j++) {
            printf("%d ", grafo->matriz_adj[i][j]);
        }
        printf("\n");
    }

    // imprime lista de vertices
    for (size_t i = 0; i < grafo->n_vertices; i++) {
        printf("Vertice %d\n", grafo->vertices[i]->label);
        printf("Pre/Pos: %d/%d\n", grafo->vertices[i]->pre, grafo->vertices[i]->pos);
        printf("Lv/Low: %d/%d\n", grafo->vertices[i]->lv ,grafo->vertices[i]->low);
        printf("Articulacao: %d\n", grafo->vertices[i]->eh_articulacao);
        printf("Componente: %d\n", grafo->vertices[i]->component);
        if (grafo->vertices[i]->pai)
            printf("Pai: %d\n", grafo->vertices[i]->pai->label);
        else    
            printf("Pai: NULL\n");
        printf("Estado: %d\n", grafo->vertices[i]->estado); 
        printf("--------------\n\n");
    }
}

// busca em profundidade    
void dfs(grafo_t *grafo)
{
    for (size_t i = 0; i < grafo->n_vertices; i++) {
        grafo->vertices[i]->estado = 0;
    }

    grafo->tstmp = 0;

    for (size_t i = 0; i < grafo->n_vertices; i++) {
        if (grafo->vertices[i]->estado == 0){
            grafo->vertices[i]->pai = NULL;
            dfsAux(grafo, grafo->vertices[i]);
        }
    }
}

// auxiliar da busca em profundidade
void dfsAux(grafo_t *grafo, vertice_t *r) {
    r->pre = ++(grafo->tstmp);
    r->estado = 1;

    for (size_t i = 0; i < grafo->n_vertices; i++) {
        if ((grafo->matriz_adj[r->label][i] == 1) && (grafo->vertices[i]->estado == 0))
        {
            grafo->vertices[i]->pai = r;
            dfsAux(grafo, grafo->vertices[i]);
        }
    }
    r->pos = ++(grafo->tstmp);
    r->estado = 2;
}

// encontra os vertices de corte do grafo
void encontraArticulacoes(grafo_t *grafo) {
    lowPoint(grafo);

    for (size_t i = 0; i < grafo->n_vertices; i++) {
        vertice_t *u = grafo->vertices[i];
        if (u->pai == NULL) { // eh raiz
            if (u->qtd_filhos >= 2) {
                u->eh_articulacao = 1;
            }
        }
        else { // percorre os filhos
            for (size_t j = 0; j < grafo->n_vertices; j++) {
                vertice_t *v = grafo->vertices[j];
                if ((grafo->matriz_adj[u->label][v->label] == 1) && (u == v->pai)) {
                    if (u->lv <= v->low) {
                        u->eh_articulacao = 1;
                        break;
                    }
                }
            }
        }
    }
}

// calcula o low point para cada vertice
void lowPoint(grafo_t *grafo) {
    // inicializa
    for (size_t i = 0; i < grafo->n_vertices; i++) {
        grafo->vertices[i]->estado = 0;
        grafo->vertices[i]->pai = NULL;
        grafo->vertices[i]->qtd_filhos = 0;
    }

    // percorre os vertices
    for (size_t i = 0; i < grafo->n_vertices; i++) {
        if (grafo->vertices[i]->estado == 0) {
            grafo->vertices[i]->low = grafo->vertices[i]->lv = 0;
            lowPointAux(grafo, grafo->vertices[i]);
        }
    }
}

// funcao auxiliar para o low point
void lowPointAux(grafo_t *grafo, vertice_t *r) {
    r->estado = 1;

    // percorre a vizinhanca de r
    for (size_t i = 0; i < grafo->n_vertices; i++) {
        if (grafo->matriz_adj[r->label][grafo->vertices[i]->label] == 1)
        {

            vertice_t *w = grafo->vertices[i];
            if ((w->estado == 1) && (w != r->pai))
            {
                r->low = min(r->low, w->lv);
            }
            else if (w->estado == 0)
            {
                r->qtd_filhos++;
                w->pai = r;
                w->low = w->lv = r->lv + 1;
                lowPointAux(grafo, w);
                r->low = min(r->low, w->low);
            }
        }
    }
    r->estado = 2;
}

int comparePostOrderR(const void *a, const void *b)
// compara os vertices pela pos ordem
{
    return (((const vertice_t *)(*((const vertice_t *const *)b)))->pos - ((const vertice_t *)(*((const vertice_t *const *)a)))->pos);
}

void decomposeAux(grafo_t *grafo, vertice_t *r, int c)
{
    r->estado = 1;
    for (size_t i = 0; i < grafo->n_vertices; i++)
    {
        vertice_t *v = grafo->vertices[i];
        if ((grafo->matriz_adj[r->label][v->label] == 1) && (v->estado == 0) && (!v->eh_articulacao)) {
            decomposeAux(grafo, grafo->vertices[i], c);
        }
    }
    r->component = c;
    r->estado = 2;
}

// faz a decomposicao do grafo em componentes ignorando os vertices de corte
void decomposeDif(grafo_t *grafo) {
    // reverso da pos order da busca em profundidade
    vertice_t **l = malloc(sizeof(vertice_t *) * grafo->n_vertices);
    memcpy(l, grafo->vertices, grafo->n_vertices * sizeof(vertice_t *));
    qsort(l, grafo->n_vertices, sizeof(vertice_t *), comparePostOrderR);

    for (size_t i = 0; i < grafo->n_vertices; i++) {
        grafo->vertices[i]->estado = 0;
        grafo->vertices[i]->component = 0;
    }

    int c = 0;

    for (size_t i = 0; i < grafo->n_vertices; i++) {
        if ((l[i]->estado == 0) && (!l[i]->eh_articulacao))
        {
            decomposeAux(grafo, l[i], ++c);
        }
    }

    grafo->comp_num = (size_t)c;
}

// separa os blocos do grafo
bloco_t *separaBlocos(grafo_t *grafo) {
    bloco_t *bls = malloc(sizeof(bloco_t) * grafo->comp_num);
    for (size_t i = 0; i < grafo->comp_num; i++) {
        bls[i].vertices = malloc(sizeof(vertice_t *) * grafo->n_vertices);
        bls[i].n = 0;
    }

    for (size_t i = 0; i < grafo->n_vertices; i++) {
        vertice_t *u = grafo->vertices[i];
        // vertices de corte fazem parte de mais de uma componente
        if (u->eh_articulacao) {
            // percorre os filhos
            for (size_t j = 0; j < grafo->n_vertices; j++) {
                vertice_t *v = grafo->vertices[j];
                if ((grafo->matriz_adj[u->label][v->label] == 1) && !v->eh_articulacao) {
                    size_t comp_size = bls[v->component - 1].n;
                    if (bls[v->component - 1].vertices[comp_size - 1] != u) {
                        bls[v->component - 1].vertices[comp_size] = u;
                        bls[v->component - 1].n++;
                    }
                }
            }
        }
        else {
            bls[u->component - 1].vertices[bls[u->component - 1].n] = u;
            bls[u->component - 1].n++;
        }
    }
    return bls;
}

void printBlocks(bloco_t *bls, size_t n) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < bls[i].n; j++) {
            printf("%d ", bls[i].vertices[j]->label);
        }
        printf("\n");
    }
}

info_t *info_blocos(unsigned int *num_blocos) { 
    grafo_t *grafo = ler_grafo();
    dfs(grafo);
    encontraArticulacoes(grafo);
    decomposeDif(grafo);
    //imprimir_grafo(grafo);
    bloco_t *bls = separaBlocos(grafo);
    printBlocks(bls, grafo->comp_num);

    info_t *info = malloc(sizeof(info_t) * grafo->comp_num);
    for (size_t i = 0; i < grafo->comp_num; i++) {
        printf("Componente %ld\n", i + 1);
        info[i].vertices = (unsigned int) bls[i].n;
        info[i].arestas = 0;
        
        for (size_t j = 0; j < bls[i].n; j++) {
            for (size_t k = 0; k < bls[i].n; k++) {
                printf("Verificando aresta %d %d\n", bls[i].vertices[j]->label, bls[i].vertices[k]->label);
                if (grafo->matriz_adj[bls[i].vertices[j]->label][bls[i].vertices[k]->label] == 1) {
                    printf("Aresta encontrada\n");
                    info[i].arestas++;
                }
            }
        }
        
        info[i].arestas /= 2;
        printf("Vertices: %d\n", info[i].vertices);
        printf("Arestas: %d\n", info[i].arestas);
        printf("--------------\n\n");
    }

    *num_blocos = (unsigned int) grafo->comp_num;
    return info;
}