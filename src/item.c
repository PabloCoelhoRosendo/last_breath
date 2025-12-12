#include "raylib.h"
#include "jogo.h"      // Deve vir ANTES para ter as definições completas
#include "item.h"
#include "recursos.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
                // Verificar se é a chave misteriosa (dropada pelo boss final na fase 4)
                if (jogador->fase == 4 && jogador->matouBossFinal) {
                    jogador->temChaveMisteriosa = true;
                    printf("CHAVE MISTERIOSA COLETADA! O monstro engoliu isso...\n");
                    printf("Volte a loja e procure pela porta do banheiro!\n");
                } else {
                    jogador->temChave = true;
                    printf("CHAVE COLETADA! Va ate a porta.\n");
                }
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

bool verificarInteracaoPorta(Porta *porta, Player *jogador, Recursos *recursos) {
    if (!porta->ativa) {
        return false;
    }

    float dx = jogador->posicao.x - porta->posicao.x;
    float dy = jogador->posicao.y - porta->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);

    if (distancia <= 80.0f) {
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
            
            // Tocar som de porta
            if (recursos != NULL && recursos->sfxPorta.frameCount > 0) {
                PlaySound(recursos->sfxPorta);
            }

            return true;
        }
    }
    
    return false;
}

// ========== FUNÇÕES DE MOEDA ==========

void adicionarMoeda(Moeda **lista, Vector2 posicao, int valor) {
    Moeda *novaMoeda = (Moeda *)malloc(sizeof(Moeda));
    if (novaMoeda == NULL) {
        printf("ERRO: Falha ao alocar memória para moeda!\n");
        return;
    }
    
    novaMoeda->posicao = posicao;
    novaMoeda->valor = valor;
    novaMoeda->ativa = true;
    novaMoeda->raio = 12.0f;
    novaMoeda->proximo = *lista;
    *lista = novaMoeda;
}

void desenharMoedas(Moeda *moedas) {
    Moeda *atual = moedas;
    while (atual != NULL) {
        if (atual->ativa) {
            // Desenhar moeda dourada
            DrawCircleV(atual->posicao, atual->raio, GOLD);
            DrawCircleV(atual->posicao, atual->raio - 3, YELLOW);
            DrawText("$", (int)atual->posicao.x - 5, (int)atual->posicao.y - 8, 16, DARKBROWN);
        }
        atual = atual->proximo;
    }
}

void verificarColetaMoedas(Moeda **moedas, Player *jogador, Recursos *recursos) {
    if (moedas == NULL || *moedas == NULL) return;
    
    Moeda *atual = *moedas;
    Moeda *anterior = NULL;
    
    while (atual != NULL) {
        if (atual->ativa) {
            float dx = jogador->posicao.x - atual->posicao.x;
            float dy = jogador->posicao.y - atual->posicao.y;
            float distancia = sqrtf(dx * dx + dy * dy);
            
            // Mostrar prompt E quando próximo
            if (distancia <= 40.0f) {
                DrawText("E", (int)atual->posicao.x - 5, (int)atual->posicao.y - 25, 14, WHITE);
                
                if (IsKeyPressed(KEY_E)) {
                    jogador->moedas += atual->valor;
                    jogador->moedasColetadas += atual->valor;
                    printf("Coletou %d moedas! Total: %d\n", atual->valor, jogador->moedas);
                    
                    // Tocar som de moeda
                    if (recursos != NULL) {
                        PlaySound(recursos->sfxMoeda);
                    }
                    
                    // Remover moeda da lista
                    if (anterior == NULL) {
                        *moedas = atual->proximo;
                    } else {
                        anterior->proximo = atual->proximo;
                    }
                    
                    Moeda *temp = atual;
                    atual = atual->proximo;
                    free(temp);
                    continue;
                }
            }
        }
        
        anterior = atual;
        atual = atual->proximo;
    }
}

void liberarMoedas(Moeda **moedas) {
    if (moedas == NULL) return;
    
    Moeda *atual = *moedas;
    while (atual != NULL) {
        Moeda *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    
    *moedas = NULL;
}
