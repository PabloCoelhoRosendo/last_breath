// include/jogo.h

#ifndef JOGO_H
#define JOGO_H

#include "raylib.h"

#define TAMANHO_MAPA 20

// 1. Estrutura do Jogador (Henrique/Geral - Requisito: Structs)
typedef struct {
    Vector2 posicao;
    int vida;
    int municao;
    int pontos;
} Player;

// 2. Estrutura para Zumbis (Victor - Requisito: Structs, Lista Encadeada)
typedef struct Zombie {
    Vector2 posicao;
    int saude;
    struct Zombie *proximo; // Ponteiro para o próximo zumbi
} Zombie;

// 3. Estrutura para Balas ( - Pablo - Requisito: Structs, Lista Encadeada)
typedef struct Bala {
    Vector2 posicao;
    Vector2 velocidade;
    struct Bala *proximo; // Ponteiro para a próxima bala (Requisito: Ponteiros)
} Bala;


// Protótipos das funções principais (a serem implementadas em src/jogo.c)
void iniciarJogo(Player *jogador);
void atualizarJogo(Player *jogador, struct Zombie **zumbis, struct Bala **balas);
void desenharJogo(Player *jogador, struct Zombie *zumbis, struct Bala *balas);

// Protótipos do Módulo de Balas ( - Pablo)
void adicionarBala(struct Bala **cabeca, Vector2 posInicial, Vector2 alvo);
void atualizarBalas(struct Bala **cabeca);

#endif