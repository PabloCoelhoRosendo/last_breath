#include "menina.h"
#include "pathfinding.h"
#include <math.h>
#include <stdio.h>

void inicializarMenina(Menina *menina) {
    menina->ativa = false;
    menina->seguindo = false;
    menina->raio = 15.0f;
    menina->cooldownTiro = 0.0f;
    menina->alcanceVisao = 300.0f;
    menina->danoTiro = 5;
    menina->timerSom = 0.0f;
    menina->direcaoHorizontal = 1;     // Direita
    menina->direcaoVertical = 0;       // Frente
    menina->estaAtirando = false;
    menina->timerAnimacaoTiro = 0.0f;
    menina->jaTirouPelaVez = false;
}

void carregarSpritesMenina(Menina *menina, Recursos *recursos) {
    menina->spriteDeitada = recursos->meninaDeitada;
    menina->spriteFrenteDireita = recursos->meninaFrenteDireita;
    menina->spriteFrenteEsquerda = recursos->meninaFrenteEsquerda;
    menina->spriteCostas = recursos->meninaCostas;
    menina->spriteAtirandoDireita = recursos->meninaAtirandoDireita;
    menina->spriteAtirandoEsquerda = recursos->meninaAtirandoEsquerda;
    menina->spriteAtual = menina->spriteDeitada;  // Começa deitada
}

void atualizarMenina(Menina *menina, Player *jogador, Mapa *mapa, float deltaTime, Zumbi **zumbis, Bala **balas, Recursos *recursos) {
    if (!menina->ativa) return;

    // Atualizar timer de animação de tiro
    if (menina->estaAtirando) {
        menina->timerAnimacaoTiro -= deltaTime;
        if (menina->timerAnimacaoTiro <= 0.0f) {
            menina->estaAtirando = false;
        }
    }

    // Se está deitada (não seguindo), usar sprite deitada
    if (!menina->seguindo) {
        menina->spriteAtual = menina->spriteDeitada;
        return;
    }

    // Atualizar timer do som da garota (suspiro a cada 8 segundos)
    menina->timerSom += deltaTime;
    if (menina->timerSom >= 8.0f) {
        if (recursos != NULL && recursos->sfxGarotaSuspiro.frameCount > 0) {
            PlaySound(recursos->sfxGarotaSuspiro);
        }
        menina->timerSom = 0.0f;
    }

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

        // Atualizar direção baseado no movimento
        if (fabsf(direcao.y) > fabsf(direcao.x)) {
            // Movimento vertical dominante
            if (direcao.y < 0) {
                menina->direcaoVertical = 1;  // Indo para cima (costas)
            } else {
                menina->direcaoVertical = 0;  // Indo para baixo (frente)
            }
        }

        if (direcao.x < 0) {
            menina->direcaoHorizontal = 0;  // Esquerda
        } else if (direcao.x > 0) {
            menina->direcaoHorizontal = 1;  // Direita
        }

        // Verificar colisão com mapa se disponível
        if (mapa != NULL) {
            // Testar movimento no eixo X
            Vector2 posicaoX = {novaPosicao.x, menina->posicao.y};
            if (!verificarColisaoMapa(mapa, posicaoX, menina->raio)) {
                menina->posicao.x = novaPosicao.x;
            }

            // Testar movimento no eixo Y
            Vector2 posicaoY = {menina->posicao.x, novaPosicao.y};
            if (!verificarColisaoMapa(mapa, posicaoY, menina->raio)) {
                menina->posicao.y = novaPosicao.y;
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
            menina->cooldownTiro = 0.8f;

            // Marcar que está atirando e determinar direção do tiro
            menina->estaAtirando = true;
            menina->timerAnimacaoTiro = 0.3f;  // Animação de tiro dura 0.3s
            menina->jaTirouPelaVez = true;

            // Calcular direção do tiro
            float dx = zumbiMaisProximo->posicao.x - menina->posicao.x;
            if (dx < 0) {
                menina->direcaoHorizontal = 0;  // Atirando para esquerda
            } else {
                menina->direcaoHorizontal = 1;  // Atirando para direita
            }
        }
    }

    // Selecionar sprite baseado no estado
    if (menina->estaAtirando) {
        // Animação de tiro
        if (menina->direcaoHorizontal == 0) {
            menina->spriteAtual = menina->spriteAtirandoEsquerda;
        } else {
            menina->spriteAtual = menina->spriteAtirandoDireita;
        }
    } else {
        // Animação de movimento
        if (menina->direcaoVertical == 1) {
            // Olhando para cima (costas)
            menina->spriteAtual = menina->spriteCostas;
        } else {
            // Olhando para frente
            if (menina->direcaoHorizontal == 0) {
                menina->spriteAtual = menina->spriteFrenteEsquerda;
            } else {
                menina->spriteAtual = menina->spriteFrenteDireita;
            }
        }
    }
}

void desenharMenina(Menina *menina) {
    if (!menina->ativa) return;

    // Verificar se sprite é válido
    if (menina->spriteAtual.id > 0) {
        // Menina deitada mantém escala 0.06f, demais sprites 0.081f (35% maior)
        float escala = (menina->spriteAtual.id == menina->spriteDeitada.id) ? 0.06f : 0.081f;
        float largura = menina->spriteAtual.width * escala;
        float altura = menina->spriteAtual.height * escala;

        Rectangle destino = {
            menina->posicao.x - largura / 2,
            menina->posicao.y - altura / 2,
            largura,
            altura
        };

        Rectangle origem = {
            0, 0,
            (float)menina->spriteAtual.width,
            (float)menina->spriteAtual.height
        };

        DrawTexturePro(menina->spriteAtual, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        // Fallback: círculo rosa se sprite não carregou
        DrawCircleV(menina->posicao, menina->raio, PINK);
        DrawCircleLines((int)menina->posicao.x, (int)menina->posicao.y, menina->raio, PURPLE);
        DrawText("MENINA", (int)menina->posicao.x - 30, (int)menina->posicao.y - 30, 12, WHITE);
    }
}
