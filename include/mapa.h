#ifndef MAPA_H
#define MAPA_H

#include "raylib.h"
#include <stdbool.h>

#define LARGURA_MAPA 32
#define ALTURA_MAPA 24
#define TAMANHO_TILE 32
#define MAX_TIPOS_TILE 17

typedef enum {
    TILE_CHAO = 0,
    TILE_PAREDE = 1,
    TILE_MERCADO = 2,
    TILE_POSTO = 3,
    TILE_LABORATORIO = 4,
    TILE_PREDIO_AMARELO = 5,
    TILE_CHAO_MERCADO = 6,
    TILE_PRATELEIRA_MERCADO = 7,
    TILE_CAIXA_MERCADO = 8,
    TILE_CHAO_LAB = 9,
    TILE_PORTA_MERCADO = 10,
    TILE_PORTA_LAB = 11,
    TILE_PAREDE_INVISIVEL = 12,
    TILE_PORTA_BANHEIRO = 13,
    TILE_CHAO_DEPOSITO = 14,
    TILE_LOJA = 15,
    TILE_CHAO_BANHEIRO = 16,
} TipoTile;

typedef struct {
    int tipo;
    int linhaInicio;
    int colunaInicio;
    int larguraTiles;
    int alturaTiles;
    bool desenhado;
} Predio;

typedef struct {
    int tiles[ALTURA_MAPA][LARGURA_MAPA];
    int largura;
    int altura;
    int tamanhoTile;
} Mapa;

Mapa* criarMapa(void);
void destruirMapa(Mapa* mapa);
bool carregarMapaDeArquivo(Mapa* mapa, const char* caminhoArquivo);
void inicializarMapaPadrao(Mapa* mapa);

void desenharMapaTiles(const Mapa* mapa, Texture2D texturasTiles[]);

bool verificarColisaoMapa(const Mapa* mapa, Vector2 posicao, float raio);
bool verificarColisaoMovimento(const Mapa* mapa, Vector2 posicaoAtual, Vector2 novaPosicao, float raio);
bool isTileSolido(const Mapa* mapa, int linhaGrid, int colunaGrid);

Vector2 pixelParaGrid(Vector2 posicaoPixel);
Vector2 gridParaPixel(int linhaGrid, int colunaGrid);
bool posicaoValidaNoMapa(int linhaGrid, int colunaGrid);
Vector2 gerarPosicaoValidaSpawn(const Mapa* mapa, float raio);
Vector2 certificarPosicaoWalkable(const Mapa* mapa, Vector2 posicaoDesejada, float raio);
bool isPosicaoWalkable(const Mapa* mapa, Vector2 posicao, float raio);

#endif
