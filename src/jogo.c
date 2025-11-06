// src/jogo.c

#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"

#include <stdlib.h> // Para malloc e free (Requisito: Alocação Dinâmica de Memória)
#include <stdio.h>
#include <math.h>   // Para sqrtf

// Definição da matriz global do mapa
int mapaDoJogo[TAMANHO_MAPA][TAMANHO_MAPA];

// ===== SISTEMA DE MAPA DA CIDADE =====
// 0 = área livre (branco)
// 1 = prédio (vermelho) - jogador não pode atravessar

void mapa(int mapa[TAMANHO_MAPA][TAMANHO_MAPA]){
    int i, j;

    // Passo 1: Inicializar tudo como área livre (0)
    for (i = 0; i < TAMANHO_MAPA; i++){
        for(j = 0; j < TAMANHO_MAPA; j++){
            mapa[i][j] = 0;
        }
    }

    // Passo 2: Criar os prédios (colocar 1 nas posições desejadas)
    // Nota: A tela tem 800x600 pixels = 20x15 células (cada célula = 40 pixels)

    // Prédio 1: Canto superior esquerdo
    for (i = 1; i < 4; i++) {
        for (j = 1; j < 5; j++) {
            mapa[i][j] = 1;
        }
    }

    // Prédio 2: Canto superior direito
    for (i = 1; i < 4; i++) {
        for (j = 15; j < 19; j++) {
            mapa[i][j] = 1;
        }
    }

    // Prédio 3: Centro do mapa - REMOVIDO

    // Prédio 4: Canto inferior esquerdo
    for (i = 11; i < 14; i++) {
        for (j = 1; j < 6; j++) {
            mapa[i][j] = 1;
        }
    }

    // Prédio 5: Canto inferior direito
    for (i = 11; i < 14; i++) {
        for (j = 14; j < 19; j++) {
            mapa[i][j] = 1;
        }
    }
}

// Função para desenhar o mapa
void desenharMapa(int mapa[TAMANHO_MAPA][TAMANHO_MAPA], Texture2D texturaMapa) {
    // Passo 1: Desenhar os retângulos de colisão (base)
    for (int i = 0; i < TAMANHO_MAPA; i++) {
        for (int j = 0; j < TAMANHO_MAPA; j++) {
            int x = j * TAMANHO_CELULA;
            int y = i * TAMANHO_CELULA;

            if (mapa[i][j] == 1) {
                // Prédios em vermelho
                DrawRectangle(x, y, TAMANHO_CELULA, TAMANHO_CELULA, RED);
                DrawRectangleLines(x, y, TAMANHO_CELULA, TAMANHO_CELULA, DARKGRAY);
            } else {
                // Áreas livres em branco
                DrawRectangle(x, y, TAMANHO_CELULA, TAMANHO_CELULA, WHITE);
                DrawRectangleLines(x, y, TAMANHO_CELULA, TAMANHO_CELULA, LIGHTGRAY);
            }
        }
    }

    // Passo 2: Desenhar a imagem por cima (se foi carregada)
    if (texturaMapa.id > 0) {
        DrawTexture(texturaMapa, 0, 0, WHITE);
    }
}

// Função para verificar colisão com o mapa
// Retorna 1 se colidiu com prédio, 0 se está livre
int verificarColisaoMapa(Vector2 novaPosicao, float raio, int mapa[TAMANHO_MAPA][TAMANHO_MAPA]) {
    // Passo 1: Converter posição do jogador (em pixels) para posição na matriz
    int linha = (int)(novaPosicao.y / TAMANHO_CELULA);
    int coluna = (int)(novaPosicao.x / TAMANHO_CELULA);

    // Passo 2: Verificar se a célula onde o jogador está é um prédio (valor 1)
    if (linha >= 0 && linha < TAMANHO_MAPA && coluna >= 0 && coluna < TAMANHO_MAPA) {
        if (mapa[linha][coluna] == 1) {
            return 1; // Colidiu com prédio
        }
    }

    return 0; // Não colidiu
}

