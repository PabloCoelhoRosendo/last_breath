#include "loja.h"
#include "jogo.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>

void inicializarLoja(Loja *loja, Player *jogador) {
    loja->ativo = true;
    loja->menuAberto = false;
    loja->itemSelecionado = 0;
    loja->estadoMercador = 0;  // Sempre normal
    
    // Inicializar itens da loja
    loja->itens[ITEM_LOJA_MUNICAO_PISTOL] = (ItemLoja){
        ITEM_LOJA_MUNICAO_PISTOL,
        "Municao Pistol (30 balas)",
        "Recarregue sua pistola",
        50,
        true
    };
    
    loja->itens[ITEM_LOJA_MUNICAO_SHOTGUN] = (ItemLoja){
        ITEM_LOJA_MUNICAO_SHOTGUN,
        "Municao Shotgun (20 balas)",
        "Cartuchos para shotgun",
        100,
        false
    };
    
    loja->itens[ITEM_LOJA_MUNICAO_SMG] = (ItemLoja){
        ITEM_LOJA_MUNICAO_SMG,
        "Municao SMG (60 balas)",
        "Pentes para SMG",
        120,
        false
    };
    
    loja->itens[ITEM_LOJA_SHOTGUN] = (ItemLoja){
        ITEM_LOJA_SHOTGUN,
        "Shotgun",
        "Arma de alto dano em curta distancia",
        300,
        true
    };
    
    loja->itens[ITEM_LOJA_SMG] = (ItemLoja){
        ITEM_LOJA_SMG,
        "SMG",
        "Arma automatica de alta cadencia",
        400,
        true
    };
    
    loja->itens[ITEM_LOJA_VIDA_PEQUENA] = (ItemLoja){
        ITEM_LOJA_VIDA_PEQUENA,
        "Pacote de Reparo Basico (+30 HP)",
        "Nano-reparadores para sistemas danificados",
        80,
        true
    };
    
    loja->itens[ITEM_LOJA_VIDA_GRANDE] = (ItemLoja){
        ITEM_LOJA_VIDA_GRANDE,
        "Pacote de Reparo Avancado (+70 HP)",
        "Restauracao profunda de componentes criticos",
        150,
        true
    };
    
    loja->itens[ITEM_LOJA_UPGRADE_VIDA_MAX] = (ItemLoja){
        ITEM_LOJA_UPGRADE_VIDA_MAX,
        "Upgrade: Blindagem +20",
        "Reforco estrutural permanente",
        300,
        true
    };
}

void atualizarDisponibilidadeItens(Loja *loja, Player *jogador) {
    // Munição shotgun disponível apenas se tiver shotgun
    loja->itens[ITEM_LOJA_MUNICAO_SHOTGUN].disponivel = false;
    loja->itens[ITEM_LOJA_MUNICAO_SMG].disponivel = false;
    
    for (int i = 0; i < 3; i++) {
        if (jogador->slots[i].tipo == ARMA_SHOTGUN) {
            loja->itens[ITEM_LOJA_MUNICAO_SHOTGUN].disponivel = true;
        }
        if (jogador->slots[i].tipo == ARMA_SMG) {
            loja->itens[ITEM_LOJA_MUNICAO_SMG].disponivel = true;
        }
    }
    
    // Não vender armas que já possui
    loja->itens[ITEM_LOJA_SHOTGUN].disponivel = true;
    loja->itens[ITEM_LOJA_SMG].disponivel = true;
    
    for (int i = 0; i < 3; i++) {
        if (jogador->slots[i].tipo == ARMA_SHOTGUN) {
            loja->itens[ITEM_LOJA_SHOTGUN].disponivel = false;
        }
        if (jogador->slots[i].tipo == ARMA_SMG) {
            loja->itens[ITEM_LOJA_SMG].disponivel = false;
        }
    }
}

