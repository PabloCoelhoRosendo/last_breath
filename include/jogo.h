#ifndef JOGO_H
#define JOGO_H

#include "raylib.h"
#include "mapa.h"
#include "recursos.h"
#include "pathfinding.h"

// Forward declarations
typedef struct Moeda Moeda;

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
    
    int moedas; // Sistema de economia
    bool modoDeus; // Modo Deus (F1)
    
    // Estatísticas de jogo
    int zumbisMatados;
    int bossesMatados;
    int moedasColetadas;
    
    // Flags de progresso narrativo
    bool leuRelatorio;
    bool conheceuMenina;
    bool meninaLiberada;
    bool estaNoBanheiro;  // Flag para saber se está no mapa do banheiro
    bool finalFeliz;
    bool fase2Concluida; // Marca se a fase 2 (mercado) já foi completada
    bool fase3Concluida; // Marca se a fase 3 (rua) já foi completada
    bool matouBossFinal; // Marca se matou o boss final no laboratório
    bool temChaveMisteriosa; // Marca se coletou a chave dropada pelo boss final
    bool spawnadoRetornoFase2; // Marca se já spawnou zumbis ao voltar da fase 2
    bool spawnadoRetornoFase3; // Marca se já spawnou zumbis ao voltar da fase 3
    bool spawnadoRetornoFase4; // Marca se já spawnou zumbis ao voltar da fase 4
    int zumbisSpawnadosRetorno; // Contador de zumbis spawnados gradativamente
    float tempoSpawnRetorno; // Timer para spawn gradativo
} Player;

// Estruturas de Zumbi e Bala (mantidas aqui por serem usadas em jogo.c)
struct Zumbi {
    Vector2 posicao;
    Vector2 posicaoAnterior;
    Vector2 velocidade;
    int vida;
    int vidaMax;
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
    float tempoDano; // Timer para efeito visual de dano
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

typedef struct {
    Vector2 posicao;
    float largura;
    float altura;
    bool ativa;
    bool lida;
    bool trancada;
} Escrivaninha;

typedef struct Cartucho {
    Vector2 posicao;
    float angulo;
    float tempoVida;
    float velocidadeX;
    float velocidadeY;
    int tipo; // Tipo da arma que ejetou
    struct Cartucho *proximo;
} Cartucho;

typedef struct ManchaSangue {
    Vector2 posicao;
    float raio;
    float alpha;
    float tempoVida;
    float pontos[8][2]; // 8 pontos irregulares para formato orgânico (x, y offset)
    struct ManchaSangue *proximo;
} ManchaSangue;

// Funções principais do jogo
void iniciarJogo(Player *jogador);
void atualizarJogoComPathfinding(Player *jogador, Zumbi **zumbis, Bala **balas, const Mapa *mapa, PathfindingGrid *grid, Cartucho **cartuchos, ManchaSangue **manchas, Recursos *recursos);
void desenharJogo(Player *jogador, Zumbi *zumbis, Bala *balas, Texture2D texturaMapa, Recursos *recursos);

// Funções de Cartuchos e Manchas
void adicionarCartucho(Cartucho **cabeca, Vector2 posicao, float angulo, int tipo);
void atualizarCartuchos(Cartucho **cabeca, float deltaTime);
void desenharCartuchos(Cartucho *cabeca);
void liberarCartuchos(Cartucho **cabeca);

void adicionarManchaSangue(ManchaSangue **cabeca, Vector2 posicao, float raio);
void atualizarManchasSangue(ManchaSangue **cabeca, float deltaTime);
void desenharManchasSangue(ManchaSangue *cabeca);
void liberarManchasSangue(ManchaSangue **cabeca);

// Funções de Arma
void inicializarArma(Arma *arma, TipoArma tipo);
void equiparArma(Player *jogador, int slot);
void recarregarArma(Player *jogador, Recursos *recursos);
void atirarArma(Player *jogador, Bala **balas, Vector2 alvo, Cartucho **cartuchos, Recursos *recursos);

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
void verificarColisoesBalaZumbi(Bala **balas, Zumbi **zumbis, Player *jogador, ManchaSangue **manchas, Moeda **moedas, Recursos *recursos);
void verificarColisoesBalaJogador(Bala **balas, Player *jogador);
void verificarColisoesJogadorZumbi(Player *jogador, Zumbi *zumbis, Recursos *recursos);

// Funções da Escrivaninha
void criarEscrivaninha(Escrivaninha *esc, Vector2 posicao);
void desenharEscrivaninha(Escrivaninha *esc);
bool verificarInteracaoEscrivaninha(Escrivaninha *esc, Player *jogador);

#endif