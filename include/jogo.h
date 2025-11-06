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
    float tempoTotal;      // Tempo total de jogo
    int fase;              // Fase atual (1-3)
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

// 2.1 Estrutura para Zumbi Forte (Victor - Requisito: Structs, Lista Encadeada)
typedef struct ZumbiForte {
    Vector2 posicao;
    Vector2 velocidade;
    int vida;               // Vida maior que zumbi normal
    float raio;             // Raio maior (zumbi maior)
    float velocidadeBase;   // Velocidade reduzida (mais lento mas resistente)
    float tempoDesvio;
    float anguloDesvio;
    int tipoMovimento;
    int dano;               // Dano maior ao colidir com jogador
    float armadura;         // Redução de dano recebido (0.0 a 1.0)
    Color cor;              // Cor diferenciada (vermelho escuro)
    struct ZumbiForte *proximo;
} ZumbiForte;

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

// 4. Estrutura para Chave (Sistema de Progressão)
typedef struct {
    Vector2 posicao;
    float raio;
    bool ativa;
    bool coletada;
} Chave;

// 5. Estrutura para Porta (Sistema de Progressão)
typedef struct {
    Vector2 posicao;
    float largura;
    float altura;
    bool trancada;
    bool aberta;
} Porta;

// 6. Estrutura para Mapa (Sistema de Progressão)
typedef struct {
    int id;
    Color corFundo;
    Vector2 spawnJogador;
    int numZumbis;
    int numZumbisFortes;
    bool temChave;
    bool temPorta;
} Mapa;


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
void adicionarBala(struct Bala **cabeca, Vector2 posInicial, Vector2 alvo, int tipo, float dano);
void atualizarBalas(struct Bala **cabeca);

// Protótipos do Módulo de Zumbis ( - Victor)
void adicionarZumbi(struct Zumbi **cabeca, Vector2 posInicial, Texture2D sprites[][4]);
void atualizarZumbis(struct Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime);
void desenharZumbis(struct Zumbi *cabeca);
void liberarZumbis(struct Zumbi **cabeca);

// Protótipos do Módulo de Zumbis Fortes ( - Victor)
void adicionarZumbiForte(struct ZumbiForte **cabeca, Vector2 posInicial);
void atualizarZumbisFortes(struct ZumbiForte **cabeca, Vector2 posicaoJogador, float deltaTime);
void desenharZumbisFortes(struct ZumbiForte *cabeca);
void liberarZumbisFortes(struct ZumbiForte **cabeca);

// Protótipos das Funções de Colisão
int verificarColisaoCirculos(Vector2 pos1, float raio1, Vector2 pos2, float raio2);
void verificarColisoesBalaZumbi(struct Bala **balas, struct Zumbi **zumbis, Player *jogador);
void verificarColisoesJogadorZumbi(Player *jogador, struct Zumbi *zumbis);
void verificarColisoesZumbiZumbi(struct Zumbi *zumbis);
void verificarColisoesBalaZumbiForte(struct Bala **balas, struct ZumbiForte **zumbisFortes, Player *jogador);
void verificarColisoesJogadorZumbiForte(Player *jogador, struct ZumbiForte *zumbisFortes);
void verificarColisoesZumbiForteZumbiForte(struct ZumbiForte *zumbisFortes);

// Protótipos das Funções de Chave
void criarChave(Chave* chave, Vector2 posicao);
void desenharChave(Chave* chave);
bool verificarColetaChave(Chave* chave, Player* jogador);

// Protótipos das Funções de Porta
void criarPorta(Porta* porta, Vector2 posicao);
void desenharPorta(Porta* porta);
bool verificarInteracaoPorta(Porta* porta, Player* jogador, bool temChave);

// Protótipos das Funções de Mapa
void carregarMapa(Mapa* mapa, int idMapa);

#endif