#ifndef JOGO_H
#define JOGO_H

#include "raylib.h"
#include "mapa.h"
#include "recursos.h"
#include "pathfinding.h"

typedef enum {
    ESTADO_MENU = 0,
    ESTADO_JOGANDO = 1,
    ESTADO_PAUSADO = 2
} EstadoJogo;

typedef enum {
    HORDA_NAO_INICIADA = 0,
    HORDA_EM_PROGRESSO = 1,
    HORDA_COMPLETA = 2,
    HORDA_INTERVALO = 3
} EstadoHorda;

typedef enum {
    ARMA_NENHUMA = 0,
    ARMA_PISTOL = 1,
    ARMA_SHOTGUN = 2,
    ARMA_SMG = 3
} TipoArma;

typedef enum {
    ITEM_CHAVE = 0,
    ITEM_MAPA = 1,
    ITEM_CURE = 2,
    ITEM_SHOTGUN = 3,
    ITEM_SMG = 4
} TipoItem;

typedef enum {
    BOSS_NENHUM = 0,
    BOSS_PROWLER = 1,
    BOSS_HUNTER = 2,
    BOSS_ABOMINATION = 3
} TipoBoss;

typedef struct {
    TipoArma tipo;
    int dano;
    int taxaTiroMS;
    int penteMax;
    int penteAtual;
    int municaoTotal;
    float tempoRecarga;
    float cooldown;
} Arma;

// Forward declaration para evitar inclusões circulares
typedef struct Zumbi Zumbi;
typedef struct Bala Bala;

typedef struct {
    Vector2 posicao;
    int vida;
    Arma slots[3];
    int slotAtivo;
    int direcaoVertical;
    int direcaoHorizontal;
    Texture2D spriteAtual;
    float tempoTotal;
    int fase;
    float velocidadeBase;
    bool estaRecarregando;
    float tempoRecargaAtual;
    bool tempoJaSalvo;
    float timerBoss;
    bool bossSpawnado;
    bool temChave;
    bool temMapa;
    bool temCure;
    bool jogoVencido;
    EstadoJogo estadoJogo;

    int hordaAtual;
    EstadoHorda estadoHorda;
    int zumbisRestantes;
    int zumbisTotaisHorda;
    int zumbisSpawnados;
    int bossesTotaisHorda;
    int bossesSpawnados;
    float tempoIntervalo;
    float tempoSpawn;
    float tempoSpawnBoss;

    float cooldownDanoBala;
    float cooldownDanoZumbi;
} Player;

// Estruturas de Zumbi e Bala (mantidas aqui por serem usadas em jogo.c)
struct Zumbi {
    Vector2 posicao;
    Vector2 posicaoAnterior;
    Vector2 velocidade;
    int vida;
    float raio;
    float velocidadeBase;
    float tempoDesvio;
    float anguloDesvio;
    int tipoMovimento;
    int direcaoVertical;
    int direcaoHorizontal;
    int tipoSkin;
    Texture2D spriteFrenteDireita;
    Texture2D spriteFrenteEsquerda;
    Texture2D spriteCostasDireita;
    Texture2D spriteCostasEsquerda;
    Texture2D spriteAtual;
    Caminho caminho;
    float tempoTravado;
    Vector2 ultimaPosicaoVerificada;
    struct Zumbi *proximo;
};

struct Bala {
    Vector2 posicao;
    Vector2 velocidade;
    int tipo;
    float dano;
    float raio;
    float tempoVida;
    float angulo;
    struct Bala *proximo;
};


// Funções principais do jogo
void iniciarJogo(Player *jogador);
void atualizarJogoComPathfinding(Player *jogador, Zumbi **zumbis, Bala **balas, const Mapa *mapa, PathfindingGrid *grid);
void desenharJogo(Player *jogador, Zumbi *zumbis, Bala *balas, Texture2D texturaMapa, Recursos *recursos);


// Funções de Arma
void inicializarArma(Arma *arma, TipoArma tipo);
void equiparArma(Player *jogador, int slot);
void recarregarArma(Player *jogador);
void atirarArma(Player *jogador, Bala **balas, Vector2 alvo);


void adicionarBala(struct Bala **cabeca, Vector2 posInicial, Vector2 alvo, int tipo, float dano);
void atualizarBalas(struct Bala **cabeca, Mapa *mapa);

// Funções de Arma
void inicializarArma(Arma *arma, TipoArma tipo);
void equiparArma(Player *jogador, int slot);
void recarregarArma(Player *jogador);
void atirarArma(Player *jogador, Bala **balas, Vector2 alvo);

// Funções de Bala
void adicionarBala(Bala **cabeca, Vector2 posInicial, Vector2 alvo, int tipo, float dano);
void atualizarBalas(Bala **cabeca, Mapa *mapa);

// Funções de Zumbi
void adicionarZumbi(Zumbi **cabeca, Vector2 posInicial, Texture2D sprites[][4]);
void atualizarZumbisComPathfinding(Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime, const Mapa *mapa, PathfindingGrid *grid);
void desenharZumbis(Zumbi *cabeca);
void liberarZumbis(Zumbi **cabeca);

// Funções de Colisão
int verificarColisaoCirculos(Vector2 pos1, float raio1, Vector2 pos2, float raio2);
void verificarColisoesBalaZumbi(Bala **balas, Zumbi **zumbis, Player *jogador);
void verificarColisoesBalaJogador(Bala **balas, Player *jogador);
void verificarColisoesJogadorZumbi(Player *jogador, Zumbi *zumbis);

#endif