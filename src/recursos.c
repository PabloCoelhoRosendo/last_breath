#include "../include/recursos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Recursos* criarRecursos(void) {
    Recursos* recursos = (Recursos*)malloc(sizeof(Recursos));
    if (recursos == NULL) {
        return NULL;
    }

    for (int i = 0; i < MAX_TIPOS_TILE; i++) {
        recursos->texturasTiles[i].id = 0;
    }

    recursos->jogadorFrente.id = 0;
    recursos->jogadorTras.id = 0;
    recursos->jogadorTrasCostasEsquerda.id = 0;
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

static bool carregarTextura(Texture2D* destino, const char* caminho, const char* fallback, const char* nome) {
    if (FileExists(caminho)) {
        *destino = LoadTexture(caminho);
        if (nome) printf("    * %s: carregado\n", nome);
        return true;
    } else if (fallback && FileExists(fallback)) {
        *destino = LoadTexture(fallback);
        if (nome) printf("    * %s: carregado (fallback)\n", nome);
        return true;
    }
    if (nome) printf("    ! %s: nao encontrado\n", nome);
    return false;
}

void carregarRecursos(Recursos* recursos) {
    if (recursos == NULL) {
        return;
    }

    printf("Carregando recursos do jogo...\n");

    printf("  - Carregando texturas de tiles...\n");

    carregarTextura(&recursos->texturasTiles[TILE_CHAO], "assets/tiles/Rua.png", NULL, "Rua");
    recursos->chaoRua = recursos->texturasTiles[TILE_CHAO];

    carregarTextura(&recursos->texturasTiles[TILE_CHAO_LAB], "assets/tiles/chao laboratorio.png", "assets/tiles/chao.png", "Chao Laboratorio");
    recursos->chaoLab = recursos->texturasTiles[TILE_CHAO_LAB];

    carregarTextura(&recursos->texturasTiles[TILE_PAREDE], "assets/tiles/parede.png", NULL, "Parede");
    carregarTextura(&recursos->texturasTiles[TILE_PREDIO_VERMELHO], "assets/tiles/Mercado.png", "assets/tiles/predio_vermelho.png", "Mercado");
    carregarTextura(&recursos->texturasTiles[TILE_PREDIO_AZUL], "assets/tiles/Posto.png", "assets/tiles/predio_azul.png", "Posto");
    carregarTextura(&recursos->texturasTiles[TILE_PREDIO_VERDE], "assets/tiles/Laboratorio.png", "assets/tiles/predio_verde.png", "Laboratorio");
    carregarTextura(&recursos->texturasTiles[TILE_PREDIO_AMARELO], "assets/tiles/predio_amarelo.png", NULL, "Predio Amarelo");

    carregarTextura(&recursos->texturasTiles[TILE_CHAO_MERCADO], "assets/tiles/chao mercado.png", NULL, "Chao Mercado");
    recursos->chaoMercado = recursos->texturasTiles[TILE_CHAO_MERCADO];

    carregarTextura(&recursos->texturasTiles[TILE_PRATELEIRA_MERCADO], "assets/tiles/prateleira mercado.png", NULL, "Prateleira Mercado");
    carregarTextura(&recursos->texturasTiles[TILE_CAIXA_MERCADO], "assets/tiles/caixa mercado.png", NULL, "Caixa Mercado");
    carregarTextura(&recursos->texturasTiles[TILE_PORTA_MERCADO], "assets/tiles/porta do mercado.png", NULL, "Porta Mercado");

    printf("    * Tile 11 (TILE_PORTA_LAB) configurado como ponto de interacao invisivel\n");
    printf("    * Tile 12 (TILE_PAREDE_INVISIVEL) configurado como parede de colisao invisivel\n");

    printf("  - Carregando texturas do jogador...\n");
    carregarTextura(&recursos->jogadorDireita, "assets/avatar/direita frente.png", NULL, "Direita/Frente");
    carregarTextura(&recursos->jogadorEsquerda, "assets/avatar/esquerda frente.png", NULL, "Esquerda/Frente");
    carregarTextura(&recursos->jogadorTras, "assets/avatar/costas direita.png", NULL, "Costas Direita");
    carregarTextura(&recursos->jogadorTrasCostasEsquerda, "assets/avatar/costas esquerda.png", NULL, "Costas Esquerda");
    carregarTextura(&recursos->jogadorFrente, "assets/avatar/direita frente.png", NULL, "Frente");

    printf("  - Carregando texturas de zumbis...\n");
    const char* nomesDirecoesZumbi[] = {"frente direita", "frente esquerda", "costas direita", "costas esquerda"};

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            char caminho[256];
            snprintf(caminho, sizeof(caminho), "assets/zumbis/zumbi %d/zumbi%d %s.png", i + 1, i + 1, nomesDirecoesZumbi[j]);

            if (FileExists(caminho)) {
                recursos->zumbis[i][j] = LoadTexture(caminho);
                printf("    * Zumbi %d %s: carregado\n", i + 1, nomesDirecoesZumbi[j]);
            } else {
                printf("    ! Zumbi %d %s: nao encontrado\n", i + 1, nomesDirecoesZumbi[j]);
            }
        }
    }

    printf("  - Carregando texturas de bosses...\n");
    const char* tiposBoss[] = {"prowler", "hunter", "abomination"};
    const char* direcoesBoss[] = {"frente", "costas", "esquerda", "direita"};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            char caminho[256];
            snprintf(caminho, sizeof(caminho), "assets/bosses/%s/%s.PNG", tiposBoss[i], direcoesBoss[j]);

            if (!carregarTextura(&recursos->bosses[i][j], caminho, NULL, NULL)) {
                snprintf(caminho, sizeof(caminho), "assets/bosses/%s/%s.png", tiposBoss[i], direcoesBoss[j]);
                carregarTextura(&recursos->bosses[i][j], caminho, NULL, NULL);
            }

            if (texturaValida(recursos->bosses[i][j])) {
                printf("    * %s %s: carregado\n", tiposBoss[i], direcoesBoss[j]);
            } else if (j == 0) {
                printf("    ! %s: sprite nao encontrado\n", tiposBoss[i]);
            }
        }

        if (i == 2 && texturaValida(recursos->bosses[2][0])) {
            for (int j = 1; j < 4; j++) {
                if (!texturaValida(recursos->bosses[2][j])) {
                    recursos->bosses[2][j] = recursos->bosses[2][0];
                }
            }
        }
    }

    printf("  - Carregando textura de fundo...\n");
    carregarTextura(&recursos->fundoMapa, "assets/fundo_mapa.png", "Novo Projeto.png", "Fundo Mapa");

    printf("  - Carregando texturas de itens...\n");
    carregarTextura(&recursos->texturaBala, "assets/bala/bala.png", NULL, "Bala");
    carregarTextura(&recursos->texturaChave, "assets/chave/chave.png", NULL, "Chave");

    printf("  - Carregando texturas das armas...\n");
    carregarTextura(&recursos->texturaShotgun, "assets/armas/shotgun.png", NULL, "Shotgun");
    carregarTextura(&recursos->texturaSMG, "assets/armas/smg.png", NULL, "SMG");

    printf("Recursos carregados com sucesso!\n\n");
}

