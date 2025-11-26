
#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"
#include "recursos.h"
#include "pathfinding.h"

#include <stdlib.h> 
#include <stdio.h>
#include <math.h>   
#include <stdint.h> 

int verificarColisaoCirculos(Vector2 pos1, float raio1, Vector2 pos2, float raio2) {
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    float distanciaAoQuadrado = dx * dx + dy * dy;
    float raiosCombinados = raio1 + raio2;
    return distanciaAoQuadrado < (raiosCombinados * raiosCombinados);
}

void adicionarBala(Bala **cabeca, Vector2 posInicial, Vector2 alvo, int tipo, float dano) {
    

    Bala *novaBala = (Bala *)malloc(sizeof(Bala));

    if (novaBala == NULL) {
        printf("ERRO: Falha na alocacao de memoria para nova Bala!\n");
        return;
    }

    novaBala->posicao = posInicial;
    novaBala->tipo = tipo;
    novaBala->dano = dano;
    novaBala->raio = (tipo == 0) ? 3.0f : 5.0f;  
    novaBala->tempoVida = 0.0f;  

    Vector2 direcao = {alvo.x - posInicial.x, alvo.y - posInicial.y};

    float comprimento = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
    if (comprimento > 0) {
        float velocidadeBase = (tipo == 0) ? 500.0f : 300.0f; 
        novaBala->velocidade.x = (direcao.x / comprimento) * velocidadeBase;
        novaBala->velocidade.y = (direcao.y / comprimento) * velocidadeBase;

        novaBala->angulo = atan2f(direcao.y, direcao.x) * RAD2DEG + 90.0f;
    } else {
        novaBala->velocidade.x = 0;
        novaBala->velocidade.y = 0;
        novaBala->angulo = 0;
    }

    novaBala->proximo = *cabeca;
    *cabeca = novaBala;
}

void atualizarBalas(Bala **cabeca, Mapa *mapa) {
    if (cabeca == NULL || *cabeca == NULL) return;
    
    Bala *atual = *cabeca;
    Bala *anterior = NULL;
    float deltaTime = GetFrameTime();
    
    while (atual != NULL) {

        atual->posicao.x += atual->velocidade.x * deltaTime;
        atual->posicao.y += atual->velocidade.y * deltaTime;
        

        atual->tempoVida += deltaTime;
        

        float tempoMaximo = (atual->tipo == 0) ? 2.0f : 3.0f;
        

        int colidiuMapa = (mapa != NULL) ? verificarColisaoMapa(mapa, atual->posicao, atual->raio) : 0;
        

        if (atual->posicao.x < 0 || atual->posicao.x > 1024 ||
            atual->posicao.y < 0 || atual->posicao.y > 768 ||
            atual->tempoVida >= tempoMaximo || colidiuMapa) {
            

            Bala *temp = atual;
            Bala *proximaBala = atual->proximo;
            
            if (anterior == NULL) {
                *cabeca = proximaBala;
            } else {
                anterior->proximo = proximaBala;
            }
            
            free(temp); 
            atual = proximaBala;
        } else {
            anterior = atual;
            atual = atual->proximo;
        }
    }
}

void inicializarArma(Arma *arma, TipoArma tipo) {
    arma->tipo = tipo;
    arma->cooldown = 0.0f;
    
    switch(tipo) {
        case ARMA_PISTOL:
            arma->dano = 10;
            arma->taxaTiroMS = 400;
            arma->penteMax = 12;
            arma->penteAtual = 12;
            arma->municaoTotal = 60;  
            arma->tempoRecarga = 2.0f;
            break;
            
        case ARMA_SHOTGUN:
            arma->dano = 35;
            arma->taxaTiroMS = 1000;
            arma->penteMax = 6;
            arma->penteAtual = 6;
            arma->municaoTotal = 24;  
            arma->tempoRecarga = 3.0f;
            break;
            
        case ARMA_SMG:
            arma->dano = 8;
            arma->taxaTiroMS = 100;
            arma->penteMax = 30;
            arma->penteAtual = 30;
            arma->municaoTotal = 120;  
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

void equiparArma(Player *jogador, int slot) {

    if (slot < 0 || slot > 2) return;
    

    if (jogador->slots[slot].tipo == ARMA_NENHUMA) return;
    

    jogador->slotAtivo = slot;
}

void recarregarArma(Player *jogador) {
    if (jogador->estaRecarregando) return;
    
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];
    

    if (armaAtual->penteAtual >= armaAtual->penteMax) return;
    if (armaAtual->municaoTotal <= 0) return;
    

    jogador->estaRecarregando = true;
    jogador->tempoRecargaAtual = armaAtual->tempoRecarga;
}

void atirarArma(Player *jogador, Bala **balas, Vector2 alvo) {
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];
    

    if (armaAtual->penteAtual <= 0) return;
    if (armaAtual->cooldown > 0.0f) return;
    if (jogador->estaRecarregando) return;
    

    adicionarBala(balas, jogador->posicao, alvo, 0, (float)armaAtual->dano);
    

    armaAtual->penteAtual--;
    

    armaAtual->cooldown = armaAtual->taxaTiroMS / 1000.0f;
}

void iniciarJogo(Player *jogador) {

    jogador->posicao = (Vector2){512, 384};
    jogador->vida = 100;
    jogador->tempoTotal = 0.0f;
    jogador->fase = 1;  
    jogador->velocidadeBase = 3.0f;  
    jogador->direcaoVertical = 0;
    jogador->direcaoHorizontal = 1;
    jogador->estaRecarregando = false;
    jogador->tempoRecargaAtual = 0.0f;
    jogador->tempoJaSalvo = false;
    jogador->estadoJogo = ESTADO_JOGANDO;

    jogador->timerBoss = 0.0f;
    jogador->bossSpawnado = false;

    jogador->temChave = false;
    jogador->temMapa = false;
    jogador->temCure = false;
    jogador->jogoVencido = false;
    

    jogador->hordaAtual = 0;
    jogador->estadoHorda = HORDA_NAO_INICIADA;
    jogador->zumbisRestantes = 0;
    jogador->zumbisTotaisHorda = 0;
    jogador->zumbisSpawnados = 0;
    jogador->bossesTotaisHorda = 0;
    jogador->bossesSpawnados = 0;
    jogador->tempoIntervalo = 0.0f;
    jogador->tempoSpawn = 0.0f;
    jogador->tempoSpawnBoss = 0.0f;

    jogador->cooldownDanoBala = 0.0f;
    jogador->cooldownDanoZumbi = 0.0f;

    inicializarArma(&jogador->slots[0], ARMA_PISTOL);   
    inicializarArma(&jogador->slots[1], ARMA_NENHUMA);  
    inicializarArma(&jogador->slots[2], ARMA_NENHUMA);  
    jogador->slotAtivo = 0;  
}