void atualizarLoja(Loja *loja, Player *jogador, const Mapa *mapa) {
    if (!loja->ativo) return;

    atualizarDisponibilidadeItens(loja, jogador);

    // Encontrar área completa do bloco da loja (todos os tiles 15)
    int minLinha = -1, maxLinha = -1, minColuna = -1, maxColuna = -1;
    bool lojaEncontrada = false;

    for (int i = 0; i < mapa->altura; i++) {
        for (int j = 0; j < mapa->largura; j++) {
            if (mapa->tiles[i][j] == TILE_LOJA) {
                if (!lojaEncontrada) {
                    minLinha = maxLinha = i;
                    minColuna = maxColuna = j;
                    lojaEncontrada = true;
                } else {
                    if (i < minLinha) minLinha = i;
                    if (i > maxLinha) maxLinha = i;
                    if (j < minColuna) minColuna = j;
                    if (j > maxColuna) maxColuna = j;
                }
            }
        }
    }

    if (!lojaEncontrada) {
        loja->menuAberto = false;
        return;
    }

    // Calcular centro do bloco da loja
    Vector2 posLoja;
    posLoja.x = ((minColuna + maxColuna) / 2.0f) * TAMANHO_TILE + TAMANHO_TILE / 2;
    posLoja.y = ((minLinha + maxLinha) / 2.0f) * TAMANHO_TILE + TAMANHO_TILE / 2;

    // Verificar proximidade à loja (raio maior)
    float distancia = Vector2Distance(jogador->posicao, posLoja);

    // Abrir/fechar menu com E
    if (distancia <= 200.0f) {  // Raio de interação aumentado
        if (IsKeyPressed(KEY_E)) {
            loja->menuAberto = !loja->menuAberto;
            loja->itemSelecionado = 0;
        }
    } else {
        loja->menuAberto = false;
    }
    
    // Navegação no menu com MOUSE
    if (loja->menuAberto) {
        Vector2 mousePos = GetMousePosition();
        
        int menuX = 200;
        int menuY = 100;
        int menuLargura = 624;
        int itemY = menuY + 70;
        
        // Detectar hover sobre itens
        for (int i = 0; i < ITEM_LOJA_TOTAL; i++) {
            Rectangle itemRect = {
                menuX + 10,
                itemY + (i * 55),
                menuLargura - 20,
                50
            };
            
            if (CheckCollisionPointRec(mousePos, itemRect)) {
                loja->itemSelecionado = i;
                
                // Comprar com clique do mouse
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    comprarItem(loja, jogador, i);
                }
            }
        }
        
        // Fechar APENAS com F
        if (IsKeyPressed(KEY_F)) {
            loja->menuAberto = false;
        }
    }
}

void comprarItem(Loja *loja, Player *jogador, TipoItemLoja item) {
    ItemLoja *itemLoja = &loja->itens[item];
    
    if (!itemLoja->disponivel) {
        printf("Item nao disponivel!\n");
        return;
    }
    
    if (jogador->moedas < itemLoja->preco) {
        printf("Dinheiro insuficiente! Precisa de %d moedas.\n", itemLoja->preco);
        return;
    }
    
    // Processar compra
    jogador->moedas -= itemLoja->preco;
    
    switch (item) {
        case ITEM_LOJA_MUNICAO_PISTOL:
            for (int i = 0; i < 3; i++) {
                if (jogador->slots[i].tipo == ARMA_PISTOL) {
                    jogador->slots[i].municaoTotal += 30;
                    printf("Municao adquirida: +30 projeteis\n");
                    break;
                }
            }
            break;
            
        case ITEM_LOJA_MUNICAO_SHOTGUN:
            for (int i = 0; i < 3; i++) {
                if (jogador->slots[i].tipo == ARMA_SHOTGUN) {
                    jogador->slots[i].municaoTotal += 20;
                    printf("Comprou 20 cartuchos de shotgun!\n");
                    break;
                }
            }
            break;
            
        case ITEM_LOJA_MUNICAO_SMG:
            for (int i = 0; i < 3; i++) {
                if (jogador->slots[i].tipo == ARMA_SMG) {
                    jogador->slots[i].municaoTotal += 60;
                    printf("Comprou 60 balas de SMG!\n");
                    break;
                }
            }
            break;
            
        case ITEM_LOJA_SHOTGUN: {
            int slotVazio = -1;
            for (int i = 0; i < 3; i++) {
                if (jogador->slots[i].tipo == ARMA_NENHUMA) {
                    slotVazio = i;
                    break;
                }
            }
            if (slotVazio != -1) {
                inicializarArma(&jogador->slots[slotVazio], ARMA_SHOTGUN);
                printf("Comprou Shotgun! Equipada no slot %d\n", slotVazio + 1);
            }
            break;
        }
            
        case ITEM_LOJA_SMG: {
            int slotVazio = -1;
            for (int i = 0; i < 3; i++) {
                if (jogador->slots[i].tipo == ARMA_NENHUMA) {
                    slotVazio = i;
                    break;
                }
            }
            if (slotVazio != -1) {
                inicializarArma(&jogador->slots[slotVazio], ARMA_SMG);
                printf("Comprou SMG! Equipada no slot %d\n", slotVazio + 1);
            }
            break;
        }
            
        case ITEM_LOJA_VIDA_PEQUENA:
            jogador->vida += 30;
            if (jogador->vida > 100) jogador->vida = 100;
            printf("Nano-reparadores aplicados. Sistema: +30\n");
            break;
            
        case ITEM_LOJA_VIDA_GRANDE:
            jogador->vida += 70;
            if (jogador->vida > 100) jogador->vida = 100;
            printf("Restauracao profunda completa. Sistema: +70\n");
            break;
            
        case ITEM_LOJA_UPGRADE_VIDA_MAX:
            jogador->vida += 20;
            printf("Blindagem refor\u00e7ada. Capacidade maxima: +20\n");
            break;
            
        default:
            break;
    }
}

