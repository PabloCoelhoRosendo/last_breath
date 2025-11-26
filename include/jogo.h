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

typedef struct Zumbi {
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
    struct Zumbi *proximo;
} Zumbi;

typedef struct Bala {
    Vector2 posicao;
    Vector2 velocidade;
    int tipo;
    float dano;
    float raio;
    float tempoVida;
    float angulo;
    struct Bala *proximo;
} Bala;

typedef enum {
    BOSS_NENHUM = 0,
    BOSS_PROWLER = 1,
    BOSS_HUNTER = 2,
    BOSS_ABOMINATION = 3
} TipoBoss;

typedef struct Boss {
    TipoBoss tipo;
    Vector2 posicao;
    Vector2 posicaoAnterior;
    int vida;
    int vidaMax;
    float velocidade;
    float raio;
    bool ativo;
    bool atacando;
    float tempoAtaque;
    float cooldownAtaque;
    int padraoAtaque;
    float anguloRotacao;
    int direcaoVertical;
    int direcaoHorizontal;
    Texture2D spriteFrente;
    Texture2D spriteCostas;
    Texture2D spriteDireita;
    Texture2D spriteEsquerda;
    Texture2D spriteAtual;
    Caminho caminho;
    struct Boss *proximo;
} Boss;

typedef enum {
    ITEM_CHAVE = 0,
    ITEM_MAPA = 1,
    ITEM_CURE = 2,
    ITEM_SHOTGUN = 3,
    ITEM_SMG = 4
} TipoItem;

typedef struct {
    TipoItem tipo;
    Vector2 posicao;
    float raio;
    bool ativo;
    bool coletado;
    TipoArma tipoArma;
} Item;
typedef struct {
    Vector2 posicao;
    float largura;
    float altura;
    bool ativa;           
    bool trancada;        
    int faseDestino;      
} Porta;


void iniciarJogo(Player *jogador);
void atualizarJogo(Player *jogador, struct Zumbi **zumbis, struct Bala **balas);
void atualizarJogoComPathfinding(Player *jogador, struct Zumbi **zumbis, struct Bala **balas, const Mapa *mapa, PathfindingGrid *grid);
void desenharJogo(Player *jogador, struct Zumbi *zumbis, struct Bala *balas, Texture2D texturaMapa, Recursos *recursos);


void inicializarArma(Arma *arma, TipoArma tipo);
void equiparArma(Player *jogador, int slot);
void recarregarArma(Player *jogador);
void atirarArma(Player *jogador, struct Bala **balas, Vector2 alvo);


void criarBoss(struct Boss **bosses, TipoBoss tipo, Vector2 posicao, Texture2D spriteFrente, Texture2D spriteCostas, Texture2D spriteDireita, Texture2D spriteEsquerda);
void atualizarBoss(struct Boss **bosses, Player *jogador, struct Bala **balas, float deltaTime);
void atualizarBossComPathfinding(struct Boss **bosses, Player *jogador, struct Bala **balas, float deltaTime, const Mapa *mapa, PathfindingGrid *grid);
void desenharBoss(struct Boss *bosses);
void verificarColisoesBossBala(struct Boss **bosses, struct Bala **balas, Item *itemProgresso, Item *itemArma, Player *jogador);
void verificarColisoesBossJogador(struct Boss *bosses, Player *jogador);


void criarItem(Item *item, TipoItem tipo, Vector2 posicao);
void desenharItem(Item *item, Recursos *recursos);
bool verificarColetaItem(Item *item, Player *jogador);
void criarPorta(Porta *porta, Vector2 posicao, int faseDestino);
void desenharPorta(Porta *porta, Texture2D texturaPorta);
bool verificarInteracaoPorta(Porta *porta, Player *jogador);


void adicionarBala(struct Bala **cabeca, Vector2 posInicial, Vector2 alvo, int tipo, float dano);
void atualizarBalas(struct Bala **cabeca, Mapa *mapa);


void adicionarZumbi(struct Zumbi **cabeca, Vector2 posInicial, Texture2D sprites[][4]);
void atualizarZumbisComPathfinding(struct Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime, const Mapa *mapa, PathfindingGrid *grid);
void desenharZumbis(struct Zumbi *cabeca);
void liberarZumbis(struct Zumbi **cabeca);


void iniciarHorda(Player *jogador, int numeroHorda);
void atualizarHorda(Player *jogador, struct Zumbi **zumbis, struct Boss **bosses, float deltaTime);
int contarBossesVivos(struct Boss *bosses);


int verificarColisaoCirculos(Vector2 pos1, float raio1, Vector2 pos2, float raio2);
void verificarColisoesBalaZumbi(struct Bala **balas, struct Zumbi **zumbis, Player *jogador);
void verificarColisoesBalaJogador(struct Bala **balas, Player *jogador);
void verificarColisoesJogadorZumbi(Player *jogador, struct Zumbi *zumbis);
void verificarColisoesZumbiZumbi(struct Zumbi *zumbis);

#endif