void atualizarJogoComPathfinding(Player *jogador, Zumbi **zumbis, Bala **balas, const Mapa *mapa, PathfindingGrid *grid) {
    float deltaTime = GetFrameTime();

    if (jogador->vida <= 0 || jogador->jogoVencido) {
        return; 
    }

    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];

    if (armaAtual->cooldown > 0.0f) {
        armaAtual->cooldown -= deltaTime;
    }

    if (jogador->estaRecarregando) {
        jogador->tempoRecargaAtual -= deltaTime;

        if (jogador->tempoRecargaAtual <= 0.0f) {

            int municaoNecessaria = armaAtual->penteMax - armaAtual->penteAtual;
            if (armaAtual->municaoTotal >= municaoNecessaria) {
                armaAtual->penteAtual = armaAtual->penteMax;
                armaAtual->municaoTotal -= municaoNecessaria;
            } else {
                armaAtual->penteAtual += armaAtual->municaoTotal;
                armaAtual->municaoTotal = 0;
            }
            jogador->estaRecarregando = false;
            jogador->tempoRecargaAtual = 0.0f;
        }
    }

    if (IsKeyPressed(KEY_ONE)) {
        equiparArma(jogador, 0);
    } else if (IsKeyPressed(KEY_TWO)) {
        equiparArma(jogador, 1);
    } else if (IsKeyPressed(KEY_THREE)) {
        equiparArma(jogador, 2);
    }

    if (IsKeyPressed(KEY_R)) {
        recarregarArma(jogador);
    }

    Vector2 movimento = {0, 0};

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        movimento.y = -1;
        jogador->direcaoVertical = 1;  
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        movimento.y = 1;
        jogador->direcaoVertical = 0;  
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        movimento.x = -1;
        jogador->direcaoHorizontal = 0;  
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        movimento.x = 1;
        jogador->direcaoHorizontal = 1;  
    }

    if (movimento.x != 0 && movimento.y != 0) {
        float norma = 1.0f / sqrtf(2.0f);
        movimento.x *= norma;
        movimento.y *= norma;
    }

    jogador->posicao.x += movimento.x * jogador->velocidadeBase;
    jogador->posicao.y += movimento.y * jogador->velocidadeBase;

    if (jogador->posicao.x < 20) jogador->posicao.x = 20;
    if (jogador->posicao.x > 1004) jogador->posicao.x = 1004;
    if (jogador->posicao.y < 20) jogador->posicao.y = 20;
    if (jogador->posicao.y > 748) jogador->posicao.y = 748;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        atirarArma(jogador, balas, mousePos);
    }

    atualizarZumbisComPathfinding(zumbis, jogador->posicao, deltaTime, mapa, grid);

    verificarColisoesBalaZumbi(balas, zumbis, jogador);
    verificarColisoesBalaJogador(balas, jogador);
    if (zumbis != NULL && *zumbis != NULL) {
        verificarColisoesJogadorZumbi(jogador, *zumbis);
    }

    if (jogador->vida > 0) {
        jogador->tempoTotal += GetFrameTime();
    }
}

void desenharJogo(Player *jogador, Zumbi *zumbis, Bala *balas, Texture2D texturaMapa, Recursos *recursos) {

    desenharZumbis(zumbis);

    if (jogador->spriteAtual.id > 0) {

        float escala = 0.06f; 
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

        DrawCircleV(jogador->posicao, 15, BLUE);
    }

    Bala *balaAtual = balas;
    while (balaAtual != NULL) {

        if (recursos != NULL && texturaValida(recursos->texturaBala)) {

            float escala = 0.020f; 
            float largura = recursos->texturaBala.width * escala;
            float altura = recursos->texturaBala.height * escala;

            Rectangle origem = {0, 0, (float)recursos->texturaBala.width, (float)recursos->texturaBala.height};
            Rectangle destino = {
                balaAtual->posicao.x,
                balaAtual->posicao.y,
                largura,
                altura
            };

            Vector2 pivo = {largura / 2, altura / 2}; 

            DrawTexturePro(recursos->texturaBala, origem, destino, pivo, balaAtual->angulo, WHITE);
        } else {

            DrawCircleV(balaAtual->posicao, 5, YELLOW);
        }
        balaAtual = balaAtual->proximo;
    }

    
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];

    int hudEsquerdoX = 10;
    int linhaY = 10;
    int espacamentoLinha = 30;

    Color corVida = GREEN;
    if (jogador->vida <= 30) {
        corVida = RED;
    } else if (jogador->vida <= 60) {
        corVida = ORANGE;
    }

    DrawRectangle(hudEsquerdoX, linhaY, 200, 20, DARKGRAY);  
    DrawRectangle(hudEsquerdoX, linhaY, (int)(200 * (jogador->vida / 100.0f)), 20, corVida);  
    DrawRectangleLines(hudEsquerdoX, linhaY, 200, 20, WHITE);  
    DrawText(TextFormat("Vida: %d/100", jogador->vida), hudEsquerdoX + 5, linhaY + 2, 18, WHITE);
    linhaY += espacamentoLinha;

    Color corMunicao = WHITE;
    if (armaAtual->penteAtual <= 5) {
        corMunicao = RED;

        if ((int)(GetTime() * 2) % 2 == 0 && !jogador->estaRecarregando) {
            const char *avisoRecarga = "RECARREGUE!";
            int larguraTexto = MeasureText(avisoRecarga, 20);
            DrawText(avisoRecarga, 512 - larguraTexto / 2, 120, 20, RED);  
        }
    }

    if (jogador->estaRecarregando) {
        DrawText("RECARREGANDO...", hudEsquerdoX, linhaY, 20, YELLOW);
        linhaY += espacamentoLinha;

        float progresso = 1.0f - (jogador->tempoRecargaAtual / armaAtual->tempoRecarga);
        DrawRectangle(hudEsquerdoX, linhaY, 200, 10, DARKGRAY);
        DrawRectangle(hudEsquerdoX, linhaY, (int)(200 * progresso), 10, YELLOW);
        DrawRectangleLines(hudEsquerdoX, linhaY, 200, 10, WHITE);
        linhaY += 20;  
    } else {
        DrawText(TextFormat("Municao: [%d] / %d", armaAtual->penteAtual, armaAtual->municaoTotal), hudEsquerdoX, linhaY, 20, corMunicao);
        linhaY += espacamentoLinha;
    }

    int minutos = (int)jogador->tempoTotal / 60;
    float segundos = fmod(jogador->tempoTotal, 60.0f);
    DrawText(TextFormat("Tempo: %02d:%05.2f", minutos, segundos), hudEsquerdoX, linhaY, 20, GOLD);
    linhaY += espacamentoLinha;

    DrawText(TextFormat("Fase: %d/3", jogador->fase), hudEsquerdoX, linhaY, 20, WHITE);
    

    int hudX = 800;  
    int hudY = 690;  

    DrawText("WASD - Mover | 1,2,3 - Armas", hudX, 640, 15, LIGHTGRAY);
    DrawText("R - Recarregar | Click - Atirar", hudX, 660, 15, LIGHTGRAY);
    int slotWidth = 60;
    int slotHeight = 60;
    int slotSpacing = 10;

    const char* nomesArmas[] = {"Vazio", "Pistol", "Shotgun", "SMG"};

    for (int i = 0; i < 3; i++) {
        int posX = hudX + (i * (slotWidth + slotSpacing));
        Color corSlot = DARKGRAY;
        Color corTexto = LIGHTGRAY;

        if (i == jogador->slotAtivo) {
            corSlot = GREEN;
            corTexto = WHITE;
            DrawRectangle(posX - 2, hudY - 2, slotWidth + 4, slotHeight + 4, LIME);
        }

        DrawRectangle(posX, hudY, slotWidth, slotHeight, corSlot);
        DrawRectangleLines(posX, hudY, slotWidth, slotHeight, WHITE);

        DrawText(TextFormat("%d", i + 1), posX + 5, hudY + 5, 20, corTexto);

        if (jogador->slots[i].tipo != ARMA_NENHUMA) {
            const char* nomeArma = nomesArmas[jogador->slots[i].tipo];
            DrawText(nomeArma, posX + 5, hudY + 30, 15, corTexto);
        } else {
            DrawText("---", posX + 15, hudY + 30, 15, GRAY);
        }
    }
    

    if (jogador->jogoVencido) {

        if (!jogador->tempoJaSalvo) {
            checkAndSaveTime(jogador->tempoTotal);
            jogador->tempoJaSalvo = true;
        }
        

        DrawRectangle(0, 0, 1024, 768, BLACK);

        DrawText("VITORIA!", 380, 80, 60, GREEN);
        

        int minutos = (int)jogador->tempoTotal / 60;
        float segundos = fmod(jogador->tempoTotal, 60.0f);
        DrawText(TextFormat("Seu tempo: %02d:%05.2f", minutos, segundos), 350, 170, 30, WHITE);

        float tempos[MAX_SCORES];
        loadTimes(tempos, MAX_SCORES);

        DrawText("=== PODIO ===", 400, 230, 30, GOLD);
        
        int posY = 290;
        for (int i = 0; i < MAX_SCORES && tempos[i] < 999999.0f; i++) {
            int min = (int)tempos[i] / 60;
            float seg = fmod(tempos[i], 60.0f);

            Color cor = (fabs(tempos[i] - jogador->tempoTotal) < 0.01f) ? YELLOW : WHITE;

            DrawText(TextFormat("%dº - %02d:%05.2f", i + 1, min, seg), 400, posY, 24, cor);
            posY += 35;
        }

        DrawText("Pressione R para tentar novamente", 320, 630, 22, GREEN);
        DrawText("Pressione ESC para sair", 380, 670, 20, LIGHTGRAY);
        return; 
    }
    

    if (jogador->vida <= 0) {

        DrawRectangle(0, 0, 1024, 768, BLACK);

        DrawText("GAME OVER", 330, 250, 60, RED);

        int minutos = (int)jogador->tempoTotal / 60;
        float segundos = fmod(jogador->tempoTotal, 60.0f);
        DrawText(TextFormat("Tempo de sobrevivencia:", minutos, segundos), 300, 350, 26, WHITE);
        DrawText(TextFormat("%02d:%05.2f", minutos, segundos), 400, 390, 40, YELLOW);

        DrawText("Pressione R para tentar novamente", 320, 500, 22, GREEN);
        DrawText("Pressione ESC para sair", 260, 430, 20, LIGHTGRAY);
        return; 
    }
}

