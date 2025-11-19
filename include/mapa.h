#ifndef MAPA_H
#define MAPA_H

#include "raylib.h"
#include <stdbool.h>

// Constantes do mapa
#define LARGURA_MAPA 32
#define ALTURA_MAPA 24
#define TAMANHO_TILE 32
#define MAX_TIPOS_TILE 13

// Tipos de tiles
typedef enum {
    TILE_CHAO = 0,              // Chão genérico / Rua (Fase 2)
    TILE_PAREDE = 1,
    TILE_PREDIO_VERMELHO = 2,
    TILE_PREDIO_AZUL = 3,
    TILE_PREDIO_VERDE = 4,
    TILE_PREDIO_AMARELO = 5,
    TILE_CHAO_MERCADO = 6,      // Chão do mercado (Fase 1)
    TILE_PRATELEIRA_MERCADO = 7,  // Prateleira do mercado (obstáculo)
    TILE_CAIXA_MERCADO = 8,       // Caixa do mercado (obstáculo)
    TILE_CHAO_LAB = 9,            // Chão do laboratório (Fase 3)
    TILE_PORTA_MERCADO = 10,      // Porta do mercado (decoração Fase 1)
    TILE_PORTA_LAB = 11,          // Porta do laboratório (ponto invisível Fase 2)
    TILE_PAREDE_INVISIVEL = 12,   // Parede invisível (colisão sem textura - bordas Fase 1)
    // Adicione mais tipos conforme necessário
} TipoTile;

// Estrutura para representar um prédio como imagem completa
typedef struct {
    int tipo;           // Tipo do prédio (2=Mercado, 3=Posto, 4=Laboratório)
    int linhaInicio;    // Linha inicial na matriz
    int colunaInicio;   // Coluna inicial na matriz
    int larguraTiles;   // Largura em número de tiles
    int alturaTiles;    // Altura em número de tiles
    bool desenhado;     // Flag para evitar desenhar múltiplas vezes
} Predio;

// Estrutura do mapa
typedef struct {
    int tiles[ALTURA_MAPA][LARGURA_MAPA];  // Matriz de tiles
    int largura;                            // Largura em tiles
    int altura;                             // Altura em tiles
    int tamanhoTile;                        // Tamanho de cada tile em pixels
} Mapa;

// Funções de inicialização e carregamento
Mapa* criarMapa(void);
void destruirMapa(Mapa* mapa);
bool carregarMapaDeArquivo(Mapa* mapa, const char* caminhoArquivo);
void inicializarMapaPadrao(Mapa* mapa);

// Funções de renderização
void desenharMapaTiles(const Mapa* mapa, Texture2D texturasTiles[]);

// Funções de colisão
bool verificarColisaoMapa(const Mapa* mapa, Vector2 posicao, float raio);
bool verificarColisaoMovimento(const Mapa* mapa, Vector2 posicaoAtual, Vector2 novaPosicao, float raio);
bool isTileSolido(const Mapa* mapa, int linhaGrid, int colunaGrid);

// Funções utilitárias
Vector2 pixelParaGrid(Vector2 posicaoPixel);
Vector2 gridParaPixel(int linhaGrid, int colunaGrid);
bool posicaoValidaNoMapa(int linhaGrid, int colunaGrid);
Vector2 gerarPosicaoValidaSpawn(const Mapa* mapa, float raio);

#endif // MAPA_H
