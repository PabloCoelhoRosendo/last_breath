#ifndef BOSS_H
#define BOSS_H

#include "raylib.h"
#include "pathfinding.h"
#include "item.h"  // Para ter acesso ao tipo Item
#include <stdbool.h>

// TipoBoss, Player, Bala são definidos em jogo.h
// Este header DEVE ser incluído após jogo.h

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
    float anguloRotacao;
    int direcaoVertical;
    int direcaoHorizontal;
    Texture2D spriteFrente;
    Texture2D spriteCostas;
    Texture2D spriteDireita;
    Texture2D spriteEsquerda;
    Texture2D spriteAtual;
    Caminho caminho;
    float tempoDano; // Timer para efeito visual de dano
    struct Boss *proximo;
} Boss;

// Funções de Boss
void criarBoss(Boss **bosses, TipoBoss tipo, Vector2 posicao, 
               Texture2D spriteFrente, Texture2D spriteCostas, 
               Texture2D spriteDireita, Texture2D spriteEsquerda);

void atualizarBossComPathfinding(Boss **bosses, Player *jogador, Bala **balas, 
                                  float deltaTime, const Mapa *mapa, PathfindingGrid *grid);

void desenharBoss(Boss *bosses);

void verificarColisoesBossBala(Boss **bosses, struct Bala **balas, 
                               Item *itemProgresso, Item *itemArma, Player *jogador, Moeda **moedas);

void verificarColisoesBossJogador(Boss *bosses, Player *jogador);

int contarBossesVivos(Boss *bosses);

void liberarBosses(Boss **bosses);

#endif
