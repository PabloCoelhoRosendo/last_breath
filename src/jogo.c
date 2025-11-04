// src/jogo.c

#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"

#include <stdlib.h> // Para malloc e free (Requisito: Alocação Dinâmica de Memória)
#include <stdio.h>
#include <math.h>   // Para sqrtf

// Definição da matriz global do mapa
int mapaDoJogo[TAMANHO_MAPA][TAMANHO_MAPA];

//  Matriz do Mapa (- Leo)
    
void mapa(int mapa[TAMANHO_MAPA][TAMANHO_MAPA]){
    for (int i = 0; i < TAMANHO_MAPA; i++){
        for(int j = 0; j <  TAMANHO_MAPA; j++){
            mapa[i][j] = 0;
        }
    }
}

// --- Funções do Módulo de Balas (- Pablo) ---

// Função para Alocação Dinâmica e inserção de uma nova Bala na Lista Encadeada
void adicionarBala(Bala **cabeca, Vector2 posInicial, Vector2 alvo) {
    
    // 1. Alocação Dinâmica de Memória (Requisito: Alocação Dinâmica)
    Bala *novaBala = (Bala *)malloc(sizeof(Bala));

    if (novaBala == NULL) {
        printf("ERRO: Falha na alocacao de memoria para nova Bala!\n");
        return;
    }

    // 2. Inicializar a nova bala
    novaBala->posicao = posInicial;

    // Calcular a direção da bala (do jogador para o alvo)
    Vector2 direcao = {alvo.x - posInicial.x, alvo.y - posInicial.y};
    
    // Normalizar a direção e aplicar velocidade
    float comprimento = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
    if (comprimento > 0) {
        novaBala->velocidade.x = (direcao.x / comprimento) * 500.0f;
        novaBala->velocidade.y = (direcao.y / comprimento) * 500.0f;
    } else {
        novaBala->velocidade.x = 0;
        novaBala->velocidade.y = 0;
    }

    // 3. Inserir a bala no início da lista (Requisito: Ponteiros e Lista Encadeada)
    novaBala->proximo = *cabeca;
    *cabeca = novaBala;
}

// Função para atualizar todas as balas e remover as que saíram da tela
void atualizarBalas(Bala **cabeca) {
    Bala *atual = *cabeca;
    Bala *anterior = NULL;
    
    while (atual != NULL) {
        // Atualizar posição da bala
        atual->posicao.x += atual->velocidade.x * GetFrameTime();
        atual->posicao.y += atual->velocidade.y * GetFrameTime();
        
        // Verificar se a bala saiu da tela
        if (atual->posicao.x < 0 || atual->posicao.x > 800 ||
            atual->posicao.y < 0 || atual->posicao.y > 600) {
            
            // Remover a bala da lista
            Bala *temp = atual;
            
            if (anterior == NULL) {
                *cabeca = atual->proximo;
                atual = *cabeca;
            } else {
                anterior->proximo = atual->proximo;
                atual = atual->proximo;
            }
            
            free(temp); // Liberar memória
        } else {
            anterior = atual;
            atual = atual->proximo;
        }
    }
}

// Função para inicializar o jogador
void iniciarJogo(Player *jogador) {
    jogador->posicao.x = 400;
    jogador->posicao.y = 300;
    jogador->vida = 100;
    jogador->municao = 30;
    jogador->pontos = 0;
}

// Função para atualizar a lógica do jogo
void atualizarJogo(Player *jogador, Zombie **zumbis, Bala **balas) {
    // Movimento do jogador com WASD
    if (IsKeyDown(KEY_W)) jogador->posicao.y -= 200 * GetFrameTime();
    if (IsKeyDown(KEY_S)) jogador->posicao.y += 200 * GetFrameTime();
    if (IsKeyDown(KEY_A)) jogador->posicao.x -= 200 * GetFrameTime();
    if (IsKeyDown(KEY_D)) jogador->posicao.x += 200 * GetFrameTime();
    
    // Limitar o jogador dentro da tela
    if (jogador->posicao.x < 20) jogador->posicao.x = 20;
    if (jogador->posicao.x > 780) jogador->posicao.x = 780;
    if (jogador->posicao.y < 20) jogador->posicao.y = 20;
    if (jogador->posicao.y > 580) jogador->posicao.y = 580;
    
    // Atirar com o botão esquerdo do mouse
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && jogador->municao > 0) {
        Vector2 mousePos = GetMousePosition();
        adicionarBala(balas, jogador->posicao, mousePos);
        jogador->municao--;
    }
    
    // Atualizar balas
    atualizarBalas(balas);
}