void adicionarZumbi(Zumbi **cabeca, Vector2 posInicial, Texture2D sprites[][4]) {

    Zumbi *novoZumbi = (Zumbi *)malloc(sizeof(Zumbi));

    if (novoZumbi == NULL) {
        printf("ERRO: Falha na alocacao de memoria para novo Zumbi!\n");
        return;
    }

    novoZumbi->posicao = posInicial;
    novoZumbi->posicaoAnterior = posInicial;  
    novoZumbi->velocidade = (Vector2){0, 0};
    novoZumbi->vida = 20; 
    novoZumbi->raio = 20.0f;

    novoZumbi->tipoMovimento = GetRandomValue(0, 3);

    novoZumbi->velocidadeBase = 120.0f + (float)GetRandomValue(0, 20); 

    novoZumbi->tempoDesvio = 0.0f;
    novoZumbi->anguloDesvio = (float)GetRandomValue(0, 360) * DEG2RAD;

    novoZumbi->tipoSkin = GetRandomValue(0, 4);

    novoZumbi->spriteFrenteDireita = sprites[novoZumbi->tipoSkin][0];
    novoZumbi->spriteFrenteEsquerda = sprites[novoZumbi->tipoSkin][1];
    novoZumbi->spriteCostasDireita = sprites[novoZumbi->tipoSkin][2];
    novoZumbi->spriteCostasEsquerda = sprites[novoZumbi->tipoSkin][3];

    novoZumbi->direcaoVertical = 0;
    novoZumbi->direcaoHorizontal = 1;
    novoZumbi->spriteAtual = novoZumbi->spriteFrenteDireita;

    novoZumbi->caminho.valido = false;
    novoZumbi->caminho.tamanho = 0;
    novoZumbi->caminho.indiceAtual = 0;
    novoZumbi->caminho.tempoRecalculo = 0.0f;

    novoZumbi->proximo = *cabeca;
    *cabeca = novoZumbi;
}

