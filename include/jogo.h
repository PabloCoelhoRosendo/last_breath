// include/jogo.h

#ifndef JOGO_H
#define JOGO_H

#include "raylib.h"

#define TAMANHO_MAPA 20
#define TAMANHO_CELULA 40 // Tamanho de cada célula do mapa em pixels

// Declaração externa da matriz global do mapa
extern int mapaDoJogo[TAMANHO_MAPA][TAMANHO_MAPA];

// Enum para tipos de arma
typedef enum {
    ARMA_NENHUMA = 0,
    ARMA_PISTOL = 1,
    ARMA_SHOTGUN = 2,
    ARMA_SMG = 3
} TipoArma;

// Estrutura para Arma
typedef struct {
    TipoArma tipo;
    int dano;
    int taxaTiroMS;      // Taxa de tiro em milissegundos
    int penteMax;        // Capacidade máxima do pente
    int penteAtual;      // Munição atual no pente
    int municaoTotal;    // Munição total (reserva)
    float tempoRecarga;  // Tempo de recarga em segundos
    float cooldown;      // Cooldown atual (para controlar taxa de tiro)
} Arma;

// 1. Estrutura do Jogador (Henrique/Geral - Requisito: Structs)
typedef struct {
    Vector2 posicao;
    int vida;
    Arma slots[3];           // 3 slots de arma (índices 0, 1, 2)
    int slotAtivo;           // Slot da arma equipada (0, 1 ou 2)
    int direcaoVertical;     // 0 = frente, 1 = costas
    int direcaoHorizontal;   // 0 = esquerda, 1 = direita
    Texture2D spriteAtual;   // Sprite atual sendo exibido
    float tempoTotal;        // Tempo total de jogo
    int fase;                // Fase atual (1-3)
    float velocidadeBase;    // Velocidade base de movimento
    bool estaRecarregando;   // Flag se está recarregando
    float tempoRecargaAtual; // Tempo restante de recarga
    bool tempoJaSalvo;       // Flag para evitar salvar múltiplas vezes
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
    int direcaoVertical;      // 0 = frente, 1 = costas
    int direcaoHorizontal;    // 0 = esquerda, 1 = direita
    int tipoSkin;             // 0-4 (qual dos 5 tipos de zumbi)
    Texture2D spriteFrenteDireita;
    Texture2D spriteFrenteEsquerda;
    Texture2D spriteCostasDireita;
    Texture2D spriteCostasEsquerda;
    Texture2D spriteAtual;    // Sprite sendo renderizado
    struct Zumbi *proximo;    // Ponteiro para o próximo zumbi
} Zumbi;

// 3. Estrutura para Balas ( - Pablo - Requisito: Structs, Lista Encadeada)
typedef struct Bala {
    Vector2 posicao;
    Vector2 velocidade;
    int tipo;           // 0 = jogador, 1 = boss
    float dano;         // Quantidade de dano causado
    float raio;         // Raio da bala para colisão
    float tempoVida;    // Tempo que a bala existe (para efeitos ou limitar alcance)
    struct Bala *proximo;
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

// Protótipos do Sistema de Armas
void inicializarArma(Arma *arma, TipoArma tipo);
void equiparArma(Player *jogador, int slot);
void recarregarArma(Player *jogador);
void atirarArma(Player *jogador, struct Bala **balas, Vector2 alvo);

// Protótipos do Módulo de Balas ( - Pablo)
void adicionarBala(struct Bala **cabeca, Vector2 posInicial, Vector2 alvo, int tipo, float dano);
void atualizarBalas(struct Bala **cabeca);

// Protótipos do Módulo de Zumbis ( - Victor)
void adicionarZumbi(struct Zumbi **cabeca, Vector2 posInicial, Texture2D sprites[][4]);
void atualizarZumbis(struct Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime);
void desenharZumbis(struct Zumbi *cabeca);
void liberarZumbis(struct Zumbi **cabeca);

// Protótipos das Funções de Colisão
int verificarColisaoCirculos(Vector2 pos1, float raio1, Vector2 pos2, float raio2);
void verificarColisoesBalaZumbi(struct Bala **balas, struct Zumbi **zumbis, Player *jogador);
void verificarColisoesJogadorZumbi(Player *jogador, struct Zumbi *zumbis);
void verificarColisoesZumbiZumbi(struct Zumbi *zumbis);

#endif