void descarregarRecursos(Recursos* recursos) {
    if (recursos == NULL) {
        return;
    }

    printf("Descarregando recursos...\n");

    for (int i = 0; i < MAX_TIPOS_TILE; i++) {
        if (texturaValida(recursos->texturasTiles[i])) {
            UnloadTexture(recursos->texturasTiles[i]);
            recursos->texturasTiles[i].id = 0;
        }
    }

    if (texturaValida(recursos->jogadorFrente)) {
        UnloadTexture(recursos->jogadorFrente);
        recursos->jogadorFrente.id = 0;
    }
    if (texturaValida(recursos->jogadorTras)) {
        UnloadTexture(recursos->jogadorTras);
        recursos->jogadorTras.id = 0;
    }
    if (texturaValida(recursos->jogadorTrasCostasEsquerda)) {
        UnloadTexture(recursos->jogadorTrasCostasEsquerda);
        recursos->jogadorTrasCostasEsquerda.id = 0;
    }
    if (texturaValida(recursos->jogadorEsquerda)) {
        UnloadTexture(recursos->jogadorEsquerda);
        recursos->jogadorEsquerda.id = 0;
    }
    if (texturaValida(recursos->jogadorDireita)) {
        UnloadTexture(recursos->jogadorDireita);
        recursos->jogadorDireita.id = 0;
    }

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            if (texturaValida(recursos->zumbis[i][j])) {
                UnloadTexture(recursos->zumbis[i][j]);
                recursos->zumbis[i][j].id = 0;
            }
        }
    }

    unsigned int idsDescarregados[12] = {0};
    int numDescarregados = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (texturaValida(recursos->bosses[i][j])) {
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

    if (texturaValida(recursos->fundoMapa)) {
        UnloadTexture(recursos->fundoMapa);
        recursos->fundoMapa.id = 0;
    }

    if (texturaValida(recursos->texturaBala)) {
        UnloadTexture(recursos->texturaBala);
        recursos->texturaBala.id = 0;
    }

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

Texture2D obterTexturaTile(Recursos* recursos, int tipoTile) {
    if (recursos == NULL || tipoTile < 0 || tipoTile >= MAX_TIPOS_TILE) {
        return (Texture2D){0};
    }

    return recursos->texturasTiles[tipoTile];
}

bool texturaValida(Texture2D textura) {
    return textura.id != 0;
}
