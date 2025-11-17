#include "../include/recursos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =====================================================================
// FUNÇÕES DE GERENCIAMENTO DE RECURSOS
// =====================================================================

Recursos* criarRecursos(void) {
    Recursos* recursos = (Recursos*)malloc(sizeof(Recursos));
    if (recursos == NULL) {
        return NULL;
    }

    // Inicializa todas as texturas como inválidas
    for (int i = 0; i < MAX_TIPOS_TILE; i++) {
        recursos->texturasTiles[i].id = 0;
    }

    recursos->jogadorFrente.id = 0;
    recursos->jogadorTras.id = 0;
    recursos->jogadorEsquerda.id = 0;
    recursos->jogadorDireita.id = 0;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            recursos->zumbis[i][j].id = 0;
        }
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            recursos->bosses[i][j].id = 0;
        }
    }

    recursos->fundoMapa.id = 0;
    recursos->chaoMercado.id = 0;
    recursos->chaoRua.id = 0;
    recursos->chaoLab.id = 0;

    return recursos;
}

void carregarRecursos(Recursos* recursos) {
    if (recursos == NULL) {
        return;
    }

    printf("Carregando recursos do jogo...\n");

    // =====================================================================
    // TEXTURAS DE TILES
    // =====================================================================
    printf("  - Carregando texturas de tiles...\n");

    // Tile 0: Chão genérico / Rua (Fase 2)
    if (FileExists("assets/tiles/Rua.png")) {
        recursos->texturasTiles[TILE_CHAO] = LoadTexture("assets/tiles/Rua.png");
        recursos->chaoRua = recursos->texturasTiles[TILE_CHAO]; // Referência
        printf("    * Rua.png carregado (Tile 0 - Fase 2)\n");
    } else {
        printf("    * Rua.png não encontrado\n");
    }

    // Carregar chao.png para laboratório se existir
    if (FileExists("assets/tiles/chao.png")) {
        recursos->chaoLab = LoadTexture("assets/tiles/chao.png");
        printf("    * chao.png carregado (Fase 3)\n");
    }

    // Tile 1: Parede (bordas) - não usado na fase 1, mas mantido
    if (FileExists("assets/tiles/parede.png")) {
        recursos->texturasTiles[TILE_PAREDE] = LoadTexture("assets/tiles/parede.png");
        printf("    * parede.png carregado\n");
    } else {
        printf("    * parede.png não encontrado (usando placeholder)\n");
    }

    // Tile 2: MERCADO (Prédio Vermelho)
    if (FileExists("assets/tiles/Mercado.png")) {
        recursos->texturasTiles[TILE_PREDIO_VERMELHO] = LoadTexture("assets/tiles/Mercado.png");
        printf("    * Mercado.png carregado\n");
    } else if (FileExists("assets/tiles/predio_vermelho.png")) {
        recursos->texturasTiles[TILE_PREDIO_VERMELHO] = LoadTexture("assets/tiles/predio_vermelho.png");
        printf("    * predio_vermelho.png carregado (fallback)\n");
    } else {
        printf("    * Mercado.png não encontrado (usando placeholder)\n");
    }

    // Tile 3: POSTO (Prédio Azul)
    if (FileExists("assets/tiles/Posto.png")) {
        recursos->texturasTiles[TILE_PREDIO_AZUL] = LoadTexture("assets/tiles/Posto.png");
        printf("    * Posto.png carregado\n");
    } else if (FileExists("assets/tiles/predio_azul.png")) {
        recursos->texturasTiles[TILE_PREDIO_AZUL] = LoadTexture("assets/tiles/predio_azul.png");
        printf("    * predio_azul.png carregado (fallback)\n");
    } else {
        printf("    * Posto.png não encontrado (usando placeholder)\n");
    }

    // Tile 4: LABORATÓRIO (Prédio Verde)
    if (FileExists("assets/tiles/Laboratorio.png")) {
        recursos->texturasTiles[TILE_PREDIO_VERDE] = LoadTexture("assets/tiles/Laboratorio.png");
        printf("    * Laboratorio.png carregado\n");
    } else if (FileExists("assets/tiles/predio_verde.png")) {
        recursos->texturasTiles[TILE_PREDIO_VERDE] = LoadTexture("assets/tiles/predio_verde.png");
        printf("    * predio_verde.png carregado (fallback)\n");
    } else {
        printf("    * Laboratorio.png não encontrado (usando placeholder)\n");
    }

    // Tile 5: Prédio Amarelo (reservado para futuro uso)
    if (FileExists("assets/tiles/predio_amarelo.png")) {
        recursos->texturasTiles[TILE_PREDIO_AMARELO] = LoadTexture("assets/tiles/predio_amarelo.png");
        printf("    * predio_amarelo.png carregado\n");
    } else {
        printf("    * predio_amarelo.png não encontrado (usando placeholder)\n");
    }

    // Tile 6: Chão do Mercado (Fase 1)
    if (FileExists("assets/tiles/chao mercado.png")) {
        recursos->texturasTiles[TILE_CHAO_MERCADO] = LoadTexture("assets/tiles/chao mercado.png");
        recursos->chaoMercado = recursos->texturasTiles[TILE_CHAO_MERCADO]; // Referência
        printf("    * chao mercado.png carregado (Tile 6)\n");
    } else {
        printf("    * chao mercado.png não encontrado (usando placeholder)\n");
    }

    // Tile 7: Prateleira do Mercado (obstáculo)
    if (FileExists("assets/tiles/prateleira mercado.png")) {
        recursos->texturasTiles[TILE_PRATELEIRA_MERCADO] = LoadTexture("assets/tiles/prateleira mercado.png");
        printf("    * prateleira mercado.png carregado (Tile 7)\n");
    } else {
        printf("    * prateleira mercado.png não encontrado (usando placeholder)\n");
    }

    // Tile 8: Caixa do Mercado (obstáculo)
    if (FileExists("assets/tiles/caixa mercado.png")) {
        recursos->texturasTiles[TILE_CAIXA_MERCADO] = LoadTexture("assets/tiles/caixa mercado.png");
        printf("    * caixa mercado.png carregado (Tile 8)\n");
    } else {
        printf("    * caixa mercado.png não encontrado (usando placeholder)\n");
    }

    // =====================================================================
    // TEXTURAS DO JOGADOR
    // =====================================================================
    printf("  - Carregando texturas do jogador...\n");

    // Tenta carregar das novas pastas primeiro, senão tenta das antigas
    if (FileExists("assets/sprites/player/avatar_frente.png")) {
        recursos->jogadorFrente = LoadTexture("assets/sprites/player/avatar_frente.png");
    } else if (FileExists("avatar/avatar_frente.png")) {
        recursos->jogadorFrente = LoadTexture("avatar/avatar_frente.png");
    }

    if (FileExists("assets/sprites/player/avatar_tras.png")) {
        recursos->jogadorTras = LoadTexture("assets/sprites/player/avatar_tras.png");
    } else if (FileExists("avatar/avatar_tras.png")) {
        recursos->jogadorTras = LoadTexture("avatar/avatar_tras.png");
    }

    if (FileExists("assets/sprites/player/avatar_esquerda.png")) {
        recursos->jogadorEsquerda = LoadTexture("assets/sprites/player/avatar_esquerda.png");
    } else if (FileExists("avatar/avatar_esquerda.png")) {
        recursos->jogadorEsquerda = LoadTexture("avatar/avatar_esquerda.png");
    }

    if (FileExists("assets/sprites/player/avatar_direita.png")) {
        recursos->jogadorDireita = LoadTexture("assets/sprites/player/avatar_direita.png");
    } else if (FileExists("avatar/avatar_direita.png")) {
        recursos->jogadorDireita = LoadTexture("avatar/avatar_direita.png");
    }

    // =====================================================================
    // TEXTURAS DE ZUMBIS
    // =====================================================================
    printf("  - Carregando texturas de zumbis...\n");

    const char* direcoesZumbi[] = {"frente", "tras", "esquerda", "direita"};

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            char caminho[256];

            // Tenta nova estrutura
            snprintf(caminho, sizeof(caminho), "assets/sprites/zombies/zumbi_%d/%s.png", i + 1, direcoesZumbi[j]);
            if (FileExists(caminho)) {
                recursos->zumbis[i][j] = LoadTexture(caminho);
            } else {
                // Tenta estrutura antiga
                snprintf(caminho, sizeof(caminho), "zumbis/zumbi %d/%s.png", i + 1, direcoesZumbi[j]);
                if (FileExists(caminho)) {
                    recursos->zumbis[i][j] = LoadTexture(caminho);
                }
            }
        }
    }

    // =====================================================================
    // TEXTURAS DE BOSSES
    // =====================================================================
    printf("  - Carregando texturas de bosses...\n");

    const char* tiposBoss[] = {"prowler", "hunter", "abomination"};
    const char* direcoesBoss[] = {"frente", "tras", "esquerda", "direita"};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            char caminho[256];

            // Tenta nova estrutura
            snprintf(caminho, sizeof(caminho), "assets/sprites/bosses/%s/%s.png", tiposBoss[i], direcoesBoss[j]);
            if (FileExists(caminho)) {
                recursos->bosses[i][j] = LoadTexture(caminho);
            } else {
                // Tenta estrutura antiga
                snprintf(caminho, sizeof(caminho), "bosses/%s/%s.png", tiposBoss[i], direcoesBoss[j]);
                if (FileExists(caminho)) {
                    recursos->bosses[i][j] = LoadTexture(caminho);
                }
            }
        }
    }

    // =====================================================================
    // TEXTURA DE FUNDO (OPCIONAL)
    // =====================================================================
    if (FileExists("assets/fundo_mapa.png")) {
        recursos->fundoMapa = LoadTexture("assets/fundo_mapa.png");
        printf("  - Textura de fundo carregada\n");
    } else if (FileExists("Novo Projeto.png")) {
        recursos->fundoMapa = LoadTexture("Novo Projeto.png");
        printf("  - Textura de fundo (antiga) carregada\n");
    }

    printf("Recursos carregados com sucesso!\n\n");
}

