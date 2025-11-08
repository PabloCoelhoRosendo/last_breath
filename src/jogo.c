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

// --- Funções do Sistema de Armas ---

// Função para inicializar uma arma com base no tipo
void inicializarArma(Arma *arma, TipoArma tipo) {
    arma->tipo = tipo;
    arma->cooldown = 0.0f;
    
    switch(tipo) {
        case ARMA_PISTOL:
            arma->dano = 10;
            arma->taxaTiroMS = 400;
            arma->penteMax = 12;
            arma->penteAtual = 12;
            arma->municaoTotal = 60;  // 5 pentes extras
            arma->tempoRecarga = 2.0f;
            break;
            
        case ARMA_SHOTGUN:
            arma->dano = 35;
            arma->taxaTiroMS = 1000;
            arma->penteMax = 6;
            arma->penteAtual = 6;
            arma->municaoTotal = 24;  // 4 pentes extras
            arma->tempoRecarga = 3.0f;
            break;
            
        case ARMA_SMG:
            arma->dano = 8;
            arma->taxaTiroMS = 100;
            arma->penteMax = 30;
            arma->penteAtual = 30;
            arma->municaoTotal = 120;  // 4 pentes extras
            arma->tempoRecarga = 2.5f;
            break;
            
        case ARMA_NENHUMA:
        default:
            arma->dano = 0;
            arma->taxaTiroMS = 0;
            arma->penteMax = 0;
            arma->penteAtual = 0;
            arma->municaoTotal = 0;
            arma->tempoRecarga = 0.0f;
            break;
    }
}

// Função para equipar uma arma de um slot específico
void equiparArma(Player *jogador, int slot) {
    // Verificar se o slot é válido (0, 1 ou 2)
    if (slot < 0 || slot > 2) return;
    
    // Verificar se o slot tem uma arma
    if (jogador->slots[slot].tipo == ARMA_NENHUMA) return;
    
    // Equipar a arma
    jogador->slotAtivo = slot;
}

// Função para recarregar a arma equipada
void recarregarArma(Player *jogador) {
    if (jogador->estaRecarregando) return;
    
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];
    
    // Verificar se precisa recarregar
    if (armaAtual->penteAtual >= armaAtual->penteMax) return;
    if (armaAtual->municaoTotal <= 0) return;
    
    // Iniciar recarga
    jogador->estaRecarregando = true;
    jogador->tempoRecargaAtual = armaAtual->tempoRecarga;
}

// Função para atirar com a arma equipada
void atirarArma(Player *jogador, Bala **balas, Vector2 alvo) {
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];
    
    // Verificar se pode atirar
    if (armaAtual->penteAtual <= 0) return;
    if (armaAtual->cooldown > 0.0f) return;
    if (jogador->estaRecarregando) return;
    
    // Adicionar bala
    adicionarBala(balas, jogador->posicao, alvo, 0, (float)armaAtual->dano);
    
    // Reduzir munição
    armaAtual->penteAtual--;
    
    // Aplicar cooldown (converter MS para segundos)
    armaAtual->cooldown = armaAtual->taxaTiroMS / 1000.0f;
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
    jogador->tempoTotal = 0.0f;
    jogador->fase = 1;
    jogador->velocidadeBase = 4.0f;  // Velocidade inicial 4.0 m/s
    jogador->direcaoVertical = 0;
    jogador->direcaoHorizontal = 1;
    jogador->estaRecarregando = false;
    jogador->tempoRecargaAtual = 0.0f;
    jogador->tempoJaSalvo = false;
    
    // Inicializar sistema de boss
    jogador->timerBoss = 0.0f;
    jogador->bossSpawnado = false;
    
    // Inicializar sistema de itens
    jogador->temChave = false;
    jogador->temMapa = false;
    jogador->temCure = false;
    
    // Inicializar slots de arma
    inicializarArma(&jogador->slots[0], ARMA_PISTOL);   // Slot 1: Pistol (inicial)
    inicializarArma(&jogador->slots[1], ARMA_NENHUMA);  // Slot 2: Vazio
    inicializarArma(&jogador->slots[2], ARMA_NENHUMA);  // Slot 3: Vazio
    jogador->slotAtivo = 0;  // Começa com a Pistol equipada
}

