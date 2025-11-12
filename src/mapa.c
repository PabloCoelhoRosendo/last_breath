#include "../include/mapa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// =====================================================================
// FUNÇÕES DE INICIALIZAÇÃO E CARREGAMENTO
// =====================================================================

Mapa* criarMapa(void) {
    Mapa* mapa = (Mapa*)malloc(sizeof(Mapa));
    if (mapa == NULL) {
        return NULL;
    }

    mapa->largura = LARGURA_MAPA;
    mapa->altura = ALTURA_MAPA;
    mapa->tamanhoTile = TAMANHO_TILE;

    // Inicializa todos os tiles como chão
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
        // Ignora linhas de comentário
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r') {
            continue;
        }

        // Parse dos números separados por espaço
        int colunaAtual = 0;
        char* token = strtok(linha, " \t\n\r");

        while (token != NULL && colunaAtual < LARGURA_MAPA) {
            mapa->tiles[linhaAtual][colunaAtual] = atoi(token);
            colunaAtual++;
            token = strtok(NULL, " \t\n\r");
        }

        // Preenche o resto da linha com chão se necessário
        while (colunaAtual < LARGURA_MAPA) {
            mapa->tiles[linhaAtual][colunaAtual] = TILE_CHAO;
            colunaAtual++;
        }

        linhaAtual++;
    }

    fclose(arquivo);

    // Se não leu linhas suficientes, preenche o resto com chão
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

    // Cria um mapa simples com bordas e alguns prédios
    for (int i = 0; i < ALTURA_MAPA; i++) {
        for (int j = 0; j < LARGURA_MAPA; j++) {
            // Bordas
            if (i == 0 || i == ALTURA_MAPA - 1 || j == 0 || j == LARGURA_MAPA - 1) {
                mapa->tiles[i][j] = TILE_PAREDE;
            }
            // Prédio superior esquerdo (vermelho)
            else if (i >= 2 && i <= 5 && j >= 2 && j <= 6) {
                mapa->tiles[i][j] = TILE_PREDIO_VERMELHO;
            }
            // Prédio superior direito (azul)
            else if (i >= 2 && i <= 5 && j >= 25 && j <= 29) {
                mapa->tiles[i][j] = TILE_PREDIO_AZUL;
            }
            // Prédio inferior esquerdo (verde)
            else if (i >= 18 && i <= 21 && j >= 2 && j <= 6) {
                mapa->tiles[i][j] = TILE_PREDIO_VERDE;
            }
            // Prédio inferior direito (amarelo)
            else if (i >= 18 && i <= 21 && j >= 25 && j <= 29) {
                mapa->tiles[i][j] = TILE_PREDIO_AMARELO;
            }
            else {
                mapa->tiles[i][j] = TILE_CHAO;
            }
        }
    }
}

// =====================================================================
// FUNÇÕES DE RENDERIZAÇÃO
// =====================================================================

