// src/main.c
// Arquivo principal do jogo Last Breath

#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"

int main(void) {
    // Configurações da janela
    const int larguraTela = 800;
    const int alturaTela = 600;
    
    InitWindow(larguraTela, alturaTela, "Last Breath - Zombie Survival Game");
    SetTargetFPS(60);
    
    // Inicializar estruturas do jogo
    Player jogador;
    Zombie *listaZumbis = NULL;  // Lista encadeada de zumbis
    Bala *listaBalas = NULL;      // Lista encadeada de balas
    
    // Inicializar o mapa
    mapa(mapaDoJogo);
    
    // Inicializar o jogador
    iniciarJogo(&jogador);
    
    // Loop principal do jogo
    while (!WindowShouldClose()) {
        // Atualizar a lógica do jogo
        atualizarJogo(&jogador, &listaZumbis, &listaBalas);
        
        // Desenhar tudo na tela
        BeginDrawing();
        ClearBackground(DARKGRAY);
        
        desenharJogo(&jogador, listaZumbis, listaBalas);
        
        EndDrawing();
    }
    
    // Limpar recursos antes de fechar
    CloseWindow();
    
    return 0;
}