void desenharLoja(Loja *loja, Player *jogador, const Mapa *mapa) {
    if (!loja->ativo) return;

    // Encontrar área completa do bloco da loja (todos os tiles 15)
    int minLinha = -1, maxLinha = -1, minColuna = -1, maxColuna = -1;
    bool lojaEncontrada = false;

    for (int i = 0; i < mapa->altura; i++) {
        for (int j = 0; j < mapa->largura; j++) {
            if (mapa->tiles[i][j] == TILE_LOJA) {
                if (!lojaEncontrada) {
                    minLinha = maxLinha = i;
                    minColuna = maxColuna = j;
                    lojaEncontrada = true;
                } else {
                    if (i < minLinha) minLinha = i;
                    if (i > maxLinha) maxLinha = i;
                    if (j < minColuna) minColuna = j;
                    if (j > maxColuna) maxColuna = j;
                }
            }
        }
    }

    if (!lojaEncontrada) return;

    // Calcular centro do bloco da loja
    Vector2 posLoja;
    posLoja.x = ((minColuna + maxColuna) / 2.0f) * TAMANHO_TILE + TAMANHO_TILE / 2;
    posLoja.y = ((minLinha + maxLinha) / 2.0f) * TAMANHO_TILE + TAMANHO_TILE / 2;

    // Verificar proximidade para mostrar prompt (raio maior)
    float distancia = Vector2Distance(jogador->posicao, posLoja);

    if (distancia <= 200.0f && !loja->menuAberto) {
        const char *texto = "Pressione E para abrir a loja";
        int largura = MeasureText(texto, 18);
        DrawText(texto, (int)posLoja.x - largura / 2, (int)posLoja.y - 40, 18, YELLOW);
    }
    
    // Menu da loja
    if (loja->menuAberto) {
        int menuX = 200;
        int menuY = 100;
        int menuLargura = 624;
        int menuAltura = 568;
        
        // Fundo do menu
        DrawRectangle(menuX, menuY, menuLargura, menuAltura, (Color){0, 0, 0, 200});
        DrawRectangleLinesEx((Rectangle){menuX, menuY, menuLargura, menuAltura}, 3, GOLD);
        
        // Título
        DrawText("=== LOJA ===", menuX + 240, menuY + 20, 24, GOLD);
        DrawText(TextFormat("Moedas: %d", jogador->moedas), menuX + 450, menuY + 25, 20, YELLOW);
        
        // Lista de itens
        int itemY = menuY + 70;
        for (int i = 0; i < ITEM_LOJA_TOTAL; i++) {
            ItemLoja *item = &loja->itens[i];
            
            Color corItem = item->disponivel ? WHITE : DARKGRAY;
            Color corFundo = (Color){20, 20, 20, 255};
            Color corBorda = DARKGRAY;
            
            // Destacar item selecionado (hover do mouse)
            if (i == loja->itemSelecionado) {
                corFundo = (Color){50, 50, 50, 255};
                corBorda = GOLD;
            }
            
            DrawRectangle(menuX + 10, itemY, menuLargura - 20, 50, corFundo);
            DrawRectangleLinesEx((Rectangle){menuX + 10, itemY, menuLargura - 20, 50}, 2, corBorda);
            
            DrawText(item->nome, menuX + 20, itemY + 8, 18, corItem);
            DrawText(item->descricao, menuX + 20, itemY + 28, 14, GRAY);
            
            Color corPreco = (jogador->moedas >= item->preco && item->disponivel) ? GREEN : RED;
            DrawText(TextFormat("%d moedas", item->preco), menuX + 500, itemY + 15, 18, corPreco);
            
            if (!item->disponivel) {
                DrawText("[INDISPONIVEL]", menuX + 360, itemY + 15, 16, RED);
            }
            
            itemY += 55;
        }
        
        // Instruções
        DrawText("Mouse: Selecionar e Comprar | F: Fechar", menuX + 150, menuY + menuAltura - 35, 16, LIGHTGRAY);
    }
}
