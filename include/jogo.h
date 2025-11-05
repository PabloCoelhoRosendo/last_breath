// include/jogo.h

#ifndef JOGO_H
#define JOGO_H

#include "raylib.h"

#define TAMANHO_MAPA 20
#define TAMANHO_CELULA 40 // Tamanho de cada célula do mapa em pixels

// Declaração externa da matriz global do mapa
extern int mapaDoJogo[TAMANHO_MAPA][TAMANHO_MAPA];

// 1. Estrutura do Jogador (Henrique/Geral - Requisito: Structs)
typedef struct {
    Vector2 posicao;
    int vida;
    int municao;
    int pontos;
    int direcaoVertical;   // 0 = frente, 1 = costas
    int direcaoHorizontal; // 0 = esquerda, 1 = direita
    Texture2D spriteAtual; // Sprite atual sendo exibido
} Player;

// 2. Estrutura para Zumbis (Victor - Requisito: Structs, Lista Encadeada)
typedef struct Zumbi {
    Vector2 posicao;
    Vector2 velocidade;
    int vida;
    float raio;
    float velocidadeBase;     // Velocidade original do zumbi
    float tempoDesvio;        // Timer para mudança de direção
    float anguloDesvio;       // Ângulo de desvio aleatório
    int tipoMovimento;        // 0=direto, 1=zigzag, 2=circular, 3=imprevisível
    struct Zumbi *proximo;    // Ponteiro para o próximo zumbi
} Zumbi;

// 3. Estrutura para Balas ( - Pablo - Requisito: Structs, Lista Encadeada)
typedef struct Bala {
    Vector2 posicao;
    Vector2 velocidade;
    struct Bala *proximo; // Ponteiro para a próxima bala (Requisito: Ponteiros)
} Bala;


// Protótipos das funções do mapa
void mapa(int mapa[TAMANHO_MAPA][TAMANHO_MAPA]);
void desenharMapa(int mapa[TAMANHO_MAPA][TAMANHO_MAPA], Texture2D texturaMapa);
int verificarColisaoMapa(Vector2 novaPosicao, float raio, int mapa[TAMANHO_MAPA][TAMANHO_MAPA]);
Vector2 gerarPosicaoValidaSpawn(int mapa[TAMANHO_MAPA][TAMANHO_MAPA], float raio);

// Protótipos das funções principais (a serem implementadas em src/jogo.c)
void iniciarJogo(Player *jogador);
void atualizarJogo(Player *jogador, struct Zumbi **zumbis, struct Bala **balas);
void desenharJogo(Player *jogador, struct Zumbi *zumbis, struct Bala *balas, Texture2D texturaMapa);

// Protótipos do Módulo de Balas ( - Pablo)
void adicionarBala(struct Bala **cabeca, Vector2 posInicial, Vector2 alvo);
void atualizarBalas(struct Bala **cabeca);

// Protótipos do Módulo de Zumbis ( - Victor)
void adicionarZumbi(struct Zumbi **cabeca, Vector2 posInicial);
void atualizarZumbis(struct Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime);
void desenharZumbis(struct Zumbi *cabeca);
void liberarZumbis(struct Zumbi **cabeca);

// Protótipos das Funções de Colisão
int verificarColisaoCirculos(Vector2 pos1, float raio1, Vector2 pos2, float raio2);
void verificarColisoesBalaZumbi(struct Bala **balas, struct Zumbi **zumbis, Player *jogador);
void verificarColisoesJogadorZumbi(Player *jogador, struct Zumbi *zumbis);
void verificarColisoesZumbiZumbi(struct Zumbi *zumbis);

#endif