#ifndef MENINA_H
#define MENINA_H

#include "raylib.h"
#include "jogo.h"
#include "recursos.h"
#include "mapa.h"

typedef struct {
    Vector2 posicao;
    bool ativa;
    bool seguindo;
    float raio;
    float cooldownTiro;
    float alcanceVisao;
    int danoTiro;
    float timerSom;

    // Sprites
    Texture2D spriteDeitada;
    Texture2D spriteFrenteDireita;
    Texture2D spriteFrenteEsquerda;
    Texture2D spriteCostas;
    Texture2D spriteAtirandoDireita;
    Texture2D spriteAtirandoEsquerda;
    Texture2D spriteAtual;

    // Estado e direção
    int direcaoHorizontal;     // 0 = esquerda, 1 = direita
    int direcaoVertical;       // 0 = frente/baixo, 1 = costas/cima
    bool estaAtirando;         // Flag temporária para animação de tiro
    float timerAnimacaoTiro;   // Timer para duração da animação de tiro
    bool jaTirouPelaVez;       // Flag para marcar primeiro disparo
} Menina;

// Funções públicas
void inicializarMenina(Menina *menina);
void carregarSpritesMenina(Menina *menina, Recursos *recursos);
void atualizarMenina(Menina *menina, Player *jogador, Mapa *mapa, float deltaTime, Zumbi **zumbis, Bala **balas, Recursos *recursos);
void desenharMenina(Menina *menina);

#endif
