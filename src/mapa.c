#include "../include/mapa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

Mapa* criarMapa(void) {
    Mapa* mapa = (Mapa*)malloc(sizeof(Mapa));
    if (mapa == NULL) {
        return NULL;
    }

    mapa->largura = LARGURA_MAPA;
    mapa->altura = ALTURA_MAPA;
    mapa->tamanhoTile = TAMANHO_TILE;

    for (int i = 0; i < ALTURA_MAPA; i++) {
        for (int j = 0; j < LARGURA_MAPA; j++) {
            mapa->tiles[i][j] = TILE_CHAO;
        }
    }

    return mapa;
}

void destruirMapa(Mapa* mapa) {
    if (mapa != NULL) {
        free(mapa);
    }
}

bool carregarMapaDeArquivo(Mapa* mapa, const char* caminhoArquivo) {
    if (mapa == NULL || caminhoArquivo == NULL) {
        return false;
    }

    FILE* arquivo = fopen(caminhoArquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo de mapa: %s\n", caminhoArquivo);
        return false;
    }

    char linha[512];
    int linhaAtual = 0;

    while (fgets(linha, sizeof(linha), arquivo) != NULL && linhaAtual < ALTURA_MAPA) {
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r') {
            continue;
        }

        int colunaAtual = 0;
        char* token = strtok(linha, " \t\n\r");

        while (token != NULL && colunaAtual < LARGURA_MAPA) {
            mapa->tiles[linhaAtual][colunaAtual] = atoi(token);
            colunaAtual++;
            token = strtok(NULL, " \t\n\r");
        }

        while (colunaAtual < LARGURA_MAPA) {
            mapa->tiles[linhaAtual][colunaAtual] = TILE_CHAO;
            colunaAtual++;
        }

        linhaAtual++;
    }

    fclose(arquivo);

    while (linhaAtual < ALTURA_MAPA) {
        for (int j = 0; j < LARGURA_MAPA; j++) {
            mapa->tiles[linhaAtual][j] = TILE_CHAO;
        }
        linhaAtual++;
    }

    printf("Mapa carregado com sucesso: %s\n", caminhoArquivo);
    return true;
}

void inicializarMapaPadrao(Mapa* mapa) {
    if (mapa == NULL) {
        return;
    }

    for (int i = 0; i < ALTURA_MAPA; i++) {
        for (int j = 0; j < LARGURA_MAPA; j++) {
            if (i == 0 || i == ALTURA_MAPA - 1 || j == 0 || j == LARGURA_MAPA - 1) {
                mapa->tiles[i][j] = TILE_PAREDE;
            }
            else if (i >= 2 && i <= 5 && j >= 2 && j <= 6) {
                mapa->tiles[i][j] = TILE_MERCADO;
            }
            else if (i >= 2 && i <= 5 && j >= 25 && j <= 29) {
                mapa->tiles[i][j] = TILE_POSTO;
            }
            else if (i >= 18 && i <= 21 && j >= 2 && j <= 6) {
                mapa->tiles[i][j] = TILE_LABORATORIO;
            }
            else if (i >= 18 && i <= 21 && j >= 25 && j <= 29) {
                mapa->tiles[i][j] = TILE_PREDIO_AMARELO;
            }
            else {
                mapa->tiles[i][j] = TILE_CHAO;
            }
        }
    }
}

static void detectarPredio(const Mapa* mapa, int linha, int coluna, Predio* predio) {
    int tipo = mapa->tiles[linha][coluna];

    if (tipo == TILE_CHAO || tipo == TILE_PAREDE || tipo == TILE_CHAO_MERCADO ||
        tipo == TILE_PRATELEIRA_MERCADO || tipo == TILE_CAIXA_MERCADO || tipo == TILE_CHAO_LAB ||
        tipo == TILE_PAREDE_INVISIVEL || tipo == TILE_CHAO_DEPOSITO || tipo == TILE_LOJA) {
        predio->tipo = 0;
        return;
    }

    predio->tipo = tipo;
    predio->linhaInicio = linha;
    predio->colunaInicio = coluna;
    predio->larguraTiles = 1;
    predio->alturaTiles = 1;

    for (int j = coluna + 1; j < mapa->largura; j++) {
        if (mapa->tiles[linha][j] == tipo) {
            predio->larguraTiles++;
        } else {
            break;
        }
    }

    for (int i = linha + 1; i < mapa->altura; i++) {
        if (mapa->tiles[i][coluna] == tipo) {
            predio->alturaTiles++;
        } else {
            break;
        }
    }
}