// Função auxiliar para detectar prédios na matriz
static void detectarPredio(const Mapa* mapa, int linha, int coluna, Predio* predio) {
    int tipo = mapa->tiles[linha][coluna];

    if (tipo == TILE_CHAO || tipo == TILE_PAREDE) {
        predio->tipo = 0;
        return;
    }

    predio->tipo = tipo;
    predio->linhaInicio = linha;
    predio->colunaInicio = coluna;
    predio->larguraTiles = 1;
    predio->alturaTiles = 1;

    // Detectar largura do prédio
    for (int j = coluna + 1; j < mapa->largura; j++) {
        if (mapa->tiles[linha][j] == tipo) {
            predio->larguraTiles++;
        } else {
            break;
        }
    }

    // Detectar altura do prédio
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

    // Matriz para marcar tiles já processados (para prédios)
    bool processado[ALTURA_MAPA][LARGURA_MAPA] = {false};

    // Primeiro, desenhar o chão como uma imagem completa cobrindo toda a tela
    int larguraTelaPx = mapa->largura * mapa->tamanhoTile;  // 1024px
    int alturaTelaPx = mapa->altura * mapa->tamanhoTile;    // 768px

    if (texturasTiles != NULL && texturasTiles[TILE_CHAO].id != 0) {
        // Desenhar a imagem Rua.png esticada para cobrir toda a tela
        Rectangle origem = {0, 0, (float)texturasTiles[TILE_CHAO].width, (float)texturasTiles[TILE_CHAO].height};
        Rectangle destino = {0, 0, (float)larguraTelaPx, (float)alturaTelaPx};
        DrawTexturePro(texturasTiles[TILE_CHAO], origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        // Placeholder cinza claro cobrindo toda a tela
        DrawRectangle(0, 0, larguraTelaPx, alturaTelaPx, (Color){200, 200, 200, 255});
    }

    // Segundo, desenhar os prédios como imagens completas
    for (int i = 0; i < mapa->altura; i++) {
        for (int j = 0; j < mapa->largura; j++) {
            if (processado[i][j]) {
                continue;
            }

            int tipoTile = mapa->tiles[i][j];

            // Se for um prédio (tipos 2, 3, 4, 5)
            if (tipoTile >= TILE_PREDIO_VERMELHO && tipoTile <= TILE_PREDIO_AMARELO) {
                Predio predio;
                detectarPredio(mapa, i, j, &predio);

                if (predio.tipo > 0) {
                    // Calcular posição e dimensões em pixels
                    int x = predio.colunaInicio * mapa->tamanhoTile;
                    int y = predio.linhaInicio * mapa->tamanhoTile;
                    int larguraPx = predio.larguraTiles * mapa->tamanhoTile;
                    int alturaPx = predio.alturaTiles * mapa->tamanhoTile;

                    // Desenhar a imagem completa do prédio
                    if (texturasTiles != NULL && texturasTiles[tipoTile].id != 0) {
                        Rectangle origem = {0, 0, (float)texturasTiles[tipoTile].width, (float)texturasTiles[tipoTile].height};
                        Rectangle destino = {(float)x, (float)y, (float)larguraPx, (float)alturaPx};
                        DrawTexturePro(texturasTiles[tipoTile], origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
                    } else {
                        // Placeholder colorido se imagem não carregar
                        Color cor;
                        switch (tipoTile) {
                            case TILE_PREDIO_VERMELHO:
                                cor = (Color){180, 50, 50, 255};   // Vermelho (Mercado)
                                break;
                            case TILE_PREDIO_AZUL:
                                cor = (Color){50, 100, 180, 255};  // Azul (Posto)
                                break;
                            case TILE_PREDIO_VERDE:
                                cor = (Color){50, 150, 50, 255};   // Verde (Laboratório)
                                break;
                            case TILE_PREDIO_AMARELO:
                                cor = (Color){200, 180, 50, 255};  // Amarelo
                                break;
                            default:
                                cor = (Color){255, 0, 255, 255};   // Magenta
                                break;
                        }
                        DrawRectangle(x, y, larguraPx, alturaPx, cor);
                    }

                    // Marcar todos os tiles deste prédio como processados
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
        }
    }
}

// =====================================================================
// FUNÇÕES DE COLISÃO
// =====================================================================

bool isTileSolido(const Mapa* mapa, int linhaGrid, int colunaGrid) {
    // Verifica se está fora dos limites
    if (linhaGrid < 0 || linhaGrid >= mapa->altura ||
        colunaGrid < 0 || colunaGrid >= mapa->largura) {
        return true; // Trata fora do mapa como sólido
    }

    int tipoTile = mapa->tiles[linhaGrid][colunaGrid];
    // Qualquer tile diferente de chão (0) é sólido
    return (tipoTile != TILE_CHAO);
}

bool verificarColisaoMapa(const Mapa* mapa, Vector2 posicao, float raio) {
    if (mapa == NULL) {
        return false;
    }

    // Verifica os 4 pontos extremos do círculo (cima, baixo, esquerda, direita)
    Vector2 pontos[4] = {
        {posicao.x, posicao.y - raio},           // Cima
        {posicao.x, posicao.y + raio},           // Baixo
        {posicao.x - raio, posicao.y},           // Esquerda
        {posicao.x + raio, posicao.y}            // Direita
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

    // Verifica se a nova posição causa colisão
    return verificarColisaoMapa(mapa, novaPosicao, raio);
}

// =====================================================================
// FUNÇÕES UTILITÁRIAS
// =====================================================================

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

Vector2 gerarPosicaoValidaSpawn(const Mapa* mapa, float raio) {
    if (mapa == NULL) {
        return (Vector2){400, 300};
    }

    // Tenta até 100 vezes encontrar uma posição válida
    for (int tentativa = 0; tentativa < 100; tentativa++) {
        // Gera posição aleatória (evita as bordas)
        int linhaGrid = 2 + rand() % (ALTURA_MAPA - 4);
        int colunaGrid = 2 + rand() % (LARGURA_MAPA - 4);

        // Verifica se o tile e os vizinhos são walkable
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

            // Verifica colisão com o raio completo
            if (!verificarColisaoMapa(mapa, posicaoPixel, raio)) {
                return posicaoPixel;
            }
        }
    }

    // Se não encontrou, retorna centro do mapa
    return (Vector2){
        (LARGURA_MAPA * TAMANHO_TILE) / 2.0f,
        (ALTURA_MAPA * TAMANHO_TILE) / 2.0f
    };
}