// Função para desenhar todos os elementos do jogo
void desenharJogo(Player *jogador, Zombie *zumbis, Bala *balas) {
    // Desenhar o jogador
    DrawCircleV(jogador->posicao, 15, BLUE);
    
    // Desenhar as balas
    Bala *balaAtual = balas;
    while (balaAtual != NULL) {
        DrawCircleV(balaAtual->posicao, 5, YELLOW);
        balaAtual = balaAtual->proximo;
    }
    
    // Desenhar HUD (Interface)
    DrawText(TextFormat("Vida: %d", jogador->vida), 10, 10, 20, GREEN);
    DrawText(TextFormat("Municao: %d", jogador->municao), 10, 35, 20, WHITE);
    DrawText(TextFormat("Pontos: %d", jogador->pontos), 10, 60, 20, GOLD);
    
    // Instruções
    DrawText("WASD - Mover | Mouse - Mirar | Click - Atirar", 200, 570, 15, LIGHTGRAY);
}
// --- Funções do Módulo de Zumbis ---

// Função para adicionar um novo Zumbi na Lista Encadeada
void adicionarZumbi(Zumbi **cabeca, Vector2 posInicial) {
    
    // 1. Alocação Dinâmica de Memória
    Zumbi *novoZumbi = (Zumbi *)malloc(sizeof(Zumbi));
    
    if (novoZumbi == NULL) {
        printf("ERRO: Falha na alocacao de memoria para novo Zumbi!\n");
        return;
    }
    
    // 2. Inicializar o novo zumbi
    novoZumbi->posicao = posInicial;
    novoZumbi->velocidade = (Vector2){0, 0}; // Será calculada depois
    novoZumbi->vida = 100;
    novoZumbi->raio = 20.0f;
    
    // 3. Inserir no início da lista
    novoZumbi->proximo = *cabeca;
    *cabeca = novoZumbi;
}
// Função para atualizar todos os zumbis (movimento em direção ao jogador)
void atualizarZumbis(Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime) {
    Zumbi *atual = *cabeca;
    Zumbi *anterior = NULL;
    
    while (atual != NULL) {
        // Calcular direção para o jogador
        Vector2 direcao = {
            posicaoJogador.x - atual->posicao.x,
            posicaoJogador.y - atual->posicao.y
        };
        
        // Normalizar direção
        float magnitude = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
        if (magnitude > 0) {
            direcao.x /= magnitude;
            direcao.y /= magnitude;
        }
        
        // Velocidade do zumbi
        float velocidadeZumbi = 50.0f; // pixels por segundo
        atual->velocidade.x = direcao.x * velocidadeZumbi;
        atual->velocidade.y = direcao.y * velocidadeZumbi;
        
        // Atualizar posição
        atual->posicao.x += atual->velocidade.x * deltaTime;
        atual->posicao.y += atual->velocidade.y * deltaTime;
        // Verificar se o zumbi morreu
        if (atual->vida <= 0) {
            // Remover da lista
            Zumbi *removido = atual;
            if (anterior == NULL) {
                *cabeca = atual->proximo;
            } else {
                anterior->proximo = atual->proximo;
            }
            atual = atual->proximo;
            free(removido); // Liberar memória
        } else {
            anterior = atual;
            atual = atual->proximo;
        }
    }
}
// Função para desenhar todos os zumbis
void desenharZumbis(Zumbi *cabeca) {
    Zumbi *atual = cabeca;
    
    while (atual != NULL) {
        // Desenhar zumbi (círculo verde)
        DrawCircleV(atual->posicao, atual->raio, GREEN);
        
        // Desenhar barra de vida
        float barraLargura = 40.0f;
        float porcentagemVida = atual->vida / 100.0f;
        DrawRectangle(
            atual->posicao.x - barraLargura/2,
            atual->posicao.y - atual->raio - 10,
            barraLargura * porcentagemVida,
            5,
            RED
        );
        
        atual = atual->proximo;
    }
}
// Função para liberar toda a lista de zumbis (chamar ao finalizar o jogo)
void liberarZumbis(Zumbi **cabeca) {
    Zumbi *atual = *cabeca;
    
    while (atual != NULL) {
        Zumbi *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    
    *cabeca = NULL;
}