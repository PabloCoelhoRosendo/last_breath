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

// --- Funções Auxiliares de Colisão ---

// Função para verificar colisão entre dois círculos
int verificarColisaoCirculos(Vector2 pos1, float raio1, Vector2 pos2, float raio2) {
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    float distanciaAoQuadrado = dx * dx + dy * dy;
    float raiosCombinados = raio1 + raio2;
    return distanciaAoQuadrado < (raiosCombinados * raiosCombinados);
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
void atualizarJogo(Player *jogador, Zumbi **zumbis, Bala **balas) {
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

    // Atualizar zumbis
    atualizarZumbis(zumbis, jogador->posicao, GetFrameTime());

    // Verificar colisões
    verificarColisoesBalaZumbi(balas, zumbis, jogador);
    verificarColisoesJogadorZumbi(jogador, *zumbis);
    verificarColisoesZumbiZumbi(*zumbis);
}

// Função para desenhar todos os elementos do jogo
void desenharJogo(Player *jogador, Zumbi *zumbis, Bala *balas) {
    // Desenhar os zumbis primeiro (para ficarem atrás)
    desenharZumbis(zumbis);

    // Desenhar o jogador
    DrawCircleV(jogador->posicao, 15, BLUE);

    // Desenhar as balas
    Bala *balaAtual = balas;
    while (balaAtual != NULL) {
        DrawCircleV(balaAtual->posicao, 5, YELLOW);
        balaAtual = balaAtual->proximo;
    }

    // --- HUD MELHORADO ---
    
    // Barra de Vida (com cores baseadas na vida)
    Color corVida = GREEN;
    if (jogador->vida <= 30) {
        corVida = RED;
    } else if (jogador->vida <= 60) {
        corVida = ORANGE;
    }
    
    // Desenhar fundo da barra de vida
    DrawRectangle(10, 10, 204, 24, DARKGRAY);
    // Desenhar barra de vida atual
    DrawRectangle(12, 12, (jogador->vida * 2), 20, corVida);
    // Contorno da barra
    DrawRectangleLines(10, 10, 204, 24, WHITE);
    // Texto da vida
    DrawText(TextFormat("Vida: %d/100", jogador->vida), 15, 13, 20, WHITE);
    
    // Munição (com aviso de pouca munição)
    Color corMunicao = WHITE;
    if (jogador->municao <= 5) {
        corMunicao = RED;
        // Piscar quando está sem munição
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawText("MUNICAO BAIXA!", 10, 85, 20, RED);
        }
    }
    DrawText(TextFormat("Municao: %d", jogador->municao), 10, 40, 20, corMunicao);
    
    // Pontos
    DrawText(TextFormat("Pontos: %d", jogador->pontos), 10, 65, 20, GOLD);
    
    // Instruções
    DrawText("WASD - Mover | Mouse - Mirar | Click - Atirar", 200, 570, 15, LIGHTGRAY);
    
    // Aviso de Game Over
    if (jogador->vida <= 0) {
        DrawRectangle(0, 0, 800, 600, (Color){0, 0, 0, 150});
        DrawText("GAME OVER", 250, 250, 60, RED);
        DrawText(TextFormat("Pontuacao Final: %d", jogador->pontos), 280, 320, 30, WHITE);
        DrawText("Pressione ESC para sair", 260, 360, 20, LIGHTGRAY);
    }
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
    
    // 2. Inicializar o novo zumbi com comportamento aleatório
    novoZumbi->posicao = posInicial;
    novoZumbi->velocidade = (Vector2){0, 0};
    novoZumbi->vida = 100;
    novoZumbi->raio = 20.0f;
    
    // Atribuir tipo de movimento aleatório (0-3)
    novoZumbi->tipoMovimento = GetRandomValue(0, 3);
    
    // Velocidade base varia entre zumbis
    novoZumbi->velocidadeBase = 30.0f + GetRandomValue(0, 40); // 30-70 pixels/s
    
    // Inicializar timers e ângulos aleatórios
    novoZumbi->tempoDesvio = 0.0f;
    novoZumbi->anguloDesvio = (float)GetRandomValue(0, 360) * DEG2RAD;
    
    // 3. Inserir no início da lista
    novoZumbi->proximo = *cabeca;
    *cabeca = novoZumbi;
}
// Função para atualizar todos os zumbis com diferentes comportamentos
void atualizarZumbis(Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime) {
    Zumbi *atual = *cabeca;
    Zumbi *anterior = NULL;
    
    while (atual != NULL) {
        // Calcular direção base para o jogador
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
        
        // Aplicar comportamento baseado no tipo
        Vector2 direcaoFinal = direcao;
        float velocidadeFinal = atual->velocidadeBase;
        
        switch (atual->tipoMovimento) {
            case 0: // DIRETO - vai reto pro jogador
                // Usa direção e velocidade padrão
                break;
                
            case 1: // ZIGZAG - movimento em onda
                atual->tempoDesvio += deltaTime * 3.0f;
                {
                    float desvio = sinf(atual->tempoDesvio) * 0.5f;
                    // Adiciona movimento perpendicular
                    direcaoFinal.x += -direcao.y * desvio;
                    direcaoFinal.y += direcao.x * desvio;
                    
                    // Renormalizar
                    float mag = sqrtf(direcaoFinal.x * direcaoFinal.x + 
                                     direcaoFinal.y * direcaoFinal.y);
                    if (mag > 0) {
                        direcaoFinal.x /= mag;
                        direcaoFinal.y /= mag;
                    }
                }
                break;
                
            case 2: // CIRCULAR - faz círculos enquanto se aproxima
                atual->tempoDesvio += deltaTime * 2.0f;
                atual->anguloDesvio += deltaTime * 2.0f;
                {
                    float cosA = cosf(atual->anguloDesvio);
                    float sinA = sinf(atual->anguloDesvio);
                    // Mistura movimento circular com aproximação
                    direcaoFinal.x = direcao.x * 0.7f + (-direcao.y * cosA) * 0.3f;
                    direcaoFinal.y = direcao.y * 0.7f + (direcao.x * sinA) * 0.3f;
                    
                    // Renormalizar
                    float mag = sqrtf(direcaoFinal.x * direcaoFinal.x + 
                                     direcaoFinal.y * direcaoFinal.y);
                    if (mag > 0) {
                        direcaoFinal.x /= mag;
                        direcaoFinal.y /= mag;
                    }
                }
                break;
                
            case 3: // IMPREVISÍVEL - muda de direção aleatoriamente
                atual->tempoDesvio += deltaTime;
                if (atual->tempoDesvio > 1.0f) { // Muda direção a cada 1 segundo
                    atual->tempoDesvio = 0.0f;
                    atual->anguloDesvio = (float)GetRandomValue(-45, 45) * DEG2RAD;
                }
                {
                    float cosA = cosf(atual->anguloDesvio);
                    float sinA = sinf(atual->anguloDesvio);
                    // Aplica rotação à direção
                    float tempX = direcao.x * cosA - direcao.y * sinA;
                    float tempY = direcao.x * sinA + direcao.y * cosA;
                    direcaoFinal.x = tempX;
                    direcaoFinal.y = tempY;
                }
                break;
        }
        
        // Calcular velocidade final
        atual->velocidade.x = direcaoFinal.x * velocidadeFinal;
        atual->velocidade.y = direcaoFinal.y * velocidadeFinal;
        
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

// --- Funções de Verificação de Colisão ---

// Função para verificar colisões entre balas e zumbis
void verificarColisoesBalaZumbi(Bala **balas, Zumbi **zumbis, Player *jogador) {
    Bala *balaAtual = *balas;
    Bala *balaAnterior = NULL;

    // Iterar sobre todas as balas
    while (balaAtual != NULL) {
        Zumbi *zumbiAtual = *zumbis;
        Zumbi *zumbiAnterior = NULL;
        int balaRemovida = 0;

        // Iterar sobre todos os zumbis
        while (zumbiAtual != NULL && !balaRemovida) {
            // Verificar colisão (raio da bala = 5, raio do zumbi = 20)
            if (verificarColisaoCirculos(balaAtual->posicao, 5.0f, zumbiAtual->posicao, zumbiAtual->raio)) {
                // Aplicar dano ao zumbi
                zumbiAtual->vida -= 25;

                // Se o zumbi morreu
                if (zumbiAtual->vida <= 0) {
                    jogador->pontos += 10; // Adicionar pontos

                    // Remover zumbi da lista
                    Zumbi *zumbiRemover = zumbiAtual;
                    if (zumbiAnterior == NULL) {
                        *zumbis = zumbiAtual->proximo;
                    } else {
                        zumbiAnterior->proximo = zumbiAtual->proximo;
                    }
                    zumbiAtual = zumbiAtual->proximo;
                    free(zumbiRemover);
                } else {
                    zumbiAtual = zumbiAtual->proximo;
                }

                // Remover a bala da lista
                Bala *balaRemover = balaAtual;
                if (balaAnterior == NULL) {
                    *balas = balaAtual->proximo;
                } else {
                    balaAnterior->proximo = balaAtual->proximo;
                }
                balaAtual = balaAtual->proximo;
                free(balaRemover);
                balaRemovida = 1;
            } else {
                zumbiAnterior = zumbiAtual;
                zumbiAtual = zumbiAtual->proximo;
            }
        }

        // Se a bala não foi removida, avançar para a próxima
        if (!balaRemovida) {
            balaAnterior = balaAtual;
            balaAtual = balaAtual->proximo;
        }
    }
}

// Função para verificar colisões entre jogador e zumbis
void verificarColisoesJogadorZumbi(Player *jogador, Zumbi *zumbis) {
    static float cooldownDano = 0.0f;
    static float flashDano = 0.0f; // Para piscar a tela em vermelho
    
    // Atualizar timers
    cooldownDano -= GetFrameTime();
    if (flashDano > 0.0f) {
        flashDano -= GetFrameTime();
    }
    
    Zumbi *zumbiAtual = zumbis;

    while (zumbiAtual != NULL) {
        // Verificar colisão (raio do jogador = 15, raio do zumbi = 20)
        if (verificarColisaoCirculos(jogador->posicao, 15.0f, zumbiAtual->posicao, zumbiAtual->raio)) {
            
            // Aplicar dano apenas se o cooldown acabou (a cada 0.5 segundos)
            if (cooldownDano <= 0.0f) {
                int dano = 10; // Dano fixo de 10 HP
                jogador->vida -= dano;
                cooldownDano = 0.5f; // Esperar 0.5 segundos para próximo dano
                flashDano = 0.15f;   // Ativar flash vermelho por 0.15 segundos
                
                printf("OUCH! Jogador recebeu %d de dano. Vida: %d\n", dano, jogador->vida);
                
                // KNOCKBACK: Empurrar jogador para trás
                Vector2 direcaoEmpurrao = {
                    jogador->posicao.x - zumbiAtual->posicao.x,
                    jogador->posicao.y - zumbiAtual->posicao.y
                };
                
                // Normalizar direção
                float magnitude = sqrtf(direcaoEmpurrao.x * direcaoEmpurrao.x + 
                                       direcaoEmpurrao.y * direcaoEmpurrao.y);
                
                if (magnitude > 0) {
                    direcaoEmpurrao.x /= magnitude;
                    direcaoEmpurrao.y /= magnitude;
                    
                    // Aplicar força de empurrão
                    float forcaEmpurrao = 40.0f;
                    jogador->posicao.x += direcaoEmpurrao.x * forcaEmpurrao;
                    jogador->posicao.y += direcaoEmpurrao.y * forcaEmpurrao;
                    
                    // Garantir que o jogador não saia da tela após o knockback
                    if (jogador->posicao.x < 20) jogador->posicao.x = 20;
                    if (jogador->posicao.x > 780) jogador->posicao.x = 780;
                    if (jogador->posicao.y < 20) jogador->posicao.y = 20;
                    if (jogador->posicao.y > 580) jogador->posicao.y = 580;
                }
            }

            // Garantir que a vida não fique negativa
            if (jogador->vida < 0) {
                jogador->vida = 0;
            }
        }

        zumbiAtual = zumbiAtual->proximo;
    }
    
    // Desenhar flash vermelho se tomou dano recente
    if (flashDano > 0.0f) {
        // Intensidade do flash diminui com o tempo
        int alpha = (int)(flashDano * 200.0f); // 0-200 de transparência
        DrawRectangle(0, 0, 800, 600, (Color){255, 0, 0, alpha});
    }
}

// Função para verificar e resolver colisões entre zumbis
void verificarColisoesZumbiZumbi(Zumbi *zumbis) {
    Zumbi *zumbi1 = zumbis;

    while (zumbi1 != NULL) {
        Zumbi *zumbi2 = zumbi1->proximo;

        while (zumbi2 != NULL) {
            // Verificar colisão entre dois zumbis
            if (verificarColisaoCirculos(zumbi1->posicao, zumbi1->raio, zumbi2->posicao, zumbi2->raio)) {
                // Calcular vetor de separação
                float dx = zumbi2->posicao.x - zumbi1->posicao.x;
                float dy = zumbi2->posicao.y - zumbi1->posicao.y;
                float distancia = sqrtf(dx * dx + dy * dy);

                // Evitar divisão por zero
                if (distancia > 0) {
                    // Normalizar o vetor de direção
                    float nx = dx / distancia;
                    float ny = dy / distancia;

                    // Calcular quanto os zumbis estão sobrepostos
                    float sobreposicao = (zumbi1->raio + zumbi2->raio) - distancia;

                    // Separar os zumbis pela metade da sobreposição cada um
                    float separacao = sobreposicao / 2.0f;

                    zumbi1->posicao.x -= nx * separacao;
                    zumbi1->posicao.y -= ny * separacao;
                    zumbi2->posicao.x += nx * separacao;
                    zumbi2->posicao.y += ny * separacao;
                }
            }

            zumbi2 = zumbi2->proximo;
        }

        zumbi1 = zumbi1->proximo;
    }
}