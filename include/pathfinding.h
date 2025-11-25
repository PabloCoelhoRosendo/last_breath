// include/pathfinding.h
// Sistema de Pathfinding A* para zumbis

#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "raylib.h"
#include "mapa.h"
#include <stdbool.h>

// Tamanho máximo do caminho
#define MAX_PATH_LENGTH 64

// Estrutura para representar um nó no grid do pathfinding
typedef struct PathNode {
    int x, y;           // Posição no grid
    float g;            // Custo do início até este nó
    float h;            // Heurística (estimativa até o destino)
    float f;            // f = g + h
    int parentX;        // Posição X do nó pai
    int parentY;        // Posição Y do nó pai
    bool fechado;       // Se já foi processado
    bool aberto;        // Se está na lista aberta
} PathNode;

// Estrutura para armazenar um caminho calculado
typedef struct {
    Vector2 pontos[MAX_PATH_LENGTH];  // Pontos do caminho em coordenadas de pixel
    int tamanho;                       // Número de pontos no caminho
    int indiceAtual;                   // Índice do ponto atual sendo seguido
    float tempoRecalculo;              // Timer para recalcular o caminho
    bool valido;                       // Se o caminho é válido
} Caminho;

// Estrutura para o sistema de pathfinding (grid de nós)
typedef struct {
    PathNode nos[ALTURA_MAPA][LARGURA_MAPA];  // Grid de nós
    bool inicializado;
} PathfindingGrid;

// Funções principais do pathfinding
void inicializarPathfinding(PathfindingGrid *grid);
void resetarGrid(PathfindingGrid *grid);

// Calcula o caminho do ponto inicial ao destino
// Retorna true se encontrou caminho, false caso contrário
bool calcularCaminho(PathfindingGrid *grid, const Mapa *mapa,
                     Vector2 inicio, Vector2 destino, Caminho *caminhoSaida);

// Obtém a próxima direção a seguir baseado no caminho
// Retorna vetor normalizado da direção
Vector2 obterDirecaoCaminho(Caminho *caminho, Vector2 posicaoAtual);

// Atualiza o seguimento do caminho (avança para próximo ponto se necessário)
void atualizarSeguimentoCaminho(Caminho *caminho, Vector2 posicaoAtual, float distanciaMinima);

// Verifica se precisa recalcular o caminho
bool precisaRecalcularCaminho(Caminho *caminho, Vector2 destinoAtual, float deltaTime);

// Funções utilitárias
float calcularHeuristica(int x1, int y1, int x2, int y2);
bool posicaoValida(const Mapa *mapa, int x, int y);

#endif // PATHFINDING_H
