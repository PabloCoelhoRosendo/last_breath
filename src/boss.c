#include "raylib.h"
#include "jogo.h"      // Deve vir ANTES para ter as definições completas
#include "boss.h"
#include "item.h"
#include "mapa.h"
#include "pathfinding.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
    novoBoss->cooldownAtaque = 2.0f;
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

void liberarBosses(Boss **bosses) {
    if (bosses == NULL) return;
    
    Boss *atual = *bosses;
    while (atual != NULL) {
        Boss *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    
    *bosses = NULL;
}