void descarregarRecursos(Recursos* recursos) {
    if (recursos == NULL) {
        return;
    }

    printf("Descarregando recursos...\n");

    // Descarrega texturas de tiles
    for (int i = 0; i < MAX_TIPOS_TILE; i++) {
        if (texturaValida(recursos->texturasTiles[i])) {
            UnloadTexture(recursos->texturasTiles[i]);
        }
    }

    // Descarrega texturas do jogador
    if (texturaValida(recursos->jogadorFrente)) UnloadTexture(recursos->jogadorFrente);
    if (texturaValida(recursos->jogadorTras)) UnloadTexture(recursos->jogadorTras);
    if (texturaValida(recursos->jogadorEsquerda)) UnloadTexture(recursos->jogadorEsquerda);
    if (texturaValida(recursos->jogadorDireita)) UnloadTexture(recursos->jogadorDireita);

    // Descarrega texturas de zumbis
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            if (texturaValida(recursos->zumbis[i][j])) {
                UnloadTexture(recursos->zumbis[i][j]);
            }
        }
    }

    // Descarrega texturas de bosses
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (texturaValida(recursos->bosses[i][j])) {
                UnloadTexture(recursos->bosses[i][j]);
            }
        }
    }

    // Descarrega textura de fundo
    if (texturaValida(recursos->fundoMapa)) {
        UnloadTexture(recursos->fundoMapa);
    }

    // Descarrega chaoLab (carregado independentemente)
    if (texturaValida(recursos->chaoLab)) {
        UnloadTexture(recursos->chaoLab);
    }

    // Texturas de chão são descarregadas via texturasTiles[]
    // (chaoMercado e chaoRua são apenas referências)

    printf("Recursos descarregados!\n");
}

void destruirRecursos(Recursos* recursos) {
    if (recursos != NULL) {
        free(recursos);
    }
}

// =====================================================================
// FUNÇÕES AUXILIARES
// =====================================================================

Texture2D obterTexturaTile(Recursos* recursos, int tipoTile) {
    if (recursos == NULL || tipoTile < 0 || tipoTile >= MAX_TIPOS_TILE) {
        return (Texture2D){0};
    }

    return recursos->texturasTiles[tipoTile];
}

bool texturaValida(Texture2D textura) {
    return textura.id != 0;
}
