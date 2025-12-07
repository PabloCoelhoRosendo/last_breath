#ifndef LOJA_H
#define LOJA_H

#include "raylib.h"
#include "jogo.h"

typedef enum {
    ITEM_LOJA_MUNICAO_PISTOL = 0,
    ITEM_LOJA_MUNICAO_SHOTGUN,
    ITEM_LOJA_MUNICAO_SMG,
    ITEM_LOJA_SHOTGUN,
    ITEM_LOJA_SMG,
    ITEM_LOJA_VIDA_PEQUENA,
    ITEM_LOJA_VIDA_GRANDE,
    ITEM_LOJA_UPGRADE_VIDA_MAX,
    ITEM_LOJA_TOTAL
} TipoItemLoja;

typedef struct {
    TipoItemLoja tipo;
    const char* nome;
    const char* descricao;
    int preco;
    bool disponivel;
} ItemLoja;

typedef struct {
    Vector2 posicao;
    float raio;
    bool ativo;
    bool menuAberto;
    int itemSelecionado;
    ItemLoja itens[ITEM_LOJA_TOTAL];
    int estadoMercador; // 0=normal, 1=faiscando, 2=muito desgastado, 3=robô revelado
} Loja;

// Funções da loja
void inicializarLoja(Loja *loja, Player *jogador);
void atualizarLoja(Loja *loja, Player *jogador);
void desenharLoja(Loja *loja, Player *jogador);
void comprarItem(Loja *loja, Player *jogador, TipoItemLoja item);
void atualizarDisponibilidadeItens(Loja *loja, Player *jogador);

#endif
