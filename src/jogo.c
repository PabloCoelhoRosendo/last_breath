
#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"
#include "recursos.h"
#include "item.h"
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
    static float deltaTime = 0.016f;
    deltaTime = GetFrameTime();
    
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

// ========== FUNÇÕES DE CARTUCHOS ==========

void adicionarCartucho(Cartucho **cabeca, Vector2 posicao, float angulo, int tipo) {
    Cartucho *novoCartucho = (Cartucho *)malloc(sizeof(Cartucho));
    if (novoCartucho == NULL) return;

    novoCartucho->posicao = posicao;
    novoCartucho->angulo = angulo + GetRandomValue(-30, 30);
    novoCartucho->tempoVida = 0.0f;
    novoCartucho->tipo = tipo;
    
    // Ejeção para o lado direito com variação
    float anguloEjecao = (angulo + 90) * DEG2RAD;
    float velocidade = 100.0f + GetRandomValue(0, 50);
    novoCartucho->velocidadeX = cosf(anguloEjecao) * velocidade;
    novoCartucho->velocidadeY = sinf(anguloEjecao) * velocidade;
    
    novoCartucho->proximo = *cabeca;
    *cabeca = novoCartucho;
}

void atualizarCartuchos(Cartucho **cabeca, float deltaTime) {
    Cartucho *atual = *cabeca;
    Cartucho *anterior = NULL;
    int contador = 0;
    
    while (atual != NULL) {
        atual->tempoVida += deltaTime;
        
        // Física de queda
        atual->posicao.x += atual->velocidadeX * deltaTime;
        atual->posicao.y += atual->velocidadeY * deltaTime;
        atual->velocidadeX *= 0.95f; // Atrito
        atual->velocidadeY *= 0.95f;
        atual->angulo += 360.0f * deltaTime; // Rotação
        
        // Remove cartuchos após 3 segundos ou se tiver muitos (limite de 50)
        if (atual->tempoVida > 3.0f || contador > 50) {
            Cartucho *temp = atual;
            if (anterior == NULL) {
                *cabeca = atual->proximo;
            } else {
                anterior->proximo = atual->proximo;
            }
            atual = atual->proximo;
            free(temp);
        } else {
            contador++;
            anterior = atual;
            atual = atual->proximo;
        }
    }
}

void desenharCartuchos(Cartucho *cabeca) {
    Cartucho *atual = cabeca;
    while (atual != NULL) {
        // Cartucho dourado/amarelo
        Color cor = (Color){218, 165, 32, 255};
        
        // Desenha retângulo pequeno rotacionado
        Rectangle rec = {atual->posicao.x, atual->posicao.y, 4, 8};
        Vector2 origem = {2, 4};
        DrawRectanglePro(rec, origem, atual->angulo, cor);
        
        atual = atual->proximo;
    }
}

