#ifndef ITEM_H
#define ITEM_H

#include "raylib.h"
#include <stdbool.h>

// TipoArma, TipoItem, Player e Recursos são definidos em jogo.h e recursos.h
// Este header DEVE ser incluído após jogo.h

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

// Funções de Item
void criarItem(Item *item, TipoItem tipo, Vector2 posicao);
void desenharItem(Item *item, Recursos *recursos);
bool verificarColetaItem(Item *item, Player *jogador);

// Funções de Porta
void criarPorta(Porta *porta, Vector2 posicao, int faseDestino);
void desenharPorta(Porta *porta, Texture2D texturaPorta);
bool verificarInteracaoPorta(Porta *porta, Player *jogador);

#endif