// Função para atualizar a lógica do jogo
void atualizarJogo(Player *jogador, Zumbi **zumbis, Bala **balas) {
    float deltaTime = GetFrameTime();
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];
    
    // Atualizar cooldown da arma
    if (armaAtual->cooldown > 0.0f) {
        armaAtual->cooldown -= deltaTime;
    }
    
    // Sistema de Recarga
    if (jogador->estaRecarregando) {
        jogador->tempoRecargaAtual -= deltaTime;
        
        if (jogador->tempoRecargaAtual <= 0.0f) {
            // Recarga completa
            int municaoNecessaria = armaAtual->penteMax - armaAtual->penteAtual;
            int municaoDisponivel = armaAtual->municaoTotal;
            int municaoRecarregar = (municaoNecessaria <= municaoDisponivel) ? municaoNecessaria : municaoDisponivel;
            
            armaAtual->penteAtual += municaoRecarregar;
            armaAtual->municaoTotal -= municaoRecarregar;
            
            jogador->estaRecarregando = false;
            jogador->tempoRecargaAtual = 0.0f;
        }
    }
    
    // Tecla R para recarregar
    if (IsKeyPressed(KEY_R) && !jogador->estaRecarregando) {
        recarregarArma(jogador);
    }
    
    // Teclas 1, 2, 3 para trocar de arma
    if (IsKeyPressed(KEY_ONE)) {
        equiparArma(jogador, 0);
    }
    if (IsKeyPressed(KEY_TWO)) {
        equiparArma(jogador, 1);
    }
    if (IsKeyPressed(KEY_THREE)) {
        equiparArma(jogador, 2);
    }
    
    // Calcular velocidade baseada no estado
    float velocidadeAtual = jogador->velocidadeBase;
    if (jogador->estaRecarregando) {
        velocidadeAtual = 2.0f;  // Reduz para 2.0 m/s durante recarga
    }
    
    // Converter m/s para pixels/frame (assumindo 60 FPS e 1 pixel = 1 metro)
    float velocidade = velocidadeAtual * 60.0f * deltaTime;
    
    // Salvar posição anterior
    Vector2 posicaoAnterior = jogador->posicao;

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
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        atirarArma(jogador, balas, mousePos);
    }
    
    // Atualizar balas
    atualizarBalas(balas);

    // Atualizar zumbis
    atualizarZumbis(zumbis, jogador->posicao, deltaTime);

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
    
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];
    
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
    
    // Munição com reserva [Pente] / [Total]
    Color corMunicao = WHITE;
    if (armaAtual->penteAtual <= 5) {
        corMunicao = RED;
        // Piscar quando munição está baixa
        if ((int)(GetTime() * 2) % 2 == 0 && !jogador->estaRecarregando) {
            const char *avisoRecarga = "RECARREGUE!";
            int larguraTexto = MeasureText(avisoRecarga, 20);
            DrawText(avisoRecarga, 400 - larguraTexto / 2, 85, 20, RED);
        }
    }
    
    // Mostrar estado de recarga
    if (jogador->estaRecarregando) {
        DrawText("RECARREGANDO...", 10, 40, 20, YELLOW);
        // Barra de progresso da recarga
        float progresso = 1.0f - (jogador->tempoRecargaAtual / armaAtual->tempoRecarga);
        DrawRectangle(10, 65, 200, 10, DARKGRAY);
        DrawRectangle(10, 65, (int)(200 * progresso), 10, YELLOW);
        DrawRectangleLines(10, 65, 200, 10, WHITE);
    } else {
        DrawText(TextFormat("Municao: [%d] / %d", armaAtual->penteAtual, armaAtual->municaoTotal), 10, 40, 20, corMunicao);
    }
    
    // Tempo e fase
    int minutos = (int)jogador->tempoTotal / 60;
    float segundos = fmod(jogador->tempoTotal, 60.0f);
    DrawText(TextFormat("Tempo: %02d:%05.2f", minutos, segundos), 10, 90, 20, GOLD);
    DrawText(TextFormat("Fase: %d/3", jogador->fase), 10, 115, 20, WHITE);
    
    // HUD de Slots de Arma (Inferior Central)
    int hudX = 300;  // Centro horizontal
    int hudY = 550;  // Parte inferior
    int slotWidth = 60;
    int slotHeight = 60;
    int slotSpacing = 10;
    
    const char* nomesArmas[] = {"Vazio", "Pistol", "Shotgun", "SMG"};
    
    for (int i = 0; i < 3; i++) {
        int posX = hudX + (i * (slotWidth + slotSpacing));
        Color corSlot = DARKGRAY;
        Color corTexto = LIGHTGRAY;
        
        // Destaque para slot ativo
        if (i == jogador->slotAtivo) {
            corSlot = GREEN;
            corTexto = WHITE;
            DrawRectangle(posX - 2, hudY - 2, slotWidth + 4, slotHeight + 4, LIME);
        }
        
        // Desenhar slot
        DrawRectangle(posX, hudY, slotWidth, slotHeight, corSlot);
        DrawRectangleLines(posX, hudY, slotWidth, slotHeight, WHITE);
        
        // Número do slot
        DrawText(TextFormat("%d", i + 1), posX + 5, hudY + 5, 20, corTexto);
        
        // Nome da arma (se tiver)
        if (jogador->slots[i].tipo != ARMA_NENHUMA) {
            const char* nomeArma = nomesArmas[jogador->slots[i].tipo];
            DrawText(nomeArma, posX + 5, hudY + 30, 15, corTexto);
        } else {
            DrawText("---", posX + 15, hudY + 30, 15, GRAY);
        }
    }
    
    // Instruções atualizadas
    DrawText("WASD - Mover | 1,2,3 - Armas | R - Recarregar | Click - Atirar", 130, 520, 15, LIGHTGRAY);
    
    // Aviso de Game Over
    if (jogador->vida <= 0) {
        // Salvar tempo apenas uma vez
        if (!jogador->tempoJaSalvo) {
            checkAndSaveTime(jogador->tempoTotal);
            jogador->tempoJaSalvo = true;
        }
        
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
                    // Remover zumbi da lista (não precisa mais de pontos)

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

// ===== SISTEMA DE BOSS =====

// Função para criar um novo boss
void criarBoss(Boss **bosses, TipoBoss tipo, Vector2 posicao) {
    Boss *novoBoss = (Boss *)malloc(sizeof(Boss));
    
    novoBoss->tipo = tipo;
    novoBoss->posicao = posicao;
    novoBoss->ativo = true;
    novoBoss->atacando = false;
    novoBoss->tempoAtaque = 0.0f;
    novoBoss->padraoAtaque = 0;
    novoBoss->anguloRotacao = 0.0f;
    
    // Configurar stats específicos por tipo de boss
    switch (tipo) {
        case BOSS_PROWLER:
            novoBoss->vidaMax = 150;
            novoBoss->vida = 150;
            novoBoss->velocidade = 2.5f;
            novoBoss->raio = 25.0f;
            novoBoss->cooldownAtaque = 3.0f; // Slam a cada 3 segundos
            break;
            
        case BOSS_HUNTER:
            novoBoss->vidaMax = 80;
            novoBoss->vida = 80;
            novoBoss->velocidade = 6.0f;
            novoBoss->raio = 20.0f;
            novoBoss->cooldownAtaque = 0.0f; // Dano por contato contínuo
            break;
            
        case BOSS_ABOMINATION:
            novoBoss->vidaMax = 250;
            novoBoss->vida = 250;
            novoBoss->velocidade = 0.0f; // Estático
            novoBoss->raio = 35.0f;
            novoBoss->cooldownAtaque = 2.0f; // Ataque de projéteis a cada 2s
            break;
            
        default:
            novoBoss->vidaMax = 100;
            novoBoss->vida = 100;
            novoBoss->velocidade = 2.0f;
            novoBoss->raio = 20.0f;
            novoBoss->cooldownAtaque = 2.0f;
            break;
    }
    
    // Adicionar à lista encadeada
    novoBoss->proximo = *bosses;
    *bosses = novoBoss;
}

// Função para atualizar lógica dos bosses
void atualizarBoss(Boss **bosses, Player *jogador, Bala **balas, float deltaTime) {
    Boss *bossAtual = *bosses;
    
    while (bossAtual != NULL) {
        if (!bossAtual->ativo) {
            bossAtual = bossAtual->proximo;
            continue;
        }
        
        // Atualizar timer de ataque
        bossAtual->tempoAtaque += deltaTime;
        
        // Comportamento específico por tipo
        switch (bossAtual->tipo) {
            case BOSS_PROWLER: {
                // Perseguir o jogador
                float dx = jogador->posicao.x - bossAtual->posicao.x;
                float dy = jogador->posicao.y - bossAtual->posicao.y;
                float distancia = sqrtf(dx * dx + dy * dy);
                
                if (distancia > 0) {
                    bossAtual->posicao.x += (dx / distancia) * bossAtual->velocidade;
                    bossAtual->posicao.y += (dy / distancia) * bossAtual->velocidade;
                }
                
                // Ataque Slam (área de efeito)
                if (bossAtual->tempoAtaque >= bossAtual->cooldownAtaque) {
                    bossAtual->atacando = true;
                    
                    // Verificar se jogador está na área de efeito (raio de 80 pixels)
                    if (distancia <= 80.0f) {
                        jogador->vida -= 15; // Dano de área
                    }
                    
                    bossAtual->tempoAtaque = 0.0f;
                    bossAtual->atacando = false;
                }
                break;
            }
            
            case BOSS_HUNTER: {
                // Perseguição agressiva e rápida
                float dx = jogador->posicao.x - bossAtual->posicao.x;
                float dy = jogador->posicao.y - bossAtual->posicao.y;
                float distancia = sqrtf(dx * dx + dy * dy);
                
                if (distancia > 0) {
                    bossAtual->posicao.x += (dx / distancia) * bossAtual->velocidade;
                    bossAtual->posicao.y += (dy / distancia) * bossAtual->velocidade;
                }
                
                // Dano por contato (20 HP)
                if (distancia <= (bossAtual->raio + 20.0f)) {
                    if (bossAtual->tempoAtaque >= 1.0f) { // Cooldown de 1s entre danos
                        jogador->vida -= 20;
                        bossAtual->tempoAtaque = 0.0f;
                    }
                }
                break;
            }
            
            case BOSS_ABOMINATION: {
                // Boss estático com ataques de projéteis
                if (bossAtual->tempoAtaque >= bossAtual->cooldownAtaque) {
                    // Alternar entre 3 padrões de ataque
                    bossAtual->padraoAtaque = (bossAtual->padraoAtaque + 1) % 3;
                    
                    switch (bossAtual->padraoAtaque) {
                        case 0: { // Padrão 1: Rajada direta ao jogador
                            float dx = jogador->posicao.x - bossAtual->posicao.x;
                            float dy = jogador->posicao.y - bossAtual->posicao.y;
                            float distancia = sqrtf(dx * dx + dy * dy);
                            
                            if (distancia > 0) {
                                Bala *novaBala = (Bala *)malloc(sizeof(Bala));
                                novaBala->posicao = bossAtual->posicao;
                                novaBala->velocidade.x = (dx / distancia) * 5.0f;
                                novaBala->velocidade.y = (dy / distancia) * 5.0f;
                                novaBala->tipo = 1; // Projétil de boss
                                novaBala->dano = 25.0f;
                                novaBala->raio = 8.0f;
                                novaBala->tempoVida = 5.0f;
                                novaBala->proximo = *balas;
                                *balas = novaBala;
                            }
                            break;
                        }
                        
                        case 1: { // Padrão 2: Spray circular (8 projéteis)
                            for (int i = 0; i < 8; i++) {
                                float angulo = (360.0f / 8.0f) * i * DEG2RAD;
                                
                                Bala *novaBala = (Bala *)malloc(sizeof(Bala));
                                novaBala->posicao = bossAtual->posicao;
                                novaBala->velocidade.x = cosf(angulo) * 4.0f;
                                novaBala->velocidade.y = sinf(angulo) * 4.0f;
                                novaBala->tipo = 1;
                                novaBala->dano = 20.0f;
                                novaBala->raio = 6.0f;
                                novaBala->tempoVida = 5.0f;
                                novaBala->proximo = *balas;
                                *balas = novaBala;
                            }
                            break;
                        }
                        
                        case 2: { // Padrão 3: Espiral rotativa (3 projéteis)
                            for (int i = 0; i < 3; i++) {
                                float angulo = (bossAtual->anguloRotacao + (120.0f * i)) * DEG2RAD;
                                
                                Bala *novaBala = (Bala *)malloc(sizeof(Bala));
                                novaBala->posicao = bossAtual->posicao;
                                novaBala->velocidade.x = cosf(angulo) * 3.5f;
                                novaBala->velocidade.y = sinf(angulo) * 3.5f;
                                novaBala->tipo = 1;
                                novaBala->dano = 15.0f;
                                novaBala->raio = 7.0f;
                                novaBala->tempoVida = 5.0f;
                                novaBala->proximo = *balas;
                                *balas = novaBala;
                            }
                            
                            bossAtual->anguloRotacao += 15.0f; // Incrementar rotação
                            if (bossAtual->anguloRotacao >= 360.0f) {
                                bossAtual->anguloRotacao = 0.0f;
                            }
                            break;
                        }
                    }
                    
                    bossAtual->tempoAtaque = 0.0f;
                }
                break;
            }
            
            default:
                break;
        }
        
        bossAtual = bossAtual->proximo;
    }
}

// Função para desenhar bosses
void desenharBoss(Boss *bosses) {
    Boss *bossAtual = bosses;
    
    while (bossAtual != NULL) {
        if (!bossAtual->ativo) {
            bossAtual = bossAtual->proximo;
            continue;
        }
        
        // Cor baseada no tipo de boss
        Color corBoss;
        switch (bossAtual->tipo) {
            case BOSS_PROWLER:
                corBoss = PURPLE; // Roxo para Prowler
                break;
            case BOSS_HUNTER:
                corBoss = ORANGE; // Laranja para Hunter
                break;
            case BOSS_ABOMINATION:
                corBoss = DARKGREEN; // Verde escuro para Abomination
                break;
            default:
                corBoss = BLACK;
                break;
        }
        
        // Desenhar círculo do boss
        DrawCircleV(bossAtual->posicao, bossAtual->raio, corBoss);
        DrawCircleLines((int)bossAtual->posicao.x, (int)bossAtual->posicao.y, bossAtual->raio, DARKGRAY);
        
        // Desenhar barra de vida acima do boss
        float barraLargura = 60.0f;
        float barraAltura = 8.0f;
        float porcentagemVida = (float)bossAtual->vida / (float)bossAtual->vidaMax;
        
        Vector2 barraPos = {bossAtual->posicao.x - barraLargura / 2, bossAtual->posicao.y - bossAtual->raio - 15.0f};
        
        // Fundo da barra (vermelho)
        DrawRectangleV(barraPos, (Vector2){barraLargura, barraAltura}, RED);
        // Vida atual (verde)
        DrawRectangleV(barraPos, (Vector2){barraLargura * porcentagemVida, barraAltura}, GREEN);
        // Contorno
        DrawRectangleLinesEx((Rectangle){barraPos.x, barraPos.y, barraLargura, barraAltura}, 1, BLACK);
        
        // Indicador de ataque para Prowler
        if (bossAtual->tipo == BOSS_PROWLER && bossAtual->atacando) {
            DrawCircleLines((int)bossAtual->posicao.x, (int)bossAtual->posicao.y, 80.0f, RED);
        }
        
        bossAtual = bossAtual->proximo;
    }
}

// Função para verificar colisões entre boss e balas do jogador
void verificarColisoesBossBala(Boss **bosses, Bala **balas, Item *item) {
    Boss *bossAtual = *bosses;
    
    while (bossAtual != NULL) {
        if (!bossAtual->ativo) {
            bossAtual = bossAtual->proximo;
            continue;
        }
        
        Bala *balaAtual = *balas;
        Bala *balaAnterior = NULL;
        
        while (balaAtual != NULL) {
            // Verificar apenas balas do jogador (tipo 0)
            if (balaAtual->tipo == 0) {
                // Verificar colisão círculo-círculo
                float dx = bossAtual->posicao.x - balaAtual->posicao.x;
                float dy = bossAtual->posicao.y - balaAtual->posicao.y;
                float distancia = sqrtf(dx * dx + dy * dy);
                
                if (distancia <= (bossAtual->raio + balaAtual->raio)) {
                    // Aplicar dano ao boss
                    bossAtual->vida -= (int)balaAtual->dano;
                    
                    // Remover bala
                    if (balaAnterior == NULL) {
                        *balas = balaAtual->proximo;
                    } else {
                        balaAnterior->proximo = balaAtual->proximo;
                    }
                    Bala *balaTemp = balaAtual;
                    balaAtual = balaAtual->proximo;
                    free(balaTemp);
                    continue;
                }
            }
            
            balaAnterior = balaAtual;
            balaAtual = balaAtual->proximo;
        }
        
        // Verificar se o boss morreu e dropar item
        if (bossAtual->vida <= 0 && bossAtual->ativo) {
            bossAtual->ativo = false;
            
            // Dropar item específico baseado no tipo de boss
            if (item != NULL && !item->ativo) {
                switch (bossAtual->tipo) {
                    case BOSS_PROWLER:
                        criarItem(item, ITEM_CHAVE, bossAtual->posicao);
                        printf("Boss morreu! CHAVE dropada!\n");
                        break;
                    case BOSS_HUNTER:
                        criarItem(item, ITEM_MAPA, bossAtual->posicao);
                        printf("Boss morreu! MAPA dropado!\n");
                        break;
                    case BOSS_ABOMINATION:
                        criarItem(item, ITEM_CURE, bossAtual->posicao);
                        printf("Boss morreu! CURE dropada! VITORIA!\n");
                        break;
                    default:
                        break;
                }
            }
        }
        
        bossAtual = bossAtual->proximo;
    }
}

// Função para verificar colisões entre boss e jogador
void verificarColisoesBossJogador(Boss *bosses, Player *jogador) {
    Boss *bossAtual = bosses;
    
    while (bossAtual != NULL) {
        if (!bossAtual->ativo) {
            bossAtual = bossAtual->proximo;
            continue;
        }
        
        // Verificar colisão círculo-círculo
        float dx = jogador->posicao.x - bossAtual->posicao.x;
        float dy = jogador->posicao.y - bossAtual->posicao.y;
        float distancia = sqrtf(dx * dx + dy * dy);
        
        if (distancia <= (bossAtual->raio + 20.0f)) { // 20.0f = raio aproximado do jogador
            // Empurrar jogador para trás
            if (distancia > 0) {
                float forcaEmpurrao = 3.0f;
                jogador->posicao.x += (dx / distancia) * forcaEmpurrao;
                jogador->posicao.y += (dy / distancia) * forcaEmpurrao;
            }
        }
        
        bossAtual = bossAtual->proximo;
    }
}

// ===== SISTEMA DE ITENS E INTERAÇÃO =====

// Função para criar um item coletável
void criarItem(Item *item, TipoItem tipo, Vector2 posicao) {
    item->tipo = tipo;
    item->posicao = posicao;
    item->raio = 30.0f;  // Raio de coleta de 30 pixels
    item->ativo = true;
    item->coletado = false;
}

// Função para desenhar um item
void desenharItem(Item *item) {
    if (!item->ativo || item->coletado) {
        return;
    }
    
    Color corItem;
    const char *nomeItem;
    
    switch (item->tipo) {
        case ITEM_CHAVE:
            corItem = GOLD;
            nomeItem = "CHAVE";
            break;
        case ITEM_MAPA:
            corItem = SKYBLUE;
            nomeItem = "MAPA";
            break;
        case ITEM_CURE:
            corItem = GREEN;
            nomeItem = "CURE";
            break;
        default:
            corItem = WHITE;
            nomeItem = "???";
            break;
    }
    
    // Desenhar círculo do item (efeito de "pulsar")
    float pulso = sinf(GetTime() * 3.0f) * 3.0f;
    DrawCircleV(item->posicao, 15.0f + pulso, corItem);
    DrawCircleLines((int)item->posicao.x, (int)item->posicao.y, 15.0f + pulso, BLACK);
    
    // Desenhar nome do item acima
    int larguraTexto = MeasureText(nomeItem, 14);
    DrawText(nomeItem, (int)item->posicao.x - larguraTexto / 2, (int)item->posicao.y - 30, 14, BLACK);
    DrawText(nomeItem, (int)item->posicao.x - larguraTexto / 2 - 1, (int)item->posicao.y - 31, 14, corItem);
}

// Função para verificar coleta de item
bool verificarColetaItem(Item *item, Player *jogador) {
    if (!item->ativo || item->coletado) {
        return false;
    }
    
    // Calcular distância entre jogador e item
    float dx = jogador->posicao.x - item->posicao.x;
    float dy = jogador->posicao.y - item->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    
    // Verificar se está próximo e pressionou E
    if (distancia <= item->raio && IsKeyPressed(KEY_E)) {
        // Coletar item baseado no tipo
        switch (item->tipo) {
            case ITEM_CHAVE:
                jogador->temChave = true;
                printf("CHAVE COLETADA! Va ate a porta.\n");
                break;
            case ITEM_MAPA:
                jogador->temMapa = true;
                printf("MAPA COLETADO! Voce pode acessar o laboratorio.\n");
                break;
            case ITEM_CURE:
                jogador->temCure = true;
                printf("CURE COLETADA! VITORIA!\n");
                break;
        }
        
        item->coletado = true;
        item->ativo = false;
        return true;
    }
    
    // Desenhar prompt "Pressione E" se estiver próximo
    if (distancia <= item->raio) {
        const char *prompt = "Pressione E";
        int largura = MeasureText(prompt, 16);
        DrawText(prompt, (int)item->posicao.x - largura / 2, (int)item->posicao.y + 25, 16, YELLOW);
    }
    
    return false;
}

// Função para criar uma porta
void criarPorta(Porta *porta, Vector2 posicao, int faseDestino) {
    porta->posicao = posicao;
    porta->largura = 60.0f;
    porta->altura = 80.0f;
    porta->ativa = true;
    porta->trancada = true;  // Por padrão, portas são trancadas
    porta->faseDestino = faseDestino;
}

// Função para desenhar uma porta
void desenharPorta(Porta *porta) {
    if (!porta->ativa) {
        return;
    }
    
    // Cor baseada no estado da porta
    Color corPorta = porta->trancada ? DARKBROWN : BROWN;
    
    // Desenhar retângulo da porta
    Rectangle rectPorta = {
        porta->posicao.x - porta->largura / 2,
        porta->posicao.y - porta->altura / 2,
        porta->largura,
        porta->altura
    };
    
    DrawRectangleRec(rectPorta, corPorta);
    DrawRectangleLinesEx(rectPorta, 3, BLACK);
    
    // Desenhar símbolo de cadeado se trancada
    if (porta->trancada) {
        DrawCircle((int)porta->posicao.x, (int)porta->posicao.y - 10, 8, GOLD);
        DrawRectangle((int)porta->posicao.x - 6, (int)porta->posicao.y - 10, 12, 15, GOLD);
    }
    
    // Desenhar texto indicando fase destino
    const char *texto = TextFormat("Fase %d", porta->faseDestino);
    int largura = MeasureText(texto, 12);
    DrawText(texto, (int)porta->posicao.x - largura / 2, (int)porta->posicao.y + 45, 12, WHITE);
}

// Função para verificar interação com porta
bool verificarInteracaoPorta(Porta *porta, Player *jogador) {
    if (!porta->ativa) {
        return false;
    }
    
    // Calcular distância entre jogador e porta
    float dx = jogador->posicao.x - porta->posicao.x;
    float dy = jogador->posicao.y - porta->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    
    // Verificar se está próximo (50 pixels)
    if (distancia <= 50.0f) {
        // Verificar se a porta está trancada
        if (porta->trancada) {
            // Verificar se tem a chave necessária
            if (porta->faseDestino == 2 && !jogador->temChave) {
                DrawText("Precisa da CHAVE", (int)porta->posicao.x - 60, (int)porta->posicao.y - 50, 14, RED);
                return false;
            }
            
            if (porta->faseDestino == 3 && !jogador->temMapa) {
                DrawText("Precisa do MAPA", (int)porta->posicao.x - 60, (int)porta->posicao.y - 50, 14, RED);
                return false;
            }
        }
        
        // Mostrar prompt de interação
        DrawText("Pressione E", (int)porta->posicao.x - 45, (int)porta->posicao.y - 50, 16, YELLOW);
        
        // Verificar se pressionou E
        if (IsKeyPressed(KEY_E)) {
            if (porta->trancada) {
                // Destrancar porta
                porta->trancada = false;
                printf("Porta destrancada!\n");
            }
            
            // Usar a porta (transição de fase)
            return true;
        }
    }
    
    return false;
}
