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
    recursos->texturaBala.id = 0;
    recursos->texturaChave.id = 0;
    recursos->texturaShotgun.id = 0;
    recursos->texturaSMG.id = 0;

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

    // Tile 9: Chão do Laboratório (Fase 3)
    if (FileExists("assets/tiles/chao laboratorio.png")) {
        recursos->texturasTiles[TILE_CHAO_LAB] = LoadTexture("assets/tiles/chao laboratorio.png");
        recursos->chaoLab = recursos->texturasTiles[TILE_CHAO_LAB]; // Referência
        printf("    * chao laboratorio.png carregado (Tile 9 - Fase 3)\n");
    } else if (FileExists("assets/tiles/chao.png")) {
        recursos->texturasTiles[TILE_CHAO_LAB] = LoadTexture("assets/tiles/chao.png");
        recursos->chaoLab = recursos->texturasTiles[TILE_CHAO_LAB]; // Referência
        printf("    * chao.png carregado (Tile 9 - Fase 3, fallback)\n");
    } else {
        printf("    * chao laboratorio.png não encontrado (usando placeholder)\n");
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

    // Tile 10: Porta do Mercado (decoração)
    if (FileExists("assets/tiles/porta do mercado.png")) {
        recursos->texturasTiles[TILE_PORTA_MERCADO] = LoadTexture("assets/tiles/porta do mercado.png");
        printf("    * porta do mercado.png carregado (Tile 10)\n");
    } else {
        printf("    * porta do mercado.png não encontrado (usando placeholder)\n");
    }

    // Tile 11: Porta do Laboratório (ponto de interação invisível - sem textura)
    // Não carrega textura, apenas marca o ponto de interação abaixo do laboratório
    printf("    * Tile 11 (TILE_PORTA_LAB) configurado como ponto de interacao invisivel\n");

    // Tile 12: Parede Invisível (colisão invisível - sem textura)
    // Não carrega textura, usado apenas para colisão nas bordas da Fase 1
    printf("    * Tile 12 (TILE_PAREDE_INVISIVEL) configurado como parede de colisao invisivel\n");

    // =====================================================================
    // TEXTURAS DO JOGADOR
    // =====================================================================
    printf("  - Carregando texturas do jogador...\n");

    // Carregar sprites do jogador: [arquivo, ponteiro para textura, nome para log]
    struct { const char* arquivo; Texture2D* textura; const char* nome; } spritesJogador[] = {
        {"assets/avatar/direita frente.png", &recursos->jogadorDireita, "Direita/Frente"},
        {"assets/avatar/esquerda frente.png", &recursos->jogadorEsquerda, "Esquerda/Frente"},
        {"assets/avatar/costas direita.png", &recursos->jogadorTras, "Costas"},
        {"assets/avatar/direita frente.png", &recursos->jogadorFrente, "Frente"}
    };

    for (int i = 0; i < 4; i++) {
        if (FileExists(spritesJogador[i].arquivo)) {
            *spritesJogador[i].textura = LoadTexture(spritesJogador[i].arquivo);
            printf("    * %s: carregado\n", spritesJogador[i].nome);
        } else {
            printf("    ! %s: nao encontrado\n", spritesJogador[i].nome);
        }
    }

    // =====================================================================
    // TEXTURAS DE ZUMBIS
    // =====================================================================
    printf("  - Carregando texturas de zumbis...\n");

    // Mapeamento: [0]=frente direita, [1]=frente esquerda, [2]=costas direita, [3]=costas esquerda
    const char* nomesDirecoesZumbi[] = {
        "frente direita",
        "frente esquerda",
        "costas direita",
        "costas esquerda"
    };

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            char caminho[256];

            // Carregar de assets/zumbis/
            snprintf(caminho, sizeof(caminho), "assets/zumbis/zumbi %d/zumbi%d %s.png",
                     i + 1, i + 1, nomesDirecoesZumbi[j]);
            if (FileExists(caminho)) {
                recursos->zumbis[i][j] = LoadTexture(caminho);
                printf("    * Carregado: %s\n", caminho);
            } else {
                printf("    ! ERRO: Nao foi possivel carregar sprite zumbi %d direcao %d\n", i + 1, j);
            }
        }
    }

    // =====================================================================
    // TEXTURAS DE BOSSES
    // =====================================================================
    printf("  - Carregando texturas de bosses...\n");

    const char* tiposBoss[] = {"prowler", "hunter", "abomination"};
    const char* direcoesBoss[] = {"frente", "costas", "esquerda", "direita"};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            char caminho[256];
            bool carregado = false;

            // Tentar carregar de assets/bosses/ com extensão .PNG (Prowler)
            snprintf(caminho, sizeof(caminho), "assets/bosses/%s/%s.PNG", tiposBoss[i], direcoesBoss[j]);
            if (FileExists(caminho)) {
                recursos->bosses[i][j] = LoadTexture(caminho);
                carregado = true;
            } else {
                // Tentar com extensão .png (Hunter, Abomination)
                snprintf(caminho, sizeof(caminho), "assets/bosses/%s/%s.png", tiposBoss[i], direcoesBoss[j]);
                if (FileExists(caminho)) {
                    recursos->bosses[i][j] = LoadTexture(caminho);
                    carregado = true;
                }
            }

            if (carregado) {
                printf("    * %s %s: carregado\n", tiposBoss[i], direcoesBoss[j]);
            } else if (j == 0) {  // Só avisar se não tem nem o sprite frontal
                printf("    ! %s: sprite nao encontrado\n", tiposBoss[i]);
            }
        }

        // Abomination: usar sprite frontal para todas as direções se outras não existirem
        if (i == 2 && texturaValida(recursos->bosses[2][0])) {
            for (int j = 1; j < 4; j++) {
                if (!texturaValida(recursos->bosses[2][j])) {
                    recursos->bosses[2][j] = recursos->bosses[2][0];
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

    // =====================================================================
    // TEXTURA DA BALA
    // =====================================================================
    printf("  - Carregando textura da bala...\n");
    if (FileExists("assets/bala/bala.png")) {
        recursos->texturaBala = LoadTexture("assets/bala/bala.png");
        printf("    * Bala: carregado\n");
    } else {
        printf("    ! Bala: nao encontrado (usando circulo)\n");
    }

    // =====================================================================
    // TEXTURA DA CHAVE
    // =====================================================================
    printf("  - Carregando textura da chave...\n");
    if (FileExists("assets/chave/chave.png")) {
        recursos->texturaChave = LoadTexture("assets/chave/chave.png");
        printf("    * Chave: carregado\n");
    } else {
        printf("    ! Chave: nao encontrado (usando circulo)\n");
    }

    // =====================================================================
    // TEXTURAS DAS ARMAS (ITENS)
    // =====================================================================
    printf("  - Carregando texturas das armas...\n");
    if (FileExists("assets/armas/shotgun.png")) {
        recursos->texturaShotgun = LoadTexture("assets/armas/shotgun.png");
        printf("    * Shotgun: carregado\n");
    } else {
        printf("    ! Shotgun: nao encontrado (usando circulo)\n");
    }

    if (FileExists("assets/armas/smg.png")) {
        recursos->texturaSMG = LoadTexture("assets/armas/smg.png");
        printf("    * SMG: carregado\n");
    } else {
        printf("    ! SMG: nao encontrado (usando circulo)\n");
    }

    printf("Recursos carregados com sucesso!\n\n");
}

void descarregarRecursos(Recursos* recursos) {
    if (recursos == NULL) {
        return;
    }

    printf("Descarregando recursos...\n");

    // Descarrega texturas de tiles e zera IDs para prevenir double-free
    for (int i = 0; i < MAX_TIPOS_TILE; i++) {
        if (texturaValida(recursos->texturasTiles[i])) {
            UnloadTexture(recursos->texturasTiles[i]);
            recursos->texturasTiles[i].id = 0;
        }
    }

    // Descarrega texturas do jogador e zera IDs
    if (texturaValida(recursos->jogadorFrente)) {
        UnloadTexture(recursos->jogadorFrente);
        recursos->jogadorFrente.id = 0;
    }
    if (texturaValida(recursos->jogadorTras)) {
        UnloadTexture(recursos->jogadorTras);
        recursos->jogadorTras.id = 0;
    }
    if (texturaValida(recursos->jogadorEsquerda)) {
        UnloadTexture(recursos->jogadorEsquerda);
        recursos->jogadorEsquerda.id = 0;
    }
    if (texturaValida(recursos->jogadorDireita)) {
        UnloadTexture(recursos->jogadorDireita);
        recursos->jogadorDireita.id = 0;
    }

    // Descarrega texturas de zumbis e zera IDs
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            if (texturaValida(recursos->zumbis[i][j])) {
                UnloadTexture(recursos->zumbis[i][j]);
                recursos->zumbis[i][j].id = 0;
            }
        }
    }

    // Descarrega texturas de bosses e zera IDs
    // IMPORTANTE: Evitar double-free nas texturas compartilhadas (Abomination usa mesma textura para múltiplas direções)
    // Rastrear quais texture IDs já foram descarregados
    unsigned int idsDescarregados[12] = {0}; // Máximo de 12 texturas (3 bosses x 4 direções)
    int numDescarregados = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (texturaValida(recursos->bosses[i][j])) {
                // Verificar se este ID já foi descarregado
                bool jaDescarregado = false;
                for (int k = 0; k < numDescarregados; k++) {
                    if (idsDescarregados[k] == recursos->bosses[i][j].id) {
                        jaDescarregado = true;
                        break;
                    }
                }

                if (!jaDescarregado) {
                    UnloadTexture(recursos->bosses[i][j]);
                    idsDescarregados[numDescarregados++] = recursos->bosses[i][j].id;
                }
                recursos->bosses[i][j].id = 0;
            }
        }
    }

    // Descarrega textura de fundo e zera ID
    if (texturaValida(recursos->fundoMapa)) {
        UnloadTexture(recursos->fundoMapa);
        recursos->fundoMapa.id = 0;
    }

    // Descarrega textura da bala
    if (texturaValida(recursos->texturaBala)) {
        UnloadTexture(recursos->texturaBala);
        recursos->texturaBala.id = 0;
    }

    // Zera IDs das referências de chão (já descarregadas via texturasTiles[])
    recursos->chaoMercado.id = 0;
    recursos->chaoRua.id = 0;
    recursos->chaoLab.id = 0;

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