void liberarCartuchos(Cartucho **cabeca) {
    Cartucho *atual = *cabeca;
    while (atual != NULL) {
        Cartucho *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
    *cabeca = NULL;
}

// ========== FUNÇÕES DE MANCHAS DE SANGUE ==========

void adicionarManchaSangue(ManchaSangue **cabeca, Vector2 posicao, float raio) {
    ManchaSangue *novaMancha = (ManchaSangue *)malloc(sizeof(ManchaSangue));
    if (novaMancha == NULL) return;

    novaMancha->posicao = posicao;
    novaMancha->raio = raio + GetRandomValue(-5, 5);
    novaMancha->alpha = 200.0f;
    novaMancha->tempoVida = 0.0f;
    
    // Gerar pontos irregulares aleatórios para formato orgânico
    for (int i = 0; i < 8; i++) {
        float angulo = (i * 45.0f) * DEG2RAD;
        float variacao = 0.6f + GetRandomValue(0, 80) / 100.0f; // 0.6 a 1.4
        novaMancha->pontos[i][0] = cosf(angulo) * novaMancha->raio * variacao;
        novaMancha->pontos[i][1] = sinf(angulo) * novaMancha->raio * variacao;
    }
    
    novaMancha->proximo = *cabeca;
    *cabeca = novaMancha;
}

void atualizarManchasSangue(ManchaSangue **cabeca, float deltaTime) {
    ManchaSangue *atual = *cabeca;
    ManchaSangue *anterior = NULL;
    int contador = 0;
    
    while (atual != NULL) {
        atual->tempoVida += deltaTime;
        contador++;
        
        // Fade out gradual nos últimos 0.5 segundos
        if (atual->tempoVida > 2.0f) {
            float tempoRestante = 2.5f - atual->tempoVida;
            atual->alpha = (tempoRestante / 0.5f) * 200.0f;
        }
        
        // Remove após 2.5 segundos ou se tiver muitas (limite 100)
        if (atual->tempoVida >= 2.5f || contador > 100) {
            ManchaSangue *temp = atual;
            if (anterior == NULL) {
                *cabeca = atual->proximo;
            } else {
                anterior->proximo = atual->proximo;
            }
            atual = atual->proximo;
            free(temp);
        } else {
            anterior = atual;
            atual = atual->proximo;
        }
    }
}

void desenharManchasSangue(ManchaSangue *cabeca) {
    ManchaSangue *atual = cabeca;
    while (atual != NULL) {
        Color cor = (Color){139, 0, 0, (unsigned char)atual->alpha};
        
        // Desenhar mancha principal com forma irregular usando triângulos
        for (int i = 0; i < 8; i++) {
            int proximo = (i + 1) % 8;
            
            Vector2 p1 = atual->posicao;
            Vector2 p2 = {
                atual->posicao.x + atual->pontos[i][0],
                atual->posicao.y + atual->pontos[i][1]
            };
            Vector2 p3 = {
                atual->posicao.x + atual->pontos[proximo][0],
                atual->posicao.y + atual->pontos[proximo][1]
            };
            
            DrawTriangle(p1, p2, p3, cor);
        }
        
        atual = atual->proximo;
    }
}

void liberarManchasSangue(ManchaSangue **cabeca) {
    ManchaSangue *atual = *cabeca;
    while (atual != NULL) {
        ManchaSangue *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
    *cabeca = NULL;
}

// ========== FUNÇÕES DE ARMA ==========

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

void atirarArma(Player *jogador, Bala **balas, Vector2 alvo, Cartucho **cartuchos) {
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];
    
    // Modo Deus: munição infinita
    if (!jogador->modoDeus) {
        if (armaAtual->penteAtual <= 0) return;
    }
    if (armaAtual->cooldown > 0.0f) return;
    if (jogador->estaRecarregando) return;
    

    adicionarBala(balas, jogador->posicao, alvo, 0, (float)armaAtual->dano);
    
    // Adiciona cartucho ejetado
    if (cartuchos != NULL) {
        float dx = alvo.x - jogador->posicao.x;
        float dy = alvo.y - jogador->posicao.y;
        float anguloTiro = atan2f(dy, dx) * RAD2DEG;
        adicionarCartucho(cartuchos, jogador->posicao, anguloTiro, armaAtual->tipo);
    }
    
    // Modo Deus: não consome munição
    if (!jogador->modoDeus) {
        armaAtual->penteAtual--;
    }
    

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
    
    jogador->modoDeus = false; // Modo Deus desativado por padrão
    
    jogador->leuRelatorio = false;
    jogador->conheceuMenina = false;
    jogador->meninaLiberada = false;
    jogador->estaNoBanheiro = false;
    jogador->finalFeliz = false;
    jogador->fase2Concluida = false;
    jogador->fase3Concluida = false;
    jogador->matouBossFinal = false;
    jogador->temChaveMisteriosa = false;  // Não começa com a chave do banheiro
    jogador->spawnadoRetornoFase2 = false;
    jogador->spawnadoRetornoFase3 = false;
    jogador->spawnadoRetornoFase4 = false;
    jogador->zumbisSpawnadosRetorno = 0;
    jogador->tempoSpawnRetorno = 0.0f;

    inicializarArma(&jogador->slots[0], ARMA_PISTOL);   
    inicializarArma(&jogador->slots[1], ARMA_NENHUMA);  
    inicializarArma(&jogador->slots[2], ARMA_NENHUMA);  
    jogador->slotAtivo = 0;  
}

void atualizarJogoComPathfinding(Player *jogador, Zumbi **zumbis, Bala **balas, const Mapa *mapa, PathfindingGrid *grid, Cartucho **cartuchos, ManchaSangue **manchas) {
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
        atirarArma(jogador, balas, mousePos, cartuchos);
    }

    atualizarZumbisComPathfinding(zumbis, jogador->posicao, deltaTime, mapa, grid);

    verificarColisoesBalaZumbi(balas, zumbis, jogador, manchas, NULL);
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
    
    Arma *armaAtual = &jogador->slots[jogador->slotAtivo];

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
    
    // Barra de recarga em cima do jogador
    if (jogador->estaRecarregando) {
        float barraLargura = 50.0f;
        float progresso = 1.0f - (jogador->tempoRecargaAtual / armaAtual->tempoRecarga);
        DrawRectangle(
            jogador->posicao.x - barraLargura/2,
            jogador->posicao.y - 30,
            barraLargura,
            6,
            DARKGRAY
        );
        DrawRectangle(
            jogador->posicao.x - barraLargura/2,
            jogador->posicao.y - 30,
            barraLargura * progresso,
            6,
            YELLOW
        );
    }
    
    // Indicador de Modo Deus
    if (jogador->modoDeus) {
        DrawText("MODO DEUS ATIVADO", jogador->posicao.x - 60, jogador->posicao.y - 45, 12, GOLD);
        DrawCircle(jogador->posicao.x, jogador->posicao.y, 20, (Color){255, 215, 0, 50});
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
    DrawText(TextFormat("SISTEMA: %d/100", jogador->vida), hudEsquerdoX + 5, linhaY + 2, 18, WHITE);
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

        // Removido "VITORIA!" - agora só mostra tempo e pódio
        
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
    novoZumbi->vidaMax = 20;
    novoZumbi->raio = 20.0f;

    novoZumbi->tipoMovimento = GetRandomValue(0, 3);

    novoZumbi->velocidadeBase = 90.0f + (float)GetRandomValue(0, 15); 

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

    novoZumbi->tempoTravado = 0.0f;
    novoZumbi->ultimaPosicaoVerificada = posInicial;
    novoZumbi->tempoDano = 0.0f;

    novoZumbi->proximo = *cabeca;
    *cabeca = novoZumbi;
}

void atualizarZumbisComPathfinding(Zumbi **cabeca, Vector2 posicaoJogador, float deltaTime, const Mapa *mapa, PathfindingGrid *grid) {
    Zumbi *atual = *cabeca;
    Zumbi *anterior = NULL;

    while (atual != NULL) {

        atual->posicaoAnterior = atual->posicao;
        
        // Atualiza timer de efeito de dano
        if (atual->tempoDano > 0.0f) {
            atual->tempoDano -= deltaTime;
            if (atual->tempoDano < 0.0f) atual->tempoDano = 0.0f;
        }

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

        // Sistema de detecção de zumbi travado
        float distanciaMovida = sqrtf(
            (atual->posicao.x - atual->ultimaPosicaoVerificada.x) * (atual->posicao.x - atual->ultimaPosicaoVerificada.x) +
            (atual->posicao.y - atual->ultimaPosicaoVerificada.y) * (atual->posicao.y - atual->ultimaPosicaoVerificada.y)
        );

        if (distanciaMovida < 3.0f) {
            atual->tempoTravado += deltaTime;
            
            // Se ficou travado por mais de 1.0s, força movimento perpendicular
            if (atual->tempoTravado > 1.0f) {
                // Força movimento perpendicular ao obstáculo
                float anguloEscape = (float)GetRandomValue(0, 360) * DEG2RAD;
                direcaoFinal.x = cosf(anguloEscape);
                direcaoFinal.y = sinf(anguloEscape);
                velocidadeFinal *= 1.5f; // Boost de velocidade para escapar
                
                // Invalida o caminho atual para recalcular
                atual->caminho.valido = false;
                
                // Reset após 1 segundo travado
                if (atual->tempoTravado > 1.0f) {
                    atual->tempoTravado = 0.0f;
                    atual->ultimaPosicaoVerificada = atual->posicao;
                }
            }
        } else {
            atual->tempoTravado = 0.0f;
            atual->ultimaPosicaoVerificada = atual->posicao;
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

            // Cor base ou vermelha se levou dano recentemente
            Color corZumbi = WHITE;
            if (atual->tempoDano > 0.0f) {
                corZumbi = RED;
            }
            
            DrawTexturePro(atual->spriteAtual, origem, destino, (Vector2){0, 0}, 0.0f, corZumbi);
        } else {

            DrawCircleV(atual->posicao, atual->raio, GREEN);
        }

        float barraLargura = 40.0f;
        float porcentagemVida = (float)atual->vida / (float)atual->vidaMax;
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

void verificarColisoesBalaZumbi(Bala **balas, Zumbi **zumbis, Player *jogador, ManchaSangue **manchas, Moeda **moedas) {
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
                zumbiAtual->tempoDano = 0.1f; // Fica vermelho por 0.1s
                
                // Se morreu, adiciona mancha de sangue e dropa moedas
                if (zumbiAtual->vida <= 0) {
                    if (manchas != NULL) {
                        adicionarManchaSangue(manchas, zumbiAtual->posicao, zumbiAtual->raio);
                    }
                    if (moedas != NULL) {
                        adicionarMoeda(moedas, zumbiAtual->posicao, 10);
                    }
                    if (jogador != NULL) {
                        jogador->zumbisMatados++;
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

                if (jogador->cooldownDanoBala <= 0.0f && !jogador->modoDeus) {
                    int dano = (int)balaAtual->dano;
                    jogador->vida -= dano;
                    jogador->cooldownDanoBala = 0.2f; 
                    
                    printf("ALERTA: Sistema danificado. -%d. Integridade: %d\n", dano, jogador->vida);
                    

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

            if (jogador->cooldownDanoZumbi <= 0.0f && !jogador->modoDeus) {
                int dano = 5; 
                jogador->vida -= dano;
                jogador->cooldownDanoZumbi = 0.5f; 

                printf("ALERTA: Componente danificado. -%d. Integridade: %d\n", dano, jogador->vida);
                
                if (jogador->vida < 0) {
                    jogador->vida = 0;
                }
            }
        }


        zumbiAtual = zumbiAtual->proximo;
    }
}

// ========== FUNÇÕES DA MENINA ==========

void atualizarMenina(Menina *menina, Player *jogador, Mapa *mapa, float deltaTime, Zumbi **zumbis, Bala **balas) {
    if (!menina->ativa || !menina->seguindo) return;
    
    // Atualizar cooldown de tiro
    if (menina->cooldownTiro > 0.0f) {
        menina->cooldownTiro -= deltaTime;
    }
    
    // Menina segue o jogador com distância de 50 pixels
    Vector2 direcao = {
        jogador->posicao.x - menina->posicao.x,
        jogador->posicao.y - menina->posicao.y
    };
    
    float distancia = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
    
    if (distancia > 50.0f) {
        float velocidade = 2.5f;
        
        // Calcular nova posição
        Vector2 novaPosicao = {
            menina->posicao.x + (direcao.x / distancia) * velocidade * 60.0f * deltaTime,
            menina->posicao.y + (direcao.y / distancia) * velocidade * 60.0f * deltaTime
        };
        
        // Verificar colisão com o mapa antes de mover
        if (mapa != NULL) {
            if (!verificarColisaoMapa(mapa, novaPosicao, menina->raio)) {
                // Não há colisão, pode mover
                menina->posicao = novaPosicao;
            } else {
                // Tentar mover apenas no eixo X
                Vector2 posicaoX = {novaPosicao.x, menina->posicao.y};
                if (!verificarColisaoMapa(mapa, posicaoX, menina->raio)) {
                    menina->posicao.x = novaPosicao.x;
                } else {
                    // Tentar mover apenas no eixo Y
                    Vector2 posicaoY = {menina->posicao.x, novaPosicao.y};
                    if (!verificarColisaoMapa(mapa, posicaoY, menina->raio)) {
                        menina->posicao.y = novaPosicao.y;
                    }
                }
            }
        } else {
            // Sem mapa, move livremente
            menina->posicao = novaPosicao;
        }
    }
    
    // Sistema de tiro da menina
    if (menina->cooldownTiro <= 0.0f && zumbis != NULL && *zumbis != NULL) {
        // Procurar zumbi mais próximo dentro do alcance
        Zumbi *zumbiMaisProximo = NULL;
        float menorDistancia = menina->alcanceVisao;
        
        Zumbi *zumbiAtual = *zumbis;
        while (zumbiAtual != NULL) {
            float dx = zumbiAtual->posicao.x - menina->posicao.x;
            float dy = zumbiAtual->posicao.y - menina->posicao.y;
            float dist = sqrtf(dx * dx + dy * dy);
            
            if (dist < menorDistancia) {
                menorDistancia = dist;
                zumbiMaisProximo = zumbiAtual;
            }
            
            zumbiAtual = zumbiAtual->proximo;
        }
        
        // Se encontrou zumbi, atirar
        if (zumbiMaisProximo != NULL) {
            adicionarBala(balas, menina->posicao, zumbiMaisProximo->posicao, 0, (float)menina->danoTiro);
            menina->cooldownTiro = 0.8f; // Atira a cada 0.8 segundos
        }
    }
}

void desenharMenina(Menina *menina) {
    if (!menina->ativa) return;
    
    // Desenhar menina como círculo rosa/roxo
    DrawCircleV(menina->posicao, menina->raio, PINK);
    DrawCircleLines((int)menina->posicao.x, (int)menina->posicao.y, menina->raio, PURPLE);
    DrawText("MENINA", (int)menina->posicao.x - 30, (int)menina->posicao.y - 30, 12, WHITE);
}

// ========== FUNÇÕES DA ESCRIVANINHA ==========

void criarEscrivaninha(Escrivaninha *esc, Vector2 posicao) {
    esc->posicao = posicao;
    esc->largura = 60.0f;
    esc->altura = 40.0f;
    esc->ativa = true;
    esc->lida = false;
}

void desenharEscrivaninha(Escrivaninha *esc) {
    if (!esc->ativa) return;
    
    Color cor = esc->lida ? DARKBROWN : BROWN;
    DrawRectangle(
        (int)esc->posicao.x - (int)esc->largura / 2,
        (int)esc->posicao.y - (int)esc->altura / 2,
        (int)esc->largura,
        (int)esc->altura,
        cor
    );
    DrawRectangleLines(
        (int)esc->posicao.x - (int)esc->largura / 2,
        (int)esc->posicao.y - (int)esc->altura / 2,
        (int)esc->largura,
        (int)esc->altura,
        BLACK
    );
    
    // Papel em cima
    DrawRectangle(
        (int)esc->posicao.x - 15,
        (int)esc->posicao.y - 10,
        30, 20, WHITE
    );
}

bool verificarInteracaoEscrivaninha(Escrivaninha *esc, Player *jogador) {
    if (!esc->ativa) return false;
    
    float distancia = sqrtf(
        (jogador->posicao.x - esc->posicao.x) * (jogador->posicao.x - esc->posicao.x) +
        (jogador->posicao.y - esc->posicao.y) * (jogador->posicao.y - esc->posicao.y)
    );
    
    if (distancia <= 60.0f) {
        DrawText("Pressione E para ler", (int)esc->posicao.x - 70, (int)esc->posicao.y - 50, 14, YELLOW);
        
        if (IsKeyPressed(KEY_E) && !esc->lida) {
            esc->lida = true;
            jogador->leuRelatorio = true;
            return true;
        }
    }
    
    return false;
}