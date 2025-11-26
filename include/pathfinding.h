#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "raylib.h"
#include "mapa.h"
#include <stdbool.h>

#define MAX_PATH_LENGTH 64

typedef struct PathNode {
    int x, y;
    float g;
    float h;
    float f;
    int parentX;
    int parentY;
    bool fechado;
    bool aberto;
} PathNode;

typedef struct {
    Vector2 pontos[MAX_PATH_LENGTH];
    int tamanho;
    int indiceAtual;
    float tempoRecalculo;
    bool valido;
} Caminho;

typedef struct {
    PathNode nos[ALTURA_MAPA][LARGURA_MAPA];
    bool inicializado;
} PathfindingGrid;

void inicializarPathfinding(PathfindingGrid *grid);
void resetarGrid(PathfindingGrid *grid);

bool calcularCaminho(PathfindingGrid *grid, const Mapa *mapa,
                     Vector2 inicio, Vector2 destino, Caminho *caminhoSaida);

Vector2 obterDirecaoCaminho(Caminho *caminho, Vector2 posicaoAtual);

void atualizarSeguimentoCaminho(Caminho *caminho, Vector2 posicaoAtual, float distanciaMinima);

bool precisaRecalcularCaminho(Caminho *caminho, Vector2 destinoAtual, float deltaTime);

float calcularHeuristica(int x1, int y1, int x2, int y2);
bool posicaoValida(const Mapa *mapa, int x, int y);

#endif