void atualizarZumbis(Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime) {
    Zumbi *atual = *cabeca;
    Zumbi *anterior = NULL;

    while (atual != NULL) {

        atual->posicaoAnterior = atual->posicao;

        Vector2 direcao = {
            posicaoJogador.x - atual->posicao.x,
            posicaoJogador.y - atual->posicao.y
        };
        

        float magnitude = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
        if (magnitude > 0) {
            direcao.x /= magnitude;
            direcao.y /= magnitude;
        }
        

        Vector2 direcaoFinal = direcao;
        float velocidadeFinal = atual->velocidadeBase;
        
        switch (atual->tipoMovimento) {
            case 0: 

                break;
                
            case 1: 
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
                
            case 2: 
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
                
            case 3: 
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
        

        atual->velocidade.x = direcaoFinal.x * velocidadeFinal;
        atual->velocidade.y = direcaoFinal.y * velocidadeFinal;

        if (atual->velocidade.y < -5.0f) {
            atual->direcaoVertical = 1; 
        } else if (atual->velocidade.y > 5.0f) {
            atual->direcaoVertical = 0; 
        }

        if (atual->velocidade.x < -5.0f) {
            atual->direcaoHorizontal = 0; 
        } else if (atual->velocidade.x > 5.0f) {
            atual->direcaoHorizontal = 1; 
        }

        if (atual->direcaoVertical == 0) { 
            if (atual->direcaoHorizontal == 0) {
                atual->spriteAtual = atual->spriteFrenteEsquerda;
            } else {
                atual->spriteAtual = atual->spriteFrenteDireita;
            }
        } else { 
            if (atual->direcaoHorizontal == 0) {
                atual->spriteAtual = atual->spriteCostasEsquerda;
            } else {
                atual->spriteAtual = atual->spriteCostasDireita;
            }
        }

        atual->posicao.x += atual->velocidade.x * deltaTime;
        atual->posicao.y += atual->velocidade.y * deltaTime;

        if (atual->vida <= 0) {

            Zumbi *removido = atual;
            if (anterior == NULL) {
                *cabeca = atual->proximo;
            } else {
                anterior->proximo = atual->proximo;
            }
            atual = atual->proximo;
            free(removido); 
        } else {
            anterior = atual;
            atual = atual->proximo;
        }
    }
}

void atualizarZumbisComPathfinding(Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime, const Mapa *mapa, PathfindingGrid *grid) {
    Zumbi *atual = *cabeca;
    Zumbi *anterior = NULL;

    while (atual != NULL) {

        atual->posicaoAnterior = atual->posicao;

        float dx = posicaoJogador.x - atual->posicao.x;
        float dy = posicaoJogador.y - atual->posicao.y;
        float distancia = sqrtf(dx * dx + dy * dy);

        Vector2 direcaoFinal = {0.0f, 0.0f};
        float velocidadeFinal = atual->velocidadeBase;

        bool usarPathfinding = (mapa != NULL && grid != NULL);

        if (distancia < TAMANHO_TILE * 1.5f) {
            usarPathfinding = false;
        }

        if (usarPathfinding) {

            if (precisaRecalcularCaminho(&atual->caminho, posicaoJogador, deltaTime)) {

                Vector2 alvoVariado = posicaoJogador;

                uintptr_t zumbiId = (uintptr_t)atual;
                float hashAngulo = (float)((zumbiId / 137) % 360); 

                float velocidadeRotacao = 0.3f + (float)((zumbiId % 5) * 0.2f); 
                float anguloTemporal = atual->tempoDesvio * velocidadeRotacao;

                float variacaoAngulo = hashAngulo + (float)(atual->tipoMovimento * 45) + anguloTemporal;
                float raioVariacao = 24.0f + (float)((zumbiId % 4) * 12); 

                alvoVariado.x += cosf(variacaoAngulo) * raioVariacao;
                alvoVariado.y += sinf(variacaoAngulo) * raioVariacao;

                if (alvoVariado.x < 32) alvoVariado.x = 32;
                if (alvoVariado.x > 992) alvoVariado.x = 992;
                if (alvoVariado.y < 32) alvoVariado.y = 32;
                if (alvoVariado.y > 736) alvoVariado.y = 736;

                calcularCaminho(grid, mapa, atual->posicao, alvoVariado, &atual->caminho);
            }

            if (atual->caminho.valido && atual->caminho.tamanho > 0) {

                atualizarSeguimentoCaminho(&atual->caminho, atual->posicao, TAMANHO_TILE * 0.4f);

                direcaoFinal = obterDirecaoCaminho(&atual->caminho, atual->posicao);
            } else {

                if (distancia > 0) {
                    direcaoFinal.x = dx / distancia;
                    direcaoFinal.y = dy / distancia;
                }
            }
        } else {

            if (distancia > 0) {
                direcaoFinal.x = dx / distancia;
                direcaoFinal.y = dy / distancia;
            }
        }

        switch (atual->tipoMovimento % 4) {
            case 1: 
                atual->tempoDesvio += deltaTime * 2.0f;
                {
                    float desvio = sinf(atual->tempoDesvio) * 0.2f;
                    direcaoFinal.x += -direcaoFinal.y * desvio;
                    direcaoFinal.y += direcaoFinal.x * desvio;
                }
                break;
            case 2: 
                velocidadeFinal *= 1.1f;
                break;
            case 3: 
                velocidadeFinal *= 0.9f;
                break;
            default:

                break;
        }

        float mag = sqrtf(direcaoFinal.x * direcaoFinal.x + direcaoFinal.y * direcaoFinal.y);
        if (mag > 0) {
            direcaoFinal.x /= mag;
            direcaoFinal.y /= mag;
        }

        atual->velocidade.x = direcaoFinal.x * velocidadeFinal;
        atual->velocidade.y = direcaoFinal.y * velocidadeFinal;

        if (atual->velocidade.y < -5.0f) {
            atual->direcaoVertical = 1; 
        } else if (atual->velocidade.y > 5.0f) {
            atual->direcaoVertical = 0; 
        }

        if (atual->velocidade.x < -5.0f) {
            atual->direcaoHorizontal = 0; 
        } else if (atual->velocidade.x > 5.0f) {
            atual->direcaoHorizontal = 1; 
        }

        if (atual->direcaoVertical == 0) { 
            if (atual->direcaoHorizontal == 0) {
                atual->spriteAtual = atual->spriteFrenteEsquerda;
            } else {
                atual->spriteAtual = atual->spriteFrenteDireita;
            }
        } else { 
            if (atual->direcaoHorizontal == 0) {
                atual->spriteAtual = atual->spriteCostasEsquerda;
            } else {
                atual->spriteAtual = atual->spriteCostasDireita;
            }
        }

        atual->posicao.x += atual->velocidade.x * deltaTime;
        atual->posicao.y += atual->velocidade.y * deltaTime;

        if (atual->vida <= 0) {

            Zumbi *removido = atual;
            if (anterior == NULL) {
                *cabeca = atual->proximo;
            } else {
                anterior->proximo = atual->proximo;
            }
            atual = atual->proximo;
            free(removido);
        } else {
            anterior = atual;
            atual = atual->proximo;
        }
    }
}

void desenharZumbis(Zumbi *cabeca) {
    Zumbi *atual = cabeca;

    while (atual != NULL) {

        if (atual->spriteAtual.id > 0) {

            float escala = 0.07f; 
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

            DrawCircleV(atual->posicao, atual->raio, GREEN);
        }

        float barraLargura = 40.0f;
        float porcentagemVida = atual->vida / 20.0f; 
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

void liberarZumbis(Zumbi **cabeca) {
    Zumbi *atual = *cabeca;

    while (atual != NULL) {
        Zumbi *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }

    *cabeca = NULL;
}

void verificarColisoesBalaZumbi(Bala **balas, Zumbi **zumbis, Player *jogador) {
    if (balas == NULL || *balas == NULL || zumbis == NULL || *zumbis == NULL) return;
    
    Bala *balaAtual = *balas;
    Bala *balaAnterior = NULL;

    while (balaAtual != NULL) {

        if (balaAtual->tipo != 0) {
            balaAnterior = balaAtual;
            balaAtual = balaAtual->proximo;
            continue;
        }
        
        Zumbi *zumbiAtual = *zumbis;
        int balaRemovida = 0;

        while (zumbiAtual != NULL && !balaRemovida) {

            if (verificarColisaoCirculos(balaAtual->posicao, balaAtual->raio, zumbiAtual->posicao, zumbiAtual->raio)) {

                zumbiAtual->vida -= (int)balaAtual->dano;

                Bala *balaRemover = balaAtual;
                Bala *proximaBala = balaAtual->proximo;
                
                if (balaAnterior == NULL) {
                    *balas = proximaBala;
                } else {
                    balaAnterior->proximo = proximaBala;
                }
                
                free(balaRemover);
                balaAtual = proximaBala;
                balaRemovida = 1;
            } else {
                zumbiAtual = zumbiAtual->proximo;
            }
        }

        if (!balaRemovida) {
            balaAnterior = balaAtual;
            balaAtual = balaAtual->proximo;
        }
    }
}

void verificarColisoesBalaJogador(Bala **balas, Player *jogador) {
    if (balas == NULL || *balas == NULL || jogador == NULL) return;

    jogador->cooldownDanoBala -= GetFrameTime();
    
    Bala *balaAtual = *balas;
    Bala *balaAnterior = NULL;
    
    while (balaAtual != NULL) {

        if (balaAtual->tipo == 1) {

            if (verificarColisaoCirculos(balaAtual->posicao, balaAtual->raio, jogador->posicao, 15.0f)) {

                if (jogador->cooldownDanoBala <= 0.0f) {
                    int dano = (int)balaAtual->dano;
                    jogador->vida -= dano;
                    jogador->cooldownDanoBala = 0.2f; 
                    
                    printf("OUCH! Jogador recebeu %d de dano de projetil do boss. Vida: %d\n", dano, jogador->vida);
                    

                    if (jogador->vida < 0) {
                        jogador->vida = 0;
                    }
                }
                

                Bala *balaRemover = balaAtual;
                Bala *proximaBala = balaAtual->proximo;
                
                if (balaAnterior == NULL) {
                    *balas = proximaBala;
                } else {
                    balaAnterior->proximo = proximaBala;
                }
                
                free(balaRemover);
                balaAtual = proximaBala;
                continue;
            }
        }
        
        balaAnterior = balaAtual;
        balaAtual = balaAtual->proximo;
    }
}

void verificarColisoesJogadorZumbi(Player *jogador, Zumbi *zumbis) {
    if (jogador == NULL || zumbis == NULL) return;

    jogador->cooldownDanoZumbi -= GetFrameTime();

    Zumbi *zumbiAtual = zumbis;
    const float raioJogador = 15.0f;

    while (zumbiAtual != NULL) {

        if (verificarColisaoCirculos(jogador->posicao, raioJogador, zumbiAtual->posicao, zumbiAtual->raio)) {

            float dx = zumbiAtual->posicao.x - jogador->posicao.x;
            float dy = zumbiAtual->posicao.y - jogador->posicao.y;
            float distancia = sqrtf(dx * dx + dy * dy);

            if (distancia > 0.1f) {

                float sobreposicao = (raioJogador + zumbiAtual->raio) - distancia;

                if (sobreposicao > 0) {

                    float dirX = dx / distancia;
                    float dirY = dy / distancia;

                    zumbiAtual->posicao.x += dirX * (sobreposicao + 2.0f);
                    zumbiAtual->posicao.y += dirY * (sobreposicao + 2.0f);
                }
            } else {

                float angulo = (float)GetRandomValue(0, 360) * DEG2RAD;
                zumbiAtual->posicao.x += cosf(angulo) * (raioJogador + zumbiAtual->raio + 5.0f);
                zumbiAtual->posicao.y += sinf(angulo) * (raioJogador + zumbiAtual->raio + 5.0f);
            }

            if (jogador->cooldownDanoZumbi <= 0.0f) {
                int dano = 5; 
                jogador->vida -= dano;
                jogador->cooldownDanoZumbi = 0.5f; 

                printf("OUCH! Jogador recebeu %d de dano. Vida: %d\n", dano, jogador->vida);
            }

            if (jogador->vida < 0) {
                jogador->vida = 0;
            }
        }

        zumbiAtual = zumbiAtual->proximo;
    }
}

void criarBoss(Boss **bosses, TipoBoss tipo, Vector2 posicao, Texture2D spriteFrente, Texture2D spriteCostas, Texture2D spriteDireita, Texture2D spriteEsquerda) {
    Boss *novoBoss = (Boss *)malloc(sizeof(Boss));

    if (novoBoss == NULL) {
        printf("ERRO: Falha na alocacao de memoria para novo Boss!\n");
        return;
    }

    novoBoss->tipo = tipo;
    novoBoss->posicao = posicao;
    novoBoss->posicaoAnterior = posicao;  
    novoBoss->ativo = true;
    novoBoss->atacando = false;
    novoBoss->tempoAtaque = 0.0f;
    novoBoss->padraoAtaque = 0;
    novoBoss->anguloRotacao = 0.0f;
    novoBoss->direcaoVertical = 0;    
    novoBoss->direcaoHorizontal = 1;  

    novoBoss->spriteFrente = spriteFrente;
    novoBoss->spriteCostas = spriteCostas;
    novoBoss->spriteDireita = spriteDireita;
    novoBoss->spriteEsquerda = spriteEsquerda;
    novoBoss->spriteAtual = spriteFrente;  

    switch (tipo) {
        case BOSS_PROWLER:
            novoBoss->vidaMax = 150;
            novoBoss->vida = 150;
            novoBoss->velocidade = 2.5f;
            novoBoss->raio = 25.0f;
            novoBoss->cooldownAtaque = 3.0f; 
            break;
            
        case BOSS_HUNTER:
            novoBoss->vidaMax = 80;
            novoBoss->vida = 80;
            novoBoss->velocidade = 3.0f;
            novoBoss->raio = 20.0f;
            novoBoss->cooldownAtaque = 0.0f; 
            break;
            
        case BOSS_ABOMINATION:
            novoBoss->vidaMax = 250;
            novoBoss->vida = 250;
            novoBoss->velocidade = 0.0f; 
            novoBoss->raio = 60.0f;
            novoBoss->cooldownAtaque = 0.4f; 
            break;
            
        default:
            novoBoss->vidaMax = 100;
            novoBoss->vida = 100;
            novoBoss->velocidade = 2.0f;
            novoBoss->raio = 20.0f;
            novoBoss->cooldownAtaque = 2.0f;
            break;
    }

    novoBoss->caminho.valido = false;
    novoBoss->caminho.tamanho = 0;
    novoBoss->caminho.indiceAtual = 0;
    novoBoss->caminho.tempoRecalculo = 0.0f;

    novoBoss->proximo = *bosses;
    *bosses = novoBoss;
}

void atualizarBossComPathfinding(Boss **bosses, Player *jogador, Bala **balas, float deltaTime, const Mapa *mapa, PathfindingGrid *grid) {
    Boss *bossAtual = *bosses;

    while (bossAtual != NULL) {
        if (!bossAtual->ativo) {
            bossAtual = bossAtual->proximo;
            continue;
        }

        bossAtual->posicaoAnterior = bossAtual->posicao;

        bossAtual->tempoAtaque += deltaTime;

        switch (bossAtual->tipo) {
            case BOSS_PROWLER: {

                float dx = jogador->posicao.x - bossAtual->posicao.x;
                float dy = jogador->posicao.y - bossAtual->posicao.y;
                float distancia = sqrtf(dx * dx + dy * dy);

                bool usarPathfinding = (mapa != NULL && grid != NULL && distancia > TAMANHO_TILE * 2.0f);

                if (usarPathfinding) {

                    if (precisaRecalcularCaminho(&bossAtual->caminho, jogador->posicao, deltaTime)) {
                        calcularCaminho(grid, mapa, bossAtual->posicao, jogador->posicao, &bossAtual->caminho);
                    }

                    if (bossAtual->caminho.valido && bossAtual->caminho.tamanho > 0) {
                        atualizarSeguimentoCaminho(&bossAtual->caminho, bossAtual->posicao, TAMANHO_TILE * 0.5f);
                        Vector2 direcao = obterDirecaoCaminho(&bossAtual->caminho, bossAtual->posicao);

                        if (distancia > 0) {
                            bossAtual->posicao.x += direcao.x * bossAtual->velocidade;
                            bossAtual->posicao.y += direcao.y * bossAtual->velocidade;
                        }
                    } else {

                        if (distancia > 0) {
                            bossAtual->posicao.x += (dx / distancia) * bossAtual->velocidade;
                            bossAtual->posicao.y += (dy / distancia) * bossAtual->velocidade;
                        }
                    }
                } else {

                    if (distancia > 0) {
                        bossAtual->posicao.x += (dx / distancia) * bossAtual->velocidade;
                        bossAtual->posicao.y += (dy / distancia) * bossAtual->velocidade;
                    }
                }

                float absDx = fabsf(dx);
                float absDy = fabsf(dy);

                if (absDx > absDy) {
                    if (dx > 0) {
                        bossAtual->spriteAtual = bossAtual->spriteDireita;
                    } else {
                        bossAtual->spriteAtual = bossAtual->spriteEsquerda;
                    }
                } else {
                    if (dy > 0) {
                        bossAtual->spriteAtual = bossAtual->spriteFrente;
                    } else {
                        bossAtual->spriteAtual = bossAtual->spriteCostas;
                    }
                }

                if (bossAtual->tempoAtaque >= bossAtual->cooldownAtaque) {
                    bossAtual->atacando = true;

                    if (distancia <= 80.0f) {
                        jogador->vida -= 15; 
                    }

                    bossAtual->tempoAtaque = 0.0f;
                    bossAtual->atacando = false;
                }
                break;
            }

            case BOSS_HUNTER: {

                float dx = jogador->posicao.x - bossAtual->posicao.x;
                float dy = jogador->posicao.y - bossAtual->posicao.y;
                float distancia = sqrtf(dx * dx + dy * dy);

                bool usarPathfinding = (mapa != NULL && grid != NULL && distancia > TAMANHO_TILE * 1.5f);

                if (usarPathfinding) {

                    if (precisaRecalcularCaminho(&bossAtual->caminho, jogador->posicao, deltaTime)) {

                        calcularCaminho(grid, mapa, bossAtual->posicao, jogador->posicao, &bossAtual->caminho);
                    }

                    if (bossAtual->caminho.valido && bossAtual->caminho.tamanho > 0) {
                        atualizarSeguimentoCaminho(&bossAtual->caminho, bossAtual->posicao, TAMANHO_TILE * 0.3f);
                        Vector2 direcao = obterDirecaoCaminho(&bossAtual->caminho, bossAtual->posicao);

                        if (distancia > 0) {
                            bossAtual->posicao.x += direcao.x * bossAtual->velocidade;
                            bossAtual->posicao.y += direcao.y * bossAtual->velocidade;
                        }
                    } else {

                        if (distancia > 0) {
                            bossAtual->posicao.x += (dx / distancia) * bossAtual->velocidade;
                            bossAtual->posicao.y += (dy / distancia) * bossAtual->velocidade;
                        }
                    }
                } else {

                    if (distancia > 0) {
                        bossAtual->posicao.x += (dx / distancia) * bossAtual->velocidade;
                        bossAtual->posicao.y += (dy / distancia) * bossAtual->velocidade;
                    }
                }

                float absDx = fabsf(dx);
                float absDy = fabsf(dy);

                if (absDx > absDy) {
                    if (dx > 0) {
                        bossAtual->spriteAtual = bossAtual->spriteDireita;
                    } else {
                        bossAtual->spriteAtual = bossAtual->spriteEsquerda;
                    }
                } else {
                    if (dy > 0) {
                        bossAtual->spriteAtual = bossAtual->spriteFrente;
                    } else {
                        bossAtual->spriteAtual = bossAtual->spriteCostas;
                    }
                }

                if (distancia <= (bossAtual->raio + 20.0f)) {
                    if (bossAtual->tempoAtaque >= 1.0f) { 
                        jogador->vida -= 20;
                        bossAtual->tempoAtaque = 0.0f;
                    }
                }
                break;
            }

            case BOSS_ABOMINATION: {

                if (bossAtual->tempoAtaque >= bossAtual->cooldownAtaque) {

                    int numProjeteis = 16;
                    float anguloBase = bossAtual->anguloRotacao;

                    for (int i = 0; i < numProjeteis; i++) {
                        float angulo = (anguloBase + (360.0f / numProjeteis) * i) * DEG2RAD;

                        Bala *novaBala = (Bala *)malloc(sizeof(Bala));
                        if (novaBala != NULL) {
                            novaBala->posicao = bossAtual->posicao;
                            novaBala->velocidade.x = cosf(angulo) * 250.0f;
                            novaBala->velocidade.y = sinf(angulo) * 250.0f;
                            novaBala->tipo = 1;
                            novaBala->dano = 20.0f;
                            novaBala->raio = 6.0f;
                            novaBala->tempoVida = 0.0f;
                            novaBala->angulo = angulo * RAD2DEG + 90.0f;
                            novaBala->proximo = *balas;
                            *balas = novaBala;
                        }
                    }

                    bossAtual->anguloRotacao += 25.0f;
                    if (bossAtual->anguloRotacao >= 360.0f) {
                        bossAtual->anguloRotacao -= 360.0f;
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

void desenharBoss(Boss *bosses) {
    Boss *bossAtual = bosses;
    
    while (bossAtual != NULL) {
        if (!bossAtual->ativo) {
            bossAtual = bossAtual->proximo;
            continue;
        }
        

        if (bossAtual->spriteAtual.id > 0) {

            float escala = 0.15f; 

            if (bossAtual->tipo == BOSS_ABOMINATION) {
                escala = 0.30f; 
            }

            float largura = bossAtual->spriteAtual.width * escala;
            float altura = bossAtual->spriteAtual.height * escala;

            Rectangle destino = {
                bossAtual->posicao.x - largura / 2,
                bossAtual->posicao.y - altura / 2,
                largura,
                altura
            };

            Rectangle origem = {0, 0, (float)bossAtual->spriteAtual.width, (float)bossAtual->spriteAtual.height};

            DrawTexturePro(bossAtual->spriteAtual, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
        } else {

            Color corBoss;
            switch (bossAtual->tipo) {
                case BOSS_PROWLER:
                    corBoss = PURPLE; 
                    break;
                case BOSS_HUNTER:
                    corBoss = ORANGE; 
                    break;
                case BOSS_ABOMINATION:
                    corBoss = DARKGREEN; 
                    break;
                default:
                    corBoss = BLACK;
                    break;
            }

            DrawCircleV(bossAtual->posicao, bossAtual->raio, corBoss);
            DrawCircleLines((int)bossAtual->posicao.x, (int)bossAtual->posicao.y, bossAtual->raio, DARKGRAY);
        }
        

        float barraLargura = 60.0f;
        float barraAltura = 8.0f;
        float porcentagemVida = (float)bossAtual->vida / (float)bossAtual->vidaMax;

        float offsetY;
        if (bossAtual->spriteAtual.id > 0) {

            float escala = (bossAtual->tipo == BOSS_ABOMINATION) ? 0.30f : 0.15f;
            offsetY = bossAtual->spriteAtual.height * escala / 2 + 15.0f;
        } else {
            offsetY = bossAtual->raio + 15.0f;
        }

        Vector2 barraPos = {bossAtual->posicao.x - barraLargura / 2, bossAtual->posicao.y - offsetY};
        

        DrawRectangleV(barraPos, (Vector2){barraLargura, barraAltura}, RED);

        DrawRectangleV(barraPos, (Vector2){barraLargura * porcentagemVida, barraAltura}, GREEN);

        DrawRectangleLinesEx((Rectangle){barraPos.x, barraPos.y, barraLargura, barraAltura}, 1, BLACK);
        

        if (bossAtual->tipo == BOSS_PROWLER && bossAtual->atacando) {
            DrawCircleLines((int)bossAtual->posicao.x, (int)bossAtual->posicao.y, 80.0f, RED);
        }
        
        bossAtual = bossAtual->proximo;
    }
}

void verificarColisoesBossBala(Boss **bosses, Bala **balas, Item *itemProgresso, Item *itemArma, Player *jogador) {
    if (bosses == NULL || balas == NULL || *balas == NULL) return;
    
    Boss *bossAtual = *bosses;
    
    while (bossAtual != NULL) {
        if (!bossAtual->ativo) {
            bossAtual = bossAtual->proximo;
            continue;
        }
        
        Bala *balaAtual = *balas;
        Bala *balaAnterior = NULL;
        
        while (balaAtual != NULL) {

            if (balaAtual->tipo == 0) {

                float dx = bossAtual->posicao.x - balaAtual->posicao.x;
                float dy = bossAtual->posicao.y - balaAtual->posicao.y;
                float distancia = sqrtf(dx * dx + dy * dy);
                
                if (distancia <= (bossAtual->raio + balaAtual->raio)) {

                    bossAtual->vida -= (int)balaAtual->dano;
                    

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
        

        if (bossAtual->vida <= 0 && bossAtual->ativo) {
            bossAtual->ativo = false;
            

            Vector2 posicaoItem1 = {bossAtual->posicao.x - 30, bossAtual->posicao.y};
            Vector2 posicaoItem2 = {bossAtual->posicao.x + 30, bossAtual->posicao.y};
            

            switch (bossAtual->tipo) {
                case BOSS_PROWLER:
                    if (itemProgresso != NULL && !itemProgresso->ativo) {
                        criarItem(itemProgresso, ITEM_CHAVE, posicaoItem1);
                        printf("Boss morreu! CHAVE dropada!\n");
                    }
                    if (itemArma != NULL && !itemArma->ativo) {
                        criarItem(itemArma, ITEM_SHOTGUN, posicaoItem2);
                        printf("Boss morreu! SHOTGUN dropada!\n");
                    }
                    break;
                case BOSS_HUNTER:

                    if (jogador != NULL && jogador->fase == 2 && jogador->hordaAtual == 3) {

                        int huntersVivos = 0;
                        Boss *b = *bosses;
                        while (b != NULL) {

                            if (b->tipo == BOSS_HUNTER && b->ativo && b != bossAtual) {
                                huntersVivos++;
                            }
                            b = b->proximo;
                        }
                        

                        if (huntersVivos == 0) {
                            if (itemProgresso != NULL && !itemProgresso->ativo) {
                                criarItem(itemProgresso, ITEM_CHAVE, posicaoItem1);
                                printf("=== ULTIMO HUNTER DA HORDA 3 MORREU! ===\n");
                                printf("CHAVE dropada!\n");
                            }
                            if (itemArma != NULL && !itemArma->ativo) {
                                criarItem(itemArma, ITEM_SMG, posicaoItem2);
                                printf("SMG dropada!\n");
                            }
                        } else {
                            printf("Hunter morreu! Ainda restam %d Hunter(s).\n", huntersVivos);
                        }
                    } else {

                        printf("Hunter morreu (horda %d)! Sem drop.\n", jogador != NULL ? jogador->hordaAtual : 0);
                    }
                    break;
                case BOSS_ABOMINATION:
                    if (itemProgresso != NULL && !itemProgresso->ativo) {
                        criarItem(itemProgresso, ITEM_CURE, bossAtual->posicao);
                        printf("Boss morreu! CURE dropada! VITORIA!\n");
                    }
                    break;
                default:
                    break;
            }
        }
        
        bossAtual = bossAtual->proximo;
    }
}

void verificarColisoesBossJogador(Boss *bosses, Player *jogador) {

    (void)bosses;
    (void)jogador;
}

void criarItem(Item *item, TipoItem tipo, Vector2 posicao) {
    item->tipo = tipo;
    item->posicao = posicao;
    item->raio = 30.0f;  
    item->ativo = true;
    item->coletado = false;
    item->tipoArma = ARMA_NENHUMA; 
    

    if (tipo == ITEM_SHOTGUN) {
        item->tipoArma = ARMA_SHOTGUN;
    } else if (tipo == ITEM_SMG) {
        item->tipoArma = ARMA_SMG;
    }
}

void desenharItem(Item *item, Recursos *recursos) {
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
        case ITEM_SHOTGUN:
            corItem = ORANGE;
            nomeItem = "SHOTGUN";
            break;
        case ITEM_SMG:
            corItem = PURPLE;
            nomeItem = "SMG";
            break;
        default:
            corItem = WHITE;
            nomeItem = "???";
            break;
    }

    float pulso = sinf(GetTime() * 3.0f) * 3.0f;

    bool texturadoDesenhado = false;
    Texture2D texturaItem = {0};
    float escalaBase = 0.05f; 

    if (recursos != NULL) {
        if (item->tipo == ITEM_CHAVE && texturaValida(recursos->texturaChave)) {
            texturaItem = recursos->texturaChave;
            escalaBase = 0.05f;
            texturadoDesenhado = true;
        } else if (item->tipo == ITEM_SHOTGUN && texturaValida(recursos->texturaShotgun)) {
            texturaItem = recursos->texturaShotgun;
            escalaBase = 0.12f; 
            texturadoDesenhado = true;
        } else if (item->tipo == ITEM_SMG && texturaValida(recursos->texturaSMG)) {
            texturaItem = recursos->texturaSMG;
            escalaBase = 0.12f; 
            texturadoDesenhado = true;
        }
    }

    if (texturadoDesenhado) {
        float escala = escalaBase + (pulso * 0.001f); 
        float largura = texturaItem.width * escala;
        float altura = texturaItem.height * escala;

        Rectangle origem = {0, 0, (float)texturaItem.width, (float)texturaItem.height};
        Rectangle destino = {
            item->posicao.x - largura / 2,
            item->posicao.y - altura / 2,
            largura,
            altura
        };
        Vector2 pivo = {0, 0};

        DrawTexturePro(texturaItem, origem, destino, pivo, 0.0f, WHITE);
    } else {

        DrawCircleV(item->posicao, 15.0f + pulso, corItem);
        DrawCircleLines((int)item->posicao.x, (int)item->posicao.y, 15.0f + pulso, BLACK);
    }

    int larguraTexto = MeasureText(nomeItem, 14);
    DrawText(nomeItem, (int)item->posicao.x - larguraTexto / 2, (int)item->posicao.y - 30, 14, BLACK);
    DrawText(nomeItem, (int)item->posicao.x - larguraTexto / 2 - 1, (int)item->posicao.y - 31, 14, corItem);
}

bool verificarColetaItem(Item *item, Player *jogador) {
    if (!item->ativo || item->coletado) {
        return false;
    }
    

    float dx = jogador->posicao.x - item->posicao.x;
    float dy = jogador->posicao.y - item->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    

    if (distancia <= item->raio && IsKeyPressed(KEY_E)) {

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

                if (jogador->fase == 3) {
                    jogador->jogoVencido = true;
                }
                printf("CURE COLETADA! VITORIA!\n");
                break;
            case ITEM_SHOTGUN:
            case ITEM_SMG: {

                int slotVazio = -1;
                for (int i = 0; i < 3; i++) {
                    if (jogador->slots[i].tipo == ARMA_NENHUMA) {
                        slotVazio = i;
                        break;
                    }
                }
                
                if (slotVazio != -1) {

                    inicializarArma(&jogador->slots[slotVazio], item->tipoArma);
                    printf("ARMA COLETADA: %s equipada no Slot %d!\n", 
                           item->tipo == ITEM_SHOTGUN ? "SHOTGUN" : "SMG", 
                           slotVazio + 1);
                    

                    equiparArma(jogador, slotVazio);
                } else {
                    printf("Todos os slots estao ocupados! Nao foi possivel coletar a arma.\n");
                    return false; 
                }
                break;
            }
        }
        
        item->coletado = true;
        item->ativo = false;
        return true;
    }
    

    if (distancia <= item->raio) {
        const char *prompt = "Pressione E";
        int largura = MeasureText(prompt, 16);
        DrawText(prompt, (int)item->posicao.x - largura / 2, (int)item->posicao.y + 25, 16, YELLOW);
    }
    
    return false;
}

void criarPorta(Porta *porta, Vector2 posicao, int faseDestino) {
    porta->posicao = posicao;
    porta->largura = 60.0f;
    porta->altura = 80.0f;
    porta->ativa = true;
    porta->trancada = true;  
    porta->faseDestino = faseDestino;
}

void desenharPorta(Porta *porta, Texture2D texturaPorta) {
    (void)texturaPorta;  
    (void)porta;  

}

bool verificarInteracaoPorta(Porta *porta, Player *jogador) {
    if (!porta->ativa) {
        return false;
    }

    float dx = jogador->posicao.x - porta->posicao.x;
    float dy = jogador->posicao.y - porta->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);

    if (distancia <= 50.0f) {

        if (porta->trancada) {

            if (porta->faseDestino == 2 && !jogador->temChave) {
                DrawText("Precisa da CHAVE", (int)porta->posicao.x - 60, (int)porta->posicao.y - 50, 14, RED);
                return false;
            }

            if (porta->faseDestino == 3 && !jogador->temChave) {
                DrawText("Precisa da CHAVE", (int)porta->posicao.x - 60, (int)porta->posicao.y - 50, 14, RED);
                return false;
            }
        }
        

        DrawText("Pressione E", (int)porta->posicao.x - 45, (int)porta->posicao.y - 50, 16, YELLOW);
        

        if (IsKeyPressed(KEY_E)) {
            if (porta->trancada) {

                if (porta->faseDestino == 2 || porta->faseDestino == 3) {
                    jogador->temChave = false;  
                    printf("Chave usada! Porta destrancada!\n");
                }

                porta->trancada = false;
            }

            return true;
        }
    }
    
    return false;
}

void iniciarHorda(Player *jogador, int numeroHorda) {
    jogador->hordaAtual = numeroHorda;
    jogador->estadoHorda = HORDA_EM_PROGRESSO;
    jogador->zumbisSpawnados = 0;
    jogador->bossesSpawnados = 0;
    jogador->tempoSpawn = 0.0f;  
    jogador->tempoSpawnBoss = 0.0f;  
    

    if (jogador->fase == 1) {

        jogador->bossesTotaisHorda = 0;  
        switch (numeroHorda) {
            case 1:
                jogador->zumbisTotaisHorda = 5;  
                break;
            case 2:
                jogador->zumbisTotaisHorda = 7;  
                break;
            case 3:
                jogador->zumbisTotaisHorda = 2;  
                jogador->bossesTotaisHorda = 1;  
                break;
            default:
                jogador->zumbisTotaisHorda = 5;
                break;
        }
    } else if (jogador->fase == 2) {

        switch (numeroHorda) {
            case 1:
                jogador->zumbisTotaisHorda = 2;   
                jogador->bossesTotaisHorda = 1;   
                break;
            case 2:
                jogador->zumbisTotaisHorda = 2;   
                jogador->bossesTotaisHorda = 1;   
                break;
            case 3:
                jogador->zumbisTotaisHorda = 0;   
                jogador->bossesTotaisHorda = 2;   
                break;
            default:
                jogador->zumbisTotaisHorda = 2;
                jogador->bossesTotaisHorda = 1;
                break;
        }
    } else {

        jogador->zumbisTotaisHorda = 5;
        jogador->bossesTotaisHorda = 0;
    }
    
    jogador->zumbisRestantes = jogador->zumbisTotaisHorda;
    
    printf("=== HORDA %d INICIADA (FASE %d) ===\n", numeroHorda, jogador->fase);
    printf("Zumbis a spawnar: %d\n", jogador->zumbisTotaisHorda);
    printf("Bosses a spawnar: %d\n", jogador->bossesTotaisHorda);
}

int contarBossesVivos(Boss *bosses) {
    if (bosses == NULL) {
        return 0;  
    }
    
    int count = 0;
    Boss *atual = bosses;
    while (atual != NULL) {
        if (atual->ativo) {
            count++;
        }
        atual = atual->proximo;
    }
    return count;
}

void atualizarHorda(Player *jogador, Zumbi **zumbis, Boss **bosses, float deltaTime) {

    if (jogador->vida <= 0 || jogador->jogoVencido) {
        return;
    }
    

    if (jogador == NULL || zumbis == NULL || bosses == NULL) {
        return;
    }
    

    int zumbisVivos = 0;
    Zumbi *z = *zumbis;
    while (z != NULL) {
        zumbisVivos++;
        z = z->proximo;
    }
    jogador->zumbisRestantes = zumbisVivos;
    

    int bossesVivos = contarBossesVivos(*bosses);
    
    switch (jogador->estadoHorda) {
        case HORDA_NAO_INICIADA:

            if ((jogador->fase == 1 || jogador->fase == 2) && jogador->hordaAtual == 0) {
                iniciarHorda(jogador, 1);
            }
            break;
            
        case HORDA_EM_PROGRESSO:

            if (zumbisVivos == 0 && jogador->zumbisSpawnados >= jogador->zumbisTotaisHorda &&
                bossesVivos == 0 && jogador->bossesSpawnados >= jogador->bossesTotaisHorda) {
                jogador->estadoHorda = HORDA_COMPLETA;
                printf("=== HORDA %d COMPLETA! ===\n", jogador->hordaAtual);
                

                int maxHordas = 3; 
                if ((jogador->fase == 1 || jogador->fase == 2) && jogador->hordaAtual < maxHordas) {
                    jogador->estadoHorda = HORDA_INTERVALO;
                    jogador->tempoIntervalo = 10.0f;  
                    printf("Proxima horda em 10 segundos...\n");
                }
            }
            break;
            
        case HORDA_COMPLETA:

            break;
            
        case HORDA_INTERVALO:

            jogador->tempoIntervalo -= deltaTime;
            
            if (jogador->tempoIntervalo <= 0.0f) {

                iniciarHorda(jogador, jogador->hordaAtual + 1);
            }
            break;
    }
}
