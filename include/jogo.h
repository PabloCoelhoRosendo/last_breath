// include/jogo.h

#ifndef JOGO_H
#define JOGO_H

#include "raylib.h"
#include "mapa.h"
#include "recursos.h"

// NOTA: As constantes de mapa agora estão definidas em mapa.h:
// - LARGURA_MAPA 32
// - ALTURA_MAPA 24
// - TAMANHO_TILE 32

// Enum para estados do jogo
typedef enum {
    ESTADO_MENU = 0,
    ESTADO_JOGANDO = 1,
    ESTADO_PAUSADO = 2
} EstadoJogo;

// Enum para estados de horda
typedef enum {
    HORDA_NAO_INICIADA = 0,
    HORDA_EM_PROGRESSO = 1,
    HORDA_COMPLETA = 2,
    HORDA_INTERVALO = 3  // Período entre hordas
} EstadoHorda;

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
    float timerBoss;         // Timer de 45s para spawn do boss
    bool bossSpawnado;       // Flag se o boss da fase já foi spawnado
    bool temChave;           // Flag se possui a Chave do Portão
    bool temMapa;            // Flag se possui o Mapa do Laboratório
    bool temCure;            // Flag se coletou a CURE
    bool jogoVencido;        // Flag se coletou a CURE e venceu o jogo
    EstadoJogo estadoJogo;   // Estado atual do jogo (menu/jogando/pausado)
    
    // Sistema de Hordas
    int hordaAtual;          // Número da horda atual (1-3 na fase 1)
    EstadoHorda estadoHorda; // Estado atual da horda
    int zumbisRestantes;     // Zumbis vivos da horda atual
    int zumbisTotaisHorda;   // Total de zumbis que devem spawnar na horda
    int zumbisSpawnados;     // Quantos zumbis já foram spawnados
    int bossesTotaisHorda;   // Total de bosses que devem spawnar na horda
    int bossesSpawnados;     // Quantos bosses já foram spawnados
    float tempoIntervalo;    // Tempo restante do intervalo entre hordas
    float tempoSpawn;        // Timer para controlar spawn gradual de zumbis
    float tempoSpawnBoss;    // Timer para controlar spawn gradual de bosses
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

// Enum para tipos de boss
typedef enum {
    BOSS_NENHUM = 0,
    BOSS_PROWLER = 1,    // Fase 1: Boss lobo com ataque slam
    BOSS_HUNTER = 2,     // Fase 2: Boss ágil de perseguição
    BOSS_ABOMINATION = 3 // Fase 3: Boss estático com projéteis
} TipoBoss;

// 4. Estrutura para Boss (Requisito: Structs)
typedef struct Boss {
    TipoBoss tipo;
    Vector2 posicao;
    int vida;
    int vidaMax;
    float velocidade;
    float raio;
    bool ativo;              // Se o boss está vivo e ativo
    bool atacando;           // Se está executando um ataque
    float tempoAtaque;       // Timer para controlar ataques
    float cooldownAtaque;    // Tempo entre ataques
    int padraoAtaque;        // Padrão de ataque atual (para Abomination)
    float anguloRotacao;     // Ângulo para ataques rotativos (Abomination)
    int direcaoVertical;     // 0 = frente, 1 = costas
    int direcaoHorizontal;   // 0 = esquerda, 1 = direita
    Texture2D spriteFrente;
    Texture2D spriteCostas;
    Texture2D spriteDireita;
    Texture2D spriteEsquerda;
    Texture2D spriteAtual;   // Sprite atual sendo renderizado
    struct Boss *proximo;    // Ponteiro para próximo boss (lista encadeada)
} Boss;

// Enum para tipos de item
typedef enum {
    ITEM_CHAVE = 0,
    ITEM_MAPA = 1,
    ITEM_CURE = 2,
    ITEM_SHOTGUN = 3,
    ITEM_SMG = 4
} TipoItem;

// 5. Estrutura para Itens Coletáveis (Requisito: Structs)
typedef struct {
    TipoItem tipo;
    Vector2 posicao;
    float raio;           // Raio de coleta (distância para coletar)
    bool ativo;           // Se o item está no mapa para ser coletado
    bool coletado;        // Se já foi coletado
    TipoArma tipoArma;    // Tipo de arma (se o item for uma arma)
} Item;

// 6. Estrutura para Porta (Requisito: Structs)
typedef struct {
    Vector2 posicao;
    float largura;
    float altura;
    bool ativa;           // Se a porta está no mapa
    bool trancada;        // Se a porta requer chave
    int faseDestino;      // Qual fase a porta leva (1->2, 2->3)
} Porta;

// Protótipos das funções principais (a serem implementadas em src/jogo.c)
void iniciarJogo(Player *jogador);
void atualizarJogo(Player *jogador, struct Zumbi **zumbis, struct Bala **balas);
void desenharJogo(Player *jogador, struct Zumbi *zumbis, struct Bala *balas, Texture2D texturaMapa);

// Protótipos do Sistema de Armas
void inicializarArma(Arma *arma, TipoArma tipo);
void equiparArma(Player *jogador, int slot);
void recarregarArma(Player *jogador);
void atirarArma(Player *jogador, struct Bala **balas, Vector2 alvo);

// Protótipos do Sistema de Boss
void criarBoss(struct Boss **bosses, TipoBoss tipo, Vector2 posicao, Texture2D spriteFrente, Texture2D spriteCostas, Texture2D spriteDireita, Texture2D spriteEsquerda);
void atualizarBoss(struct Boss **bosses, Player *jogador, struct Bala **balas, float deltaTime);
void desenharBoss(struct Boss *bosses);
void verificarColisoesBossBala(struct Boss **bosses, struct Bala **balas, Item *itemProgresso, Item *itemArma, Player *jogador);
void verificarColisoesBossJogador(struct Boss *bosses, Player *jogador);

// Protótipos do Sistema de Itens e Interação
void criarItem(Item *item, TipoItem tipo, Vector2 posicao);
void desenharItem(Item *item);
bool verificarColetaItem(Item *item, Player *jogador);
void criarPorta(Porta *porta, Vector2 posicao, int faseDestino);
void desenharPorta(Porta *porta);
bool verificarInteracaoPorta(Porta *porta, Player *jogador);

// Protótipos do Módulo de Balas ( - Pablo)
void adicionarBala(struct Bala **cabeca, Vector2 posInicial, Vector2 alvo, int tipo, float dano);
void atualizarBalas(struct Bala **cabeca, Mapa *mapa);

// Protótipos do Módulo de Zumbis ( - Victor)
void adicionarZumbi(struct Zumbi **cabeca, Vector2 posInicial, Texture2D sprites[][4]);
void atualizarZumbis(struct Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime);
void desenharZumbis(struct Zumbi *cabeca);
void liberarZumbis(struct Zumbi **cabeca);

// Protótipos do Sistema de Hordas
void iniciarHorda(Player *jogador, int numeroHorda);
void atualizarHorda(Player *jogador, struct Zumbi **zumbis, struct Boss **bosses, float deltaTime);
int contarBossesVivos(struct Boss *bosses);

// Protótipos das Funções de Colisão
int verificarColisaoCirculos(Vector2 pos1, float raio1, Vector2 pos2, float raio2);
void verificarColisoesBalaZumbi(struct Bala **balas, struct Zumbi **zumbis, Player *jogador);
void verificarColisoesBalaJogador(struct Bala **balas, Player *jogador);
void verificarColisoesJogadorZumbi(Player *jogador, struct Zumbi *zumbis);
void verificarColisoesZumbiZumbi(struct Zumbi *zumbis);

#endif