void desenharMapaTiles(const Mapa* mapa, Texture2D texturasTiles[]) {
    if (mapa == NULL) {
        return;
    }

    bool processado[ALTURA_MAPA][LARGURA_MAPA] = {false};

    int larguraTelaPx = mapa->largura * mapa->tamanhoTile;
    int alturaTelaPx = mapa->altura * mapa->tamanhoTile;

    bool temChaoMercado = false;
    bool temChaoLab = false;
    bool temChaoDeposito = false;
    bool temChaoBanheiro = false;
    for (int i = 0; i < mapa->altura; i++) {
        for (int j = 0; j < mapa->largura; j++) {
            if (mapa->tiles[i][j] == TILE_CHAO_MERCADO) {
                temChaoMercado = true;
            }
            if (mapa->tiles[i][j] == TILE_CHAO_LAB) {
                temChaoLab = true;
            }
            if (mapa->tiles[i][j] == TILE_CHAO_DEPOSITO) {
                temChaoDeposito = true;
            }
            if (mapa->tiles[i][j] == TILE_CHAO_BANHEIRO) {
                temChaoBanheiro = true;
            }
        }
    }

    Texture2D texturaChao;
    if (temChaoBanheiro && texturasTiles != NULL && texturasTiles[TILE_CHAO_BANHEIRO].id != 0) {
        texturaChao = texturasTiles[TILE_CHAO_BANHEIRO];
        printf("[DEBUG] Usando textura do BANHEIRO (tile 16) - ID: %d\n", texturaChao.id);
    } else if (temChaoDeposito && texturasTiles != NULL && texturasTiles[TILE_CHAO_DEPOSITO].id != 0) {
        texturaChao = texturasTiles[TILE_CHAO_DEPOSITO];
    } else if (temChaoMercado && texturasTiles != NULL && texturasTiles[TILE_CHAO_MERCADO].id != 0) {
        texturaChao = texturasTiles[TILE_CHAO_MERCADO];
    } else if (temChaoLab && texturasTiles != NULL && texturasTiles[TILE_CHAO_LAB].id != 0) {
        texturaChao = texturasTiles[TILE_CHAO_LAB];
    } else if (texturasTiles != NULL && texturasTiles[TILE_CHAO].id != 0) {
        texturaChao = texturasTiles[TILE_CHAO];
    } else {
        DrawRectangle(0, 0, larguraTelaPx, alturaTelaPx, (Color){200, 200, 200, 255});
        texturaChao.id = 0;
    }

    if (texturaChao.id != 0) {
        Rectangle origem = {0, 0, (float)texturaChao.width, (float)texturaChao.height};
        Rectangle destino = {0, 0, (float)larguraTelaPx, (float)alturaTelaPx};
        DrawTexturePro(texturaChao, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
    }

    for (int i = 0; i < mapa->altura; i++) {
        for (int j = 0; j < mapa->largura; j++) {
            if (processado[i][j]) {
                continue;
            }

            int tipoTile = mapa->tiles[i][j];

            if (tipoTile >= TILE_MERCADO && tipoTile <= TILE_PREDIO_AMARELO) {
                Predio predio;
                detectarPredio(mapa, i, j, &predio);

                if (predio.tipo > 0) {
                    int x = predio.colunaInicio * mapa->tamanhoTile;
                    int y = predio.linhaInicio * mapa->tamanhoTile;
                    int larguraPx = predio.larguraTiles * mapa->tamanhoTile;
                    int alturaPx = predio.alturaTiles * mapa->tamanhoTile;

                    if (texturasTiles != NULL && texturasTiles[tipoTile].id != 0) {
                        Rectangle origem = {0, 0, (float)texturasTiles[tipoTile].width, (float)texturasTiles[tipoTile].height};
                        Rectangle destino = {(float)x, (float)y, (float)larguraPx, (float)alturaPx};
                        DrawTexturePro(texturasTiles[tipoTile], origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
                    } else {
                        Color cor;
                        switch (tipoTile) {
                            case TILE_MERCADO:
                                cor = (Color){180, 50, 50, 255};
                                break;
                            case TILE_POSTO:
                                cor = (Color){50, 100, 180, 255};
                                break;
                            case TILE_LABORATORIO:
                                cor = (Color){50, 150, 50, 255};
                                break;
                            case TILE_PREDIO_AMARELO:
                                cor = (Color){200, 180, 50, 255};
                                break;
                            default:
                                cor = (Color){255, 0, 255, 255};
                                break;
                        }
                        DrawRectangle(x, y, larguraPx, alturaPx, cor);
                    }

                    for (int pi = 0; pi < predio.alturaTiles; pi++) {
                        for (int pj = 0; pj < predio.larguraTiles; pj++) {
                            if (predio.linhaInicio + pi < ALTURA_MAPA &&
                                predio.colunaInicio + pj < LARGURA_MAPA) {
                                processado[predio.linhaInicio + pi][predio.colunaInicio + pj] = true;
                            }
                        }
                    }
                }
            }
            else if (tipoTile == TILE_PRATELEIRA_MERCADO ||
                     tipoTile == TILE_CAIXA_MERCADO ||
                     tipoTile == TILE_PORTA_MERCADO ||
                     tipoTile == TILE_LOJA) {
                int larguraTiles = 1;
                int alturaTiles = 1;

                for (int jj = j + 1; jj < mapa->largura && mapa->tiles[i][jj] == tipoTile; jj++) {
                    larguraTiles++;
                }

                for (int ii = i + 1; ii < mapa->altura && mapa->tiles[ii][j] == tipoTile; ii++) {
                    alturaTiles++;
                }

                int x = j * mapa->tamanhoTile;
                int y = i * mapa->tamanhoTile;
                int larguraPx = larguraTiles * mapa->tamanhoTile;
                int alturaPx = alturaTiles * mapa->tamanhoTile;

                if (texturasTiles != NULL && texturasTiles[tipoTile].id != 0) {
                    Rectangle origem = {0, 0, (float)texturasTiles[tipoTile].width, (float)texturasTiles[tipoTile].height};
                    Rectangle destino = {(float)x, (float)y, (float)larguraPx, (float)alturaPx};
                    DrawTexturePro(texturasTiles[tipoTile], origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    Color cor;
                    if (tipoTile == TILE_PRATELEIRA_MERCADO) {
                        cor = (Color){139, 90, 43, 255};
                    } else if (tipoTile == TILE_CAIXA_MERCADO) {
                        cor = (Color){101, 67, 33, 255};
                    } else if (tipoTile == TILE_LOJA) {
                        cor = (Color){150, 100, 50, 255};
                    } else {
                        cor = (Color){80, 50, 20, 255};
                    }
                    DrawRectangle(x, y, larguraPx, alturaPx, cor);
                }

                for (int pi = 0; pi < alturaTiles; pi++) {
                    for (int pj = 0; pj < larguraTiles; pj++) {
                        if (i + pi < ALTURA_MAPA && j + pj < LARGURA_MAPA) {
                            processado[i + pi][j + pj] = true;
                        }
                    }
                }
            }
        }
    }
}

bool isTileSolido(const Mapa* mapa, int linhaGrid, int colunaGrid) {
    if (linhaGrid < 0 || linhaGrid >= mapa->altura ||
        colunaGrid < 0 || colunaGrid >= mapa->largura) {
        return true;
    }

    int tipoTile = mapa->tiles[linhaGrid][colunaGrid];
    return (tipoTile != TILE_CHAO &&
            tipoTile != TILE_CHAO_MERCADO &&
            tipoTile != TILE_CHAO_LAB &&
            tipoTile != TILE_CHAO_DEPOSITO &&
            tipoTile != TILE_CHAO_BANHEIRO &&
            tipoTile != TILE_PORTA_MERCADO &&
            tipoTile != TILE_PORTA_LAB &&
            tipoTile != TILE_PORTA_BANHEIRO);
}

bool verificarColisaoMapa(const Mapa* mapa, Vector2 posicao, float raio) {
    if (mapa == NULL) {
        return false;
    }

    Vector2 pontos[4] = {
        {posicao.x, posicao.y - raio},
        {posicao.x, posicao.y + raio},
        {posicao.x - raio, posicao.y},
        {posicao.x + raio, posicao.y}
    };

    for (int i = 0; i < 4; i++) {
        Vector2 gridPos = pixelParaGrid(pontos[i]);
        if (isTileSolido(mapa, (int)gridPos.y, (int)gridPos.x)) {
            return true;
        }
    }

    return false;
}

bool verificarColisaoMovimento(const Mapa* mapa, Vector2 posicaoAtual, Vector2 novaPosicao, float raio) {
    if (mapa == NULL) {
        return false;
    }

    return verificarColisaoMapa(mapa, novaPosicao, raio);
}

Vector2 pixelParaGrid(Vector2 posicaoPixel) {
    return (Vector2){
        posicaoPixel.x / TAMANHO_TILE,
        posicaoPixel.y / TAMANHO_TILE
    };
}

Vector2 gridParaPixel(int linhaGrid, int colunaGrid) {
    return (Vector2){
        colunaGrid * TAMANHO_TILE + TAMANHO_TILE / 2.0f,
        linhaGrid * TAMANHO_TILE + TAMANHO_TILE / 2.0f
    };
}

bool posicaoValidaNoMapa(int linhaGrid, int colunaGrid) {
    return (linhaGrid >= 0 && linhaGrid < ALTURA_MAPA &&
            colunaGrid >= 0 && colunaGrid < LARGURA_MAPA);
}

bool isPosicaoWalkable(const Mapa* mapa, Vector2 posicao, float raio) {
    if (mapa == NULL) {
        return false;
    }

    // Verificar se a posição central está em tile walkable
    Vector2 gridPos = pixelParaGrid(posicao);
    if (isTileSolido(mapa, (int)gridPos.y, (int)gridPos.x)) {
        return false;
    }

    // Verificar os 4 pontos cardinais ao redor do raio
    Vector2 pontos[4] = {
        {posicao.x, posicao.y - raio},
        {posicao.x, posicao.y + raio},
        {posicao.x - raio, posicao.y},
        {posicao.x + raio, posicao.y}
    };

    for (int i = 0; i < 4; i++) {
        Vector2 pontoGrid = pixelParaGrid(pontos[i]);
        if (isTileSolido(mapa, (int)pontoGrid.y, (int)pontoGrid.x)) {
            return false;
        }
    }

    return true;
}

Vector2 certificarPosicaoWalkable(const Mapa* mapa, Vector2 posicaoDesejada, float raio) {
    if (mapa == NULL) {
        printf("[AVISO] Mapa NULL em certificarPosicaoWalkable, usando posição padrão\n");
        return (Vector2){400, 300};
    }

    // Se a posição desejada já é walkable, retorná-la
    if (isPosicaoWalkable(mapa, posicaoDesejada, raio)) {
        printf("[CERTIFICAÇÃO] Posição (%.0f, %.0f) é walkable - OK\n",
               posicaoDesejada.x, posicaoDesejada.y);
        return posicaoDesejada;
    }

    // Se não é walkable, procurar a posição walkable mais próxima
    printf("[AVISO] Posição desejada (%.0f, %.0f) NÃO é walkable! Procurando alternativa...\n",
           posicaoDesejada.x, posicaoDesejada.y);

    Vector2 gridOriginal = pixelParaGrid(posicaoDesejada);
    int linhaOriginal = (int)gridOriginal.y;
    int colunaOriginal = (int)gridOriginal.x;

    // Busca em espiral ao redor da posição original
    for (int raioGrid = 1; raioGrid <= 10; raioGrid++) {
        for (int di = -raioGrid; di <= raioGrid; di++) {
            for (int dj = -raioGrid; dj <= raioGrid; dj++) {
                // Apenas verificar a borda do quadrado (espiral)
                if (abs(di) != raioGrid && abs(dj) != raioGrid) {
                    continue;
                }

                int novaLinha = linhaOriginal + di;
                int novaColuna = colunaOriginal + dj;

                if (!posicaoValidaNoMapa(novaLinha, novaColuna)) {
                    continue;
                }

                if (!isTileSolido(mapa, novaLinha, novaColuna)) {
                    Vector2 novaPosicao = gridParaPixel(novaLinha, novaColuna);

                    if (isPosicaoWalkable(mapa, novaPosicao, raio)) {
                        printf("[CERTIFICAÇÃO] Nova posição walkable encontrada: (%.0f, %.0f) [grid: %d, %d]\n",
                               novaPosicao.x, novaPosicao.y, novaLinha, novaColuna);
                        return novaPosicao;
                    }
                }
            }
        }
    }

    // Se não encontrou nenhuma posição próxima, usar gerarPosicaoValidaSpawn como fallback
    printf("[ERRO] Não encontrou posição walkable próxima! Usando geração aleatória...\n");
    return gerarPosicaoValidaSpawn(mapa, raio);
}

Vector2 gerarPosicaoValidaSpawn(const Mapa* mapa, float raio) {
    if (mapa == NULL) {
        return (Vector2){400, 300};
    }

    for (int tentativa = 0; tentativa < 100; tentativa++) {
        int linhaGrid = 2 + rand() % (ALTURA_MAPA - 4);
        int colunaGrid = 2 + rand() % (LARGURA_MAPA - 4);

        bool valido = true;
        for (int di = -1; di <= 1 && valido; di++) {
            for (int dj = -1; dj <= 1 && valido; dj++) {
                if (isTileSolido(mapa, linhaGrid + di, colunaGrid + dj)) {
                    valido = false;
                }
            }
        }

        if (valido) {
            Vector2 posicaoPixel = gridParaPixel(linhaGrid, colunaGrid);

            if (!verificarColisaoMapa(mapa, posicaoPixel, raio)) {
                printf("[SPAWN] Posição aleatória gerada: (%.0f, %.0f) [grid: %d, %d]\n",
                       posicaoPixel.x, posicaoPixel.y, linhaGrid, colunaGrid);
                return posicaoPixel;
            }
        }
    }

    // Fallback: procurar qualquer tile walkable
    printf("[AVISO] Não conseguiu gerar spawn aleatório, buscando primeiro tile walkable...\n");
    for (int i = 0; i < ALTURA_MAPA; i++) {
        for (int j = 0; j < LARGURA_MAPA; j++) {
            if (!isTileSolido(mapa, i, j)) {
                Vector2 posicao = gridParaPixel(i, j);
                if (isPosicaoWalkable(mapa, posicao, raio)) {
                    printf("[SPAWN FALLBACK] Usando primeiro tile walkable: (%.0f, %.0f) [grid: %d, %d]\n",
                           posicao.x, posicao.y, i, j);
                    return posicao;
                }
            }
        }
    }

    return (Vector2){
        (LARGURA_MAPA * TAMANHO_TILE) / 2.0f,
        (ALTURA_MAPA * TAMANHO_TILE) / 2.0f
    };
}
