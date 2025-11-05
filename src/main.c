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
    adicionarZumbi(&listaZumbis, (Vector2){100, 100});
    adicionarZumbi(&listaZumbis, (Vector2){700, 100});
    adicionarZumbi(&listaZumbis, (Vector2){400, 500});
    adicionarZumbi(&listaZumbis, (Vector2){100, 500});
    adicionarZumbi(&listaZumbis, (Vector2){700, 500});

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
    CloseWindow();

    return 0;
}
