// src/main.c
// Arquivo principal do jogo Last Breath

#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"

int main(void) {
    // Configurações da janela
    const int larguraTela = 800;
    const int alturaTela = 600;

    InitWindow(larguraTela, alturaTela, "Last Breath - Zumbi Survival Game");
    SetTargetFPS(60);

    // Carregar a textura do mapa
    Texture2D texturaMapa = LoadTexture("Novo Projeto.png");

    // Verificar se a textura foi carregada corretamente
    if (texturaMapa.id == 0) {
        // Se não carregou, o jogo continuará funcionando com os retângulos
        printf("Aviso: Nao foi possivel carregar 'Novo Projeto.png'. Usando mapa padrao.\n");
    }

    // Carregar os sprites do jogador
    Texture2D spriteFrenteDireita = LoadTexture("avatar/direita frente.png");
    Texture2D spriteFrenteEsquerda = LoadTexture("avatar/esquerda frente.png");
    Texture2D spriteCostasDireita = LoadTexture("avatar/costas direita.png");
    Texture2D spriteCostasEsquerda = LoadTexture("avatar/costas esquerda.png");

    // Verificar se os sprites foram carregados
    if (spriteFrenteDireita.id == 0 || spriteFrenteEsquerda.id == 0 ||
        spriteCostasDireita.id == 0 || spriteCostasEsquerda.id == 0) {
        printf("Aviso: Alguns sprites do jogador nao foram carregados. Usando circulo.\n");
    }

    // Carregar sprites dos zumbis (5 tipos × 4 direções = 20 sprites)
    // Array: [tipo][direção] onde direção: 0=frenteDireita, 1=frenteEsquerda, 2=costasDireita, 3=costasEsquerda
    Texture2D spritesZumbis[5][4];

    spritesZumbis[0][0] = LoadTexture("zumbis/zumbi 1/zumbi1 frente direita.png");
    spritesZumbis[0][1] = LoadTexture("zumbis/zumbi 1/zumbi1 frente esquerda.png");
    spritesZumbis[0][2] = LoadTexture("zumbis/zumbi 1/zumbi1 costas direita.png");
    spritesZumbis[0][3] = LoadTexture("zumbis/zumbi 1/zumbi1 costas esquerda.png");

    spritesZumbis[1][0] = LoadTexture("zumbis/zumbi 2/zumbi2 frente direita.png");
    spritesZumbis[1][1] = LoadTexture("zumbis/zumbi 2/zumbi2 frente esquerda.png");
    spritesZumbis[1][2] = LoadTexture("zumbis/zumbi 2/zumbi2 costas direita.png");
    spritesZumbis[1][3] = LoadTexture("zumbis/zumbi 2/zumbi2 costas esquerda.png");

    spritesZumbis[2][0] = LoadTexture("zumbis/zumbi 3/zumbi3 frente direita.png");
    spritesZumbis[2][1] = LoadTexture("zumbis/zumbi 3/zumbi3 frente esquerda.png");
    spritesZumbis[2][2] = LoadTexture("zumbis/zumbi 3/zumbi3 costas direita.png");
    spritesZumbis[2][3] = LoadTexture("zumbis/zumbi 3/zumbi3 costas esquerda.png");

    spritesZumbis[3][0] = LoadTexture("zumbis/zumbi 4/zumbi4 frente direita.png");
    spritesZumbis[3][1] = LoadTexture("zumbis/zumbi 4/zumbi4 frente esquerda.png");
    spritesZumbis[3][2] = LoadTexture("zumbis/zumbi 4/zumbi4 costas direita.png");
    spritesZumbis[3][3] = LoadTexture("zumbis/zumbi 4/zumbi4 costas esquerda.png");

    spritesZumbis[4][0] = LoadTexture("zumbis/zumbi 5/zumbi5 frente direita.png");
    spritesZumbis[4][1] = LoadTexture("zumbis/zumbi 5/zumbi5 frente esquerda.png");
    spritesZumbis[4][2] = LoadTexture("zumbis/zumbi 5/zumbi5 costas direita.png");
    spritesZumbis[4][3] = LoadTexture("zumbis/zumbi 5/zumbi5 costas esquerda.png");

    // Inicializar estruturas do jogo
    Player jogador;
    Zumbi *listaZumbis = NULL;  // Lista encadeada de zumbis
    Bala *listaBalas = NULL;      // Lista encadeada de balas

    // Inicializar o mapa
    mapa(mapaDoJogo);

    // Inicializar o jogador
    iniciarJogo(&jogador);

    // Inicializar o sprite do jogador (começa olhando para frente/direita)
    jogador.spriteAtual = spriteFrenteDireita;

    // Adicionar zumbis iniciais
    adicionarZumbi(&listaZumbis, (Vector2){100, 100}, spritesZumbis);
    adicionarZumbi(&listaZumbis, (Vector2){700, 100}, spritesZumbis);
    adicionarZumbi(&listaZumbis, (Vector2){400, 500}, spritesZumbis);
    adicionarZumbi(&listaZumbis, (Vector2){100, 500}, spritesZumbis);
    adicionarZumbi(&listaZumbis, (Vector2){700, 500}, spritesZumbis);

    // Loop principal do jogo
    while (!WindowShouldClose()) {
        // Atualizar a lógica do jogo
        atualizarJogo(&jogador, &listaZumbis, &listaBalas);

        // Atualizar sprite do jogador baseado na direção
        if (jogador.direcaoVertical == 0) { // Frente
            if (jogador.direcaoHorizontal == 0) {
                jogador.spriteAtual = spriteFrenteEsquerda;
            } else {
                jogador.spriteAtual = spriteFrenteDireita;
            }
        } else { // Costas
            if (jogador.direcaoHorizontal == 0) {
                jogador.spriteAtual = spriteCostasEsquerda;
            } else {
                jogador.spriteAtual = spriteCostasDireita;
            }
        }

        // Desenhar tudo na tela
        BeginDrawing();
        ClearBackground(DARKGRAY);

        desenharJogo(&jogador, listaZumbis, listaBalas, texturaMapa);

        EndDrawing();
    }

    // Limpar recursos antes de fechar
    UnloadTexture(texturaMapa);  // Descarregar a textura do mapa
    UnloadTexture(spriteFrenteDireita);
    UnloadTexture(spriteFrenteEsquerda);
    UnloadTexture(spriteCostasDireita);
    UnloadTexture(spriteCostasEsquerda);

    // Descarregar sprites dos zumbis
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            UnloadTexture(spritesZumbis[i][j]);
        }
    }

    CloseWindow();

    return 0;
}