// Função para gerar uma posição válida de spawn (não dentro de prédios)
Vector2 gerarPosicaoValidaSpawn(int mapa[TAMANHO_MAPA][TAMANHO_MAPA], float raio) {
    Vector2 posicao;
    int tentativas = 0;
    int maxTentativas = 100; // Limite de tentativas para evitar loop infinito

    do {
        // Gerar posição aleatória dentro da tela
        posicao.x = (float)GetRandomValue((int)raio, 800 - (int)raio);
        posicao.y = (float)GetRandomValue((int)raio, 600 - (int)raio);

        tentativas++;

        // Se tentar muitas vezes sem sucesso, usar posição padrão segura
        if (tentativas >= maxTentativas) {
            posicao.x = 400;
            posicao.y = 300;
            break;
        }

    } while (verificarColisaoMapa(posicao, raio, mapa)); // Repetir se colidiu com prédio

    return posicao;
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
void adicionarBala(Bala **cabeca, Vector2 posInicial, Vector2 alvo, int tipo, float dano) {
    
    // 1. Alocação Dinâmica de Memória (Requisito: Alocação Dinâmica)
    Bala *novaBala = (Bala *)malloc(sizeof(Bala));

    if (novaBala == NULL) {
        printf("ERRO: Falha na alocacao de memoria para nova Bala!\n");
        return;
    }

    // 2. Inicializar a nova bala
    novaBala->posicao = posInicial;
    novaBala->tipo = tipo;
    novaBala->dano = dano;
    novaBala->raio = (tipo == 0) ? 3.0f : 5.0f;  // Balas do boss são maiores
    novaBala->tempoVida = 0.0f;  // Começa em 0 e incrementa

    // Calcular a direção da bala (do atirador para o alvo)
    Vector2 direcao = {alvo.x - posInicial.x, alvo.y - posInicial.y};
    
    // Normalizar a direção e aplicar velocidade
    float comprimento = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
    if (comprimento > 0) {
        float velocidadeBase = (tipo == 0) ? 500.0f : 300.0f; // Boss tem projéteis mais lentos
        novaBala->velocidade.x = (direcao.x / comprimento) * velocidadeBase;
        novaBala->velocidade.y = (direcao.y / comprimento) * velocidadeBase;
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
    float deltaTime = GetFrameTime();
    
    while (atual != NULL) {
        // Atualizar posição da bala
        atual->posicao.x += atual->velocidade.x * deltaTime;
        atual->posicao.y += atual->velocidade.y * deltaTime;
        
        // Atualizar tempo de vida
        atual->tempoVida += deltaTime;
        
        // Tempo máximo de vida: 2 segundos para balas normais, 3 para boss
        float tempoMaximo = (atual->tipo == 0) ? 2.0f : 3.0f;
        
        // Verificar se a bala saiu da tela ou expirou
        if (atual->posicao.x < 0 || atual->posicao.x > 800 ||
            atual->posicao.y < 0 || atual->posicao.y > 600 ||
            atual->tempoVida >= tempoMaximo) {
            
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
    // Gerar uma posição válida (não dentro de prédios) para o jogador spawnar
    Vector2 posicaoInicial = {400, 300}; // Posição padrão no centro

    // Verificar se a posição padrão colide com prédio
    if (verificarColisaoMapa(posicaoInicial, 15.0f, mapaDoJogo)) {
        // Se colidir, gerar uma posição válida aleatória
        posicaoInicial = gerarPosicaoValidaSpawn(mapaDoJogo, 15.0f);
    }

    jogador->posicao = posicaoInicial;
    jogador->vida = 100;
    jogador->tempoTotal = 0.0f;  // Inicializa o tempo
    jogador->fase = 1;           // Começa na fase 1
    jogador->municao = 30;
    jogador->pontos = 0;
    jogador->direcaoVertical = 0;   // Começa olhando para frente
    jogador->direcaoHorizontal = 1; // Começa olhando para direita
}

// Função para atualizar a lógica do jogo
void atualizarJogo(Player *jogador, Zumbi **zumbis, Bala **balas) {
    // Salvar posição anterior
    Vector2 posicaoAnterior = jogador->posicao;

    // Movimento do jogador com WASD
    float velocidade = 200 * GetFrameTime();

    // Atualizar direção vertical baseado no movimento
    if (IsKeyDown(KEY_W)) {
        jogador->posicao.y -= velocidade;
        jogador->direcaoVertical = 1; // Costas (indo para cima)
    }
    if (IsKeyDown(KEY_S)) {
        jogador->posicao.y += velocidade;
        jogador->direcaoVertical = 0; // Frente (indo para baixo)
    }

    // Atualizar direção horizontal baseado no movimento
    if (IsKeyDown(KEY_A)) {
        jogador->posicao.x -= velocidade;
        jogador->direcaoHorizontal = 0; // Esquerda
    }
    if (IsKeyDown(KEY_D)) {
        jogador->posicao.x += velocidade;
        jogador->direcaoHorizontal = 1; // Direita
    }

    // Verificar colisão com o mapa
    if (verificarColisaoMapa(jogador->posicao, 15.0f, mapaDoJogo)) {
        // Se houve colisão, voltar para a posição anterior
        jogador->posicao = posicaoAnterior;
    }

    // Limitar o jogador dentro da tela
    if (jogador->posicao.x < 20) jogador->posicao.x = 20;
    if (jogador->posicao.x > 780) jogador->posicao.x = 780;
    if (jogador->posicao.y < 20) jogador->posicao.y = 20;
    if (jogador->posicao.y > 580) jogador->posicao.y = 580;
    
    // Atirar com o botão esquerdo do mouse
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && jogador->municao > 0) {
        Vector2 mousePos = GetMousePosition();
        adicionarBala(balas, jogador->posicao, mousePos, 0, 10.0f);  // tipo 0 = jogador, dano = 10
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
    
    // Atualizar o tempo total se o jogador estiver vivo
    if (jogador->vida > 0) {
        jogador->tempoTotal += GetFrameTime();
    }
}

// Função para desenhar todos os elementos do jogo
void desenharJogo(Player *jogador, Zumbi *zumbis, Bala *balas, Texture2D texturaMapa) {
    // Desenhar o mapa primeiro (fundo)
    desenharMapa(mapaDoJogo, texturaMapa);

    // Desenhar os zumbis
    desenharZumbis(zumbis);

    // Desenhar o jogador
    if (jogador->spriteAtual.id > 0) {
        // Desenhar sprite centralizado na posição do jogador
        float escala = 0.06f; // Ajuste conforme o tamanho do seu sprite
        float largura = jogador->spriteAtual.width * escala;
        float altura = jogador->spriteAtual.height * escala;

        Rectangle destino = {
            jogador->posicao.x - largura / 2,
            jogador->posicao.y - altura / 2,
            largura,
            altura
        };

        Rectangle origem = {0, 0, (float)jogador->spriteAtual.width, (float)jogador->spriteAtual.height};

        DrawTexturePro(jogador->spriteAtual, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        // Fallback: desenhar círculo se sprite não carregou
        DrawCircleV(jogador->posicao, 15, BLUE);
    }

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
    
    // Tempo e fase
    int minutos = (int)jogador->tempoTotal / 60;
    float segundos = fmod(jogador->tempoTotal, 60.0f);
    DrawText(TextFormat("Tempo: %02d:%05.2f", minutos, segundos), 10, 65, 20, GOLD);
    DrawText(TextFormat("Fase: %d/3", jogador->fase), 10, 90, 20, WHITE);
    
    // Instruções
    DrawText("WASD - Mover | Mouse - Mirar | Click - Atirar", 200, 570, 15, LIGHTGRAY);
    
    // Aviso de Game Over
    if (jogador->vida <= 0) {
        // Formatar o tempo final
        int minutos = (int)jogador->tempoTotal / 60;
        float segundos = fmod(jogador->tempoTotal, 60.0f);
        
        // Carregar os melhores tempos para comparação
        float tempos[MAX_SCORES];
        loadTimes(tempos, MAX_SCORES);
        
        // Verificar se o tempo entra no ranking
        int posicao = -1;
        for (int i = 0; i < MAX_SCORES; i++) {
            if (jogador->tempoTotal < tempos[i]) {
                posicao = i + 1;
                break;
            }
        }
        
        // Quando o jogador morre, salvamos o tempo se for um recorde
        checkAndSaveTime(jogador->tempoTotal);
        
        // Fundo escuro semi-transparente
        DrawRectangle(0, 0, 800, 600, (Color){0, 0, 0, 150});
        
        // Mensagens de game over
        DrawText("GAME OVER", 250, 200, 60, RED);
        DrawText(TextFormat("Seu tempo: %02d:%05.2f", minutos, segundos), 280, 280, 30, WHITE);
        
        if (posicao > 0) {
            DrawText(TextFormat("NOVO RECORDE! %dº Lugar!", posicao), 250, 320, 30, GOLD);
        }
        
        DrawText("Pressione ESC para sair", 260, 380, 20, LIGHTGRAY);
    }
}
// --- Funções do Módulo de Zumbis ---

// Função para adicionar um novo Zumbi na Lista Encadeada
void adicionarZumbi(Zumbi **cabeca, Vector2 posInicial, Texture2D sprites[][4]) {

    // 1. Alocação Dinâmica de Memória
    Zumbi *novoZumbi = (Zumbi *)malloc(sizeof(Zumbi));

    if (novoZumbi == NULL) {
        printf("ERRO: Falha na alocacao de memoria para novo Zumbi!\n");
        return;
    }

    // 2. Verificar se a posição inicial é válida (não está em um prédio)
    // Se estiver em um prédio, gerar uma posição válida
    if (verificarColisaoMapa(posInicial, 20.0f, mapaDoJogo)) {
        posInicial = gerarPosicaoValidaSpawn(mapaDoJogo, 20.0f);
    }

    // 3. Inicializar o novo zumbi com comportamento aleatório
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

    // Atribuir skin aleatória (0-4 = 5 tipos diferentes)
    novoZumbi->tipoSkin = GetRandomValue(0, 4);

    // Atribuir sprites baseado na skin escolhida
    novoZumbi->spriteFrenteDireita = sprites[novoZumbi->tipoSkin][0];
    novoZumbi->spriteFrenteEsquerda = sprites[novoZumbi->tipoSkin][1];
    novoZumbi->spriteCostasDireita = sprites[novoZumbi->tipoSkin][2];
    novoZumbi->spriteCostasEsquerda = sprites[novoZumbi->tipoSkin][3];

    // Inicializar direções (começa olhando para frente/direita)
    novoZumbi->direcaoVertical = 0;
    novoZumbi->direcaoHorizontal = 1;
    novoZumbi->spriteAtual = novoZumbi->spriteFrenteDireita;

    // 4. Inserir no início da lista
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

        // Atualizar direções do sprite baseado no movimento
        // Direção vertical: se está indo mais para cima (velocidade.y negativa) = costas, senão = frente
        if (atual->velocidade.y < -5.0f) {
            atual->direcaoVertical = 1; // Costas (indo para cima)
        } else if (atual->velocidade.y > 5.0f) {
            atual->direcaoVertical = 0; // Frente (indo para baixo)
        }

        // Direção horizontal: se está indo mais para esquerda (velocidade.x negativa) = esquerda, senão = direita
        if (atual->velocidade.x < -5.0f) {
            atual->direcaoHorizontal = 0; // Esquerda
        } else if (atual->velocidade.x > 5.0f) {
            atual->direcaoHorizontal = 1; // Direita
        }

        // Selecionar sprite correto baseado nas direções
        if (atual->direcaoVertical == 0) { // Frente
            if (atual->direcaoHorizontal == 0) {
                atual->spriteAtual = atual->spriteFrenteEsquerda;
            } else {
                atual->spriteAtual = atual->spriteFrenteDireita;
            }
        } else { // Costas
            if (atual->direcaoHorizontal == 0) {
                atual->spriteAtual = atual->spriteCostasEsquerda;
            } else {
                atual->spriteAtual = atual->spriteCostasDireita;
            }
        }

        // Salvar posição anterior do zumbi
        Vector2 posicaoAnteriorZumbi = atual->posicao;

        // Atualizar posição
        atual->posicao.x += atual->velocidade.x * deltaTime;
        atual->posicao.y += atual->velocidade.y * deltaTime;

        // Verificar colisão com o mapa
        if (verificarColisaoMapa(atual->posicao, atual->raio, mapaDoJogo)) {
            // Se colidiu com prédio, voltar para posição anterior
            atual->posicao = posicaoAnteriorZumbi;
        }

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
        // Desenhar sprite do zumbi
        if (atual->spriteAtual.id > 0) {
            // Desenhar sprite centralizado na posição do zumbi
            float escala = 0.07f; // Zumbis ligeiramente maiores que o jogador
            float largura = atual->spriteAtual.width * escala;
            float altura = atual->spriteAtual.height * escala;

            Rectangle destino = {
                atual->posicao.x - largura / 2,
                atual->posicao.y - altura / 2,
                largura,
                altura
            };

            Rectangle origem = {0, 0, (float)atual->spriteAtual.width, (float)atual->spriteAtual.height};

            DrawTexturePro(atual->spriteAtual, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            // Fallback: desenhar círculo verde se sprite não carregou
            DrawCircleV(atual->posicao, atual->raio, GREEN);
        }

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
            // Verificar colisão usando o raio da bala
            if (verificarColisaoCirculos(balaAtual->posicao, balaAtual->raio, zumbiAtual->posicao, zumbiAtual->raio)) {
                // Aplicar dano ao zumbi usando o dano específico da bala
                zumbiAtual->vida -= (int)balaAtual->dano;

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
                Vector2 posicaoAnteriorKnockback = jogador->posicao; // Salvar posição antes do knockback

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

                    // Verificar se colidiu com prédio após knockback
                    if (verificarColisaoMapa(jogador->posicao, 15.0f, mapaDoJogo)) {
                        // Se colidiu, voltar para posição anterior
                        jogador->posicao = posicaoAnteriorKnockback;
                    }

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

// ===== MÓDULO DE ZUMBI FORTE =====

// Função para adicionar um novo Zumbi Forte na Lista Encadeada
void adicionarZumbiForte(ZumbiForte **cabeca, Vector2 posInicial) {
    // 1. Alocação Dinâmica de Memória
    ZumbiForte *novoZumbiForte = (ZumbiForte *)malloc(sizeof(ZumbiForte));

    if (novoZumbiForte == NULL) {
        printf("ERRO: Falha na alocacao de memoria para novo Zumbi Forte!\n");
        return;
    }

    // 2. Verificar se a posição inicial é válida
    if (verificarColisaoMapa(posInicial, 30.0f, mapaDoJogo)) {
        posInicial = gerarPosicaoValidaSpawn(mapaDoJogo, 30.0f);
    }

    // 3. Inicializar o Zumbi Forte com atributos superiores
    novoZumbiForte->posicao = posInicial;
    novoZumbiForte->velocidade = (Vector2){0, 0};
    novoZumbiForte->vida = 300;              // 3x mais vida que zumbi normal
    novoZumbiForte->raio = 30.0f;            // Maior que zumbi normal (20.0f)
    novoZumbiForte->dano = 20;               // Dano dobrado
    novoZumbiForte->armadura = 0.5f;         // Reduz 50% do dano recebido
    novoZumbiForte->cor = (Color){139, 0, 0, 255}; // Vermelho escuro

    // Tipo de movimento aleatório
    novoZumbiForte->tipoMovimento = GetRandomValue(0, 3);

    // Velocidade reduzida (mais lento mas mais resistente)
    novoZumbiForte->velocidadeBase = 20.0f + GetRandomValue(0, 20); // 20-40 pixels/s

    // Inicializar timers e ângulos
    novoZumbiForte->tempoDesvio = 0.0f;
    novoZumbiForte->anguloDesvio = (float)GetRandomValue(0, 360) * DEG2RAD;

    // 4. Inserir no início da lista
    novoZumbiForte->proximo = *cabeca;
    *cabeca = novoZumbiForte;

    printf("Zumbi Forte adicionado! Vida: %d, Dano: %d, Armadura: %.0f%%\n", 
           novoZumbiForte->vida, novoZumbiForte->dano, novoZumbiForte->armadura * 100);
}

// Função para atualizar todos os Zumbis Fortes
void atualizarZumbisFortes(ZumbiForte **cabeca, Vector2 posicaoJogador, float deltaTime) {
    ZumbiForte *atual = *cabeca;
    ZumbiForte *anterior = NULL;
    
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
            case 0: // DIRETO
                break;
                
            case 1: // ZIGZAG
                atual->tempoDesvio += deltaTime * 3.0f;
                {
                    float desvio = sinf(atual->tempoDesvio) * 0.5f;
                    direcaoFinal.x += -direcao.y * desvio;
                    direcaoFinal.y += direcao.x * desvio;
                    
                    float mag = sqrtf(direcaoFinal.x * direcaoFinal.x + 
                                     direcaoFinal.y * direcaoFinal.y);
                    if (mag > 0) {
                        direcaoFinal.x /= mag;
                        direcaoFinal.y /= mag;
                    }
                }
                break;
                
            case 2: // CIRCULAR
                atual->tempoDesvio += deltaTime * 2.0f;
                atual->anguloDesvio += deltaTime * 2.0f;
                {
                    float cosA = cosf(atual->anguloDesvio);
                    float sinA = sinf(atual->anguloDesvio);
                    direcaoFinal.x = direcao.x * 0.7f + (-direcao.y * cosA) * 0.3f;
                    direcaoFinal.y = direcao.y * 0.7f + (direcao.x * sinA) * 0.3f;
                    
                    float mag = sqrtf(direcaoFinal.x * direcaoFinal.x + 
                                     direcaoFinal.y * direcaoFinal.y);
                    if (mag > 0) {
                        direcaoFinal.x /= mag;
                        direcaoFinal.y /= mag;
                    }
                }
                break;
                
            case 3: // IMPREVISÍVEL
                atual->tempoDesvio += deltaTime;
                if (atual->tempoDesvio > 1.0f) {
                    atual->tempoDesvio = 0.0f;
                    atual->anguloDesvio = (float)GetRandomValue(-45, 45) * DEG2RAD;
                }
                {
                    float cosA = cosf(atual->anguloDesvio);
                    float sinA = sinf(atual->anguloDesvio);
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

        // Salvar posição anterior
        Vector2 posicaoAnteriorZumbi = atual->posicao;

        // Atualizar posição
        atual->posicao.x += atual->velocidade.x * deltaTime;
        atual->posicao.y += atual->velocidade.y * deltaTime;

        // Verificar colisão com o mapa
        if (verificarColisaoMapa(atual->posicao, atual->raio, mapaDoJogo)) {
            atual->posicao = posicaoAnteriorZumbi;
            
            // Tentar contornar o obstáculo
            Vector2 direcaoLateral1 = {-direcao.y, direcao.x};
            Vector2 tentativa1 = {
                posicaoAnteriorZumbi.x + direcaoLateral1.x * velocidadeFinal * deltaTime,
                posicaoAnteriorZumbi.y + direcaoLateral1.y * velocidadeFinal * deltaTime
            };
            
            if (!verificarColisaoMapa(tentativa1, atual->raio, mapaDoJogo)) {
                atual->posicao = tentativa1;
            } else {
                Vector2 direcaoLateral2 = {direcao.y, -direcao.x};
                Vector2 tentativa2 = {
                    posicaoAnteriorZumbi.x + direcaoLateral2.x * velocidadeFinal * deltaTime,
                    posicaoAnteriorZumbi.y + direcaoLateral2.y * velocidadeFinal * deltaTime
                };
                
                if (!verificarColisaoMapa(tentativa2, atual->raio, mapaDoJogo)) {
                    atual->posicao = tentativa2;
                }
            }
        }

        // Limitar dentro da tela
        if (atual->posicao.x < atual->raio) atual->posicao.x = atual->raio;
        if (atual->posicao.x > 800 - atual->raio) atual->posicao.x = 800 - atual->raio;
        if (atual->posicao.y < atual->raio) atual->posicao.y = atual->raio;
        if (atual->posicao.y > 600 - atual->raio) atual->posicao.y = 600 - atual->raio;

        // Remover se morreu
        if (atual->vida <= 0) {
            ZumbiForte *temp = atual;
            
            if (anterior == NULL) {
                *cabeca = atual->proximo;
                atual = *cabeca;
            } else {
                anterior->proximo = atual->proximo;
                atual = atual->proximo;
            }
            
            free(temp);
            printf("Zumbi Forte eliminado!\n");
        } else {
            anterior = atual;
            atual = atual->proximo;
        }
    }
}

// Função para desenhar todos os Zumbis Fortes
void desenharZumbisFortes(ZumbiForte *cabeca) {
    ZumbiForte *atual = cabeca;
    
    while (atual != NULL) {
        // Desenhar o corpo do zumbi forte (vermelho escuro)
        DrawCircleV(atual->posicao, atual->raio, atual->cor);
        
        // Desenhar contorno grosso para destacar
        DrawCircleLines((int)atual->posicao.x, (int)atual->posicao.y, atual->raio, (Color){255, 0, 0, 255});
        DrawCircleLines((int)atual->posicao.x, (int)atual->posicao.y, atual->raio - 2, (Color){255, 0, 0, 255});
        
        // Desenhar barra de vida acima do zumbi
        float larguraBarra = atual->raio * 2;
        float alturaBarra = 5;
        float vidaPercent = (float)atual->vida / 300.0f;
        
        // Fundo da barra (preto)
        DrawRectangle((int)(atual->posicao.x - larguraBarra / 2), 
                     (int)(atual->posicao.y - atual->raio - 10),
                     (int)larguraBarra, (int)alturaBarra, BLACK);
        
        // Barra de vida (verde -> amarelo -> vermelho)
        Color corVida = GREEN;
        if (vidaPercent <= 0.3f) {
            corVida = RED;
        } else if (vidaPercent <= 0.6f) {
            corVida = ORANGE;
        }
        
        DrawRectangle((int)(atual->posicao.x - larguraBarra / 2), 
                     (int)(atual->posicao.y - atual->raio - 10),
                     (int)(larguraBarra * vidaPercent), (int)alturaBarra, corVida);
        
        // Texto indicando que é um zumbi forte
        DrawText("FORTE", (int)(atual->posicao.x - 20), (int)(atual->posicao.y - 5), 10, WHITE);
        
        atual = atual->proximo;
    }
}

// Função para liberar memória de todos os Zumbis Fortes
void liberarZumbisFortes(ZumbiForte **cabeca) {
    ZumbiForte *atual = *cabeca;
    ZumbiForte *proximo;
    
    while (atual != NULL) {
        proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    
    *cabeca = NULL;
}

// Função para verificar colisões entre balas e zumbis fortes
void verificarColisoesBalaZumbiForte(Bala **balas, ZumbiForte **zumbisFortes, Player *jogador) {
    Bala *balaAtual = *balas;
    Bala *balaAnterior = NULL;

    while (balaAtual != NULL) {
        int balaRemovida = 0;
        ZumbiForte *zumbiAtual = *zumbisFortes;
        ZumbiForte *zumbiAnterior = NULL;

        while (zumbiAtual != NULL && !balaRemovida) {
            // Verificar colisão entre bala e zumbi forte
            if (verificarColisaoCirculos(balaAtual->posicao, 5.0f, zumbiAtual->posicao, zumbiAtual->raio)) {
                // Calcular dano com armadura
                int danoBase = 50;
                int danoReal = (int)(danoBase * (1.0f - zumbiAtual->armadura));
                zumbiAtual->vida -= danoReal;

                printf("Zumbi Forte atingido! Dano: %d (Base: %d, Armadura: %.0f%%). Vida restante: %d\n",
                       danoReal, danoBase, zumbiAtual->armadura * 100, zumbiAtual->vida);

                // Remover a bala
                Bala *balaRemover = balaAtual;
                if (balaAnterior == NULL) {
                    *balas = balaAtual->proximo;
                } else {
                    balaAnterior->proximo = balaAtual->proximo;
                }
                balaAtual = balaAtual->proximo;
                free(balaRemover);
                balaRemovida = 1;

                // Se zumbi forte morreu, dar mais pontos
                if (zumbiAtual->vida <= 0) {
                    jogador->pontos += 50;
                    printf("Zumbi Forte eliminado! +50 pontos. Total: %d\n", jogador->pontos);
                }
            } else {
                zumbiAnterior = zumbiAtual;
                zumbiAtual = zumbiAtual->proximo;
            }
        }

        if (!balaRemovida) {
            balaAnterior = balaAtual;
            balaAtual = balaAtual->proximo;
        }
    }
}

// Função para verificar colisões entre jogador e zumbis fortes
void verificarColisoesJogadorZumbiForte(Player *jogador, ZumbiForte *zumbisFortes) {
    static float cooldownDanoForte = 0.0f;
    static float flashDanoForte = 0.0f;
    
    cooldownDanoForte -= GetFrameTime();
    if (flashDanoForte > 0.0f) {
        flashDanoForte -= GetFrameTime();
    }
    
    ZumbiForte *zumbiAtual = zumbisFortes;

    while (zumbiAtual != NULL) {
        if (verificarColisaoCirculos(jogador->posicao, 15.0f, zumbiAtual->posicao, zumbiAtual->raio)) {
            
            if (cooldownDanoForte <= 0.0f) {
                int dano = zumbiAtual->dano;
                jogador->vida -= dano;
                cooldownDanoForte = 0.5f;
                flashDanoForte = 0.2f;
                
                printf("OUCH FORTE! Jogador recebeu %d de dano. Vida: %d\n", dano, jogador->vida);
                
                Vector2 direcaoEmpurrao = {
                    jogador->posicao.x - zumbiAtual->posicao.x,
                    jogador->posicao.y - zumbiAtual->posicao.y
                };
                
                float magnitude = sqrtf(direcaoEmpurrao.x * direcaoEmpurrao.x + 
                                       direcaoEmpurrao.y * direcaoEmpurrao.y);
                
                if (magnitude > 0) {
                    direcaoEmpurrao.x /= magnitude;
                    direcaoEmpurrao.y /= magnitude;
                    
                    float forcaEmpurrao = 60.0f;
                    jogador->posicao.x += direcaoEmpurrao.x * forcaEmpurrao;
                    jogador->posicao.y += direcaoEmpurrao.y * forcaEmpurrao;
                    
                    if (jogador->posicao.x < 20) jogador->posicao.x = 20;
                    if (jogador->posicao.x > 780) jogador->posicao.x = 780;
                    if (jogador->posicao.y < 20) jogador->posicao.y = 20;
                    if (jogador->posicao.y > 580) jogador->posicao.y = 580;
                }
            }

            if (jogador->vida < 0) {
                jogador->vida = 0;
            }
        }

        zumbiAtual = zumbiAtual->proximo;
    }
    
    if (flashDanoForte > 0.0f) {
        int alpha = (int)(flashDanoForte * 255.0f);
        DrawRectangle(0, 0, 800, 600, (Color){255, 0, 0, alpha});
    }
}

// Função para verificar colisões entre zumbis fortes
void verificarColisoesZumbiForteZumbiForte(ZumbiForte *zumbisFortes) {
    ZumbiForte *zumbi1 = zumbisFortes;

    while (zumbi1 != NULL) {
        ZumbiForte *zumbi2 = zumbi1->proximo;

        while (zumbi2 != NULL) {
            if (verificarColisaoCirculos(zumbi1->posicao, zumbi1->raio, zumbi2->posicao, zumbi2->raio)) {
                float dx = zumbi2->posicao.x - zumbi1->posicao.x;
                float dy = zumbi2->posicao.y - zumbi1->posicao.y;
                float distancia = sqrtf(dx * dx + dy * dy);

                if (distancia > 0) {
                    float nx = dx / distancia;
                    float ny = dy / distancia;
                    float sobreposicao = (zumbi1->raio + zumbi2->raio) - distancia;
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

// ===== SISTEMA DE CHAVE =====

void criarChave(Chave* chave, Vector2 posicao) {
    chave->posicao = posicao;
    chave->raio = 15.0f;
    chave->ativa = true;
    chave->coletada = false;
}

void desenharChave(Chave* chave) {
    if (!chave->ativa || chave->coletada) return;
    
    // Efeito de brilho pulsante
    static float tempo = 0;
    tempo += 0.05f;
    float pulso = sinf(tempo) * 3.0f;
    
    // Brilho externo
    DrawCircleV(chave->posicao, chave->raio + pulso, (Color){255, 215, 0, 100});
    
    // Corpo da chave
    DrawCircleV(chave->posicao, chave->raio, GOLD);
    DrawCircleV((Vector2){chave->posicao.x, chave->posicao.y}, 8, YELLOW);
    
    // Detalhes da chave
    DrawRectangle(chave->posicao.x - 2, chave->posicao.y + 8, 4, 15, GOLD);
    DrawRectangle(chave->posicao.x - 2, chave->posicao.y + 18, 8, 3, GOLD);
    DrawRectangle(chave->posicao.x - 2, chave->posicao.y + 23, 8, 3, GOLD);
    
    // Texto flutuante
    const char* texto = "CHAVE";
    int larguraTexto = MeasureText(texto, 15);
    DrawText(texto, chave->posicao.x - larguraTexto/2, chave->posicao.y - 30, 15, YELLOW);
}

bool verificarColetaChave(Chave* chave, Player* jogador) {
    if (!chave->ativa || chave->coletada) return false;
    
    float dx = chave->posicao.x - jogador->posicao.x;
    float dy = chave->posicao.y - jogador->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    
    if (distancia < chave->raio + 15.0f) {
        chave->coletada = true;
        return true;
    }
    return false;
}

// ===== SISTEMA DE PORTA =====

void criarPorta(Porta* porta, Vector2 posicao) {
    porta->posicao = posicao;
    porta->largura = 60;
    porta->altura = 100;
    porta->trancada = true;
    porta->aberta = false;
}

void desenharPorta(Porta* porta) {
    Color corPorta = porta->trancada ? DARKBROWN : BROWN;
    
    // Moldura da porta
    DrawRectangle(porta->posicao.x - 5, porta->posicao.y - 5, 
                  porta->largura + 10, porta->altura + 10, DARKGRAY);
    
    // Porta
    DrawRectangle(porta->posicao.x, porta->posicao.y, 
                  porta->largura, porta->altura, corPorta);
    
    // Detalhes da porta
    DrawRectangle(porta->posicao.x + 5, porta->posicao.y + 5, 
                  porta->largura - 10, porta->altura - 10, Fade(BLACK, 0.3f));
    
    if (porta->trancada) {
        // Desenhar cadeado
        DrawCircle(porta->posicao.x + porta->largura/2, 
                   porta->posicao.y + porta->altura/2, 12, GRAY);
        DrawCircle(porta->posicao.x + porta->largura/2, 
                   porta->posicao.y + porta->altura/2, 10, DARKGRAY);
        DrawRectangle(porta->posicao.x + porta->largura/2 - 6, 
                     porta->posicao.y + porta->altura/2, 12, 15, DARKGRAY);
        
        // Texto
        const char* texto = "TRANCADA";
        int larguraTexto = MeasureText(texto, 15);
        DrawText(texto, porta->posicao.x + porta->largura/2 - larguraTexto/2, 
                 porta->posicao.y - 25, 15, RED);
    } else {
        // Texto quando pode passar
        const char* texto = "PASSAGEM ABERTA";
        int larguraTexto = MeasureText(texto, 15);
        DrawText(texto, porta->posicao.x + porta->largura/2 - larguraTexto/2, 
                 porta->posicao.y - 25, 15, GREEN);
    }
}

bool verificarInteracaoPorta(Porta* porta, Player* jogador, bool temChave) {
    // Calcular distância
    float dx = (porta->posicao.x + porta->largura/2) - jogador->posicao.x;
    float dy = (porta->posicao.y + porta->altura/2) - jogador->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    
    if (distancia < 50) {
        if (porta->trancada && temChave) {
            porta->trancada = false;
        }
        
        // Passa automaticamente se a porta estiver destrancada
        if (!porta->trancada) {
            porta->aberta = true;
            return true;
        }
    }
    
    return false;
}

// ===== SISTEMA DE MAPAS =====

void carregarMapa(Mapa* mapa, int idMapa) {
    mapa->id = idMapa;
    
    switch(idMapa) {
        case 1: // Mapa inicial
            mapa->corFundo = DARKGREEN;
            mapa->spawnJogador = (Vector2){400, 300};
            mapa->numZumbis = 5;
            mapa->numZumbisFortes = 1;
            mapa->temChave = true;
            mapa->temPorta = true;
            break;
            
        case 2: // Segundo mapa
            mapa->corFundo = DARKPURPLE;
            mapa->spawnJogador = (Vector2){100, 300};
            mapa->numZumbis = 8;
            mapa->numZumbisFortes = 2;
            mapa->temChave = false;
            mapa->temPorta = false;
            break;
            
        default:
            mapa->corFundo = BLACK;
            mapa->spawnJogador = (Vector2){400, 300};
            mapa->numZumbis = 3;
            mapa->numZumbisFortes = 0;
            mapa->temChave = false;
            mapa->temPorta = false;
            break;
    }
}