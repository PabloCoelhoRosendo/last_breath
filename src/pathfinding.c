#include "pathfinding.h"
#include "mapa.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>

static const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
static const float custos[8] = {1.0f, 1.414f, 1.0f, 1.414f, 1.0f, 1.414f, 1.0f, 1.414f};

void inicializarPathfinding(PathfindingGrid *grid) {
    if (grid == NULL) return;

    resetarGrid(grid);
    grid->inicializado = true;
}

void resetarGrid(PathfindingGrid *grid) {
    if (grid == NULL) return;

    for (int y = 0; y < ALTURA_MAPA; y++) {
        for (int x = 0; x < LARGURA_MAPA; x++) {
            grid->nos[y][x].x = x;
            grid->nos[y][x].y = y;
            grid->nos[y][x].g = FLT_MAX;
            grid->nos[y][x].h = 0.0f;
            grid->nos[y][x].f = FLT_MAX;
            grid->nos[y][x].parentX = -1;
            grid->nos[y][x].parentY = -1;
            grid->nos[y][x].fechado = false;
            grid->nos[y][x].aberto = false;
        }
    }
}

float calcularHeuristica(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    return (float)(dx + dy) + (1.414f - 2.0f) * (float)(dx < dy ? dx : dy);
}

bool posicaoValida(const Mapa *mapa, int x, int y) {
    if (x < 0 || x >= LARGURA_MAPA || y < 0 || y >= ALTURA_MAPA) {
        return false;
    }
    return !isTileSolido(mapa, y, x);
}

static float custoPorProximidade(const Mapa *mapa, int x, int y) {
    float custoExtra = 0.0f;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < LARGURA_MAPA && ny >= 0 && ny < ALTURA_MAPA) {
                if (isTileSolido(mapa, ny, nx)) {
                    // Aumenta o custo para células próximas a obstáculos
                    // Diagonal tem custo maior
                    if (abs(dx) + abs(dy) == 2) {
                        custoExtra += 3.0f; // Diagonal
                    } else {
                        custoExtra += 5.0f; // Adjacente direto
                    }
                }
            }
        }
    }

    return custoExtra;
}

static bool encontrarMenorF(PathfindingGrid *grid, int *outX, int *outY) {
    float menorF = FLT_MAX;
    bool encontrado = false;

    for (int y = 0; y < ALTURA_MAPA; y++) {
        for (int x = 0; x < LARGURA_MAPA; x++) {
            PathNode *no = &grid->nos[y][x];
            if (no->aberto && !no->fechado && no->f < menorF) {
                menorF = no->f;
                *outX = x;
                *outY = y;
                encontrado = true;
            }
        }
    }

    return encontrado;
}

static void reconstruirCaminho(PathfindingGrid *grid, int destinoX, int destinoY, Caminho *caminho) {
    int tamanho = 0;
    int x = destinoX;
    int y = destinoY;

    while (x != -1 && y != -1 && tamanho < MAX_PATH_LENGTH) {
        tamanho++;
        int px = grid->nos[y][x].parentX;
        int py = grid->nos[y][x].parentY;
        x = px;
        y = py;
    }

    if (tamanho > MAX_PATH_LENGTH) {
        tamanho = MAX_PATH_LENGTH;
    }

    caminho->tamanho = tamanho;
    caminho->indiceAtual = 0;
    caminho->valido = true;
    caminho->tempoRecalculo = 0.0f;

    x = destinoX;
    y = destinoY;
    int indice = tamanho - 1;

    while (x != -1 && y != -1 && indice >= 0) {
        caminho->pontos[indice].x = (float)(x * TAMANHO_TILE + TAMANHO_TILE / 2);
        caminho->pontos[indice].y = (float)(y * TAMANHO_TILE + TAMANHO_TILE / 2);

        int px = grid->nos[y][x].parentX;
        int py = grid->nos[y][x].parentY;
        x = px;
        y = py;
        indice--;
    }
}

bool calcularCaminho(PathfindingGrid *grid, const Mapa *mapa,
                     Vector2 inicio, Vector2 destino, Caminho *caminhoSaida) {
    if (grid == NULL || mapa == NULL || caminhoSaida == NULL) {
        return false;
    }

    int inicioX = (int)(inicio.x / TAMANHO_TILE);
    int inicioY = (int)(inicio.y / TAMANHO_TILE);
    int destinoX = (int)(destino.x / TAMANHO_TILE);
    int destinoY = (int)(destino.y / TAMANHO_TILE);

    if (!posicaoValida(mapa, inicioX, inicioY)) {
        bool encontrou = false;
        for (int dy = -1; dy <= 1 && !encontrou; dy++) {
            for (int dx = -1; dx <= 1 && !encontrou; dx++) {
                if (posicaoValida(mapa, inicioX + dx, inicioY + dy)) {
                    inicioX += dx;
                    inicioY += dy;
                    encontrou = true;
                }
            }
        }
        if (!encontrou) {
            caminhoSaida->valido = false;
            return false;
        }
    }

    if (!posicaoValida(mapa, destinoX, destinoY)) {
        bool encontrou = false;
        for (int r = 1; r <= 3 && !encontrou; r++) {
            for (int dy = -r; dy <= r && !encontrou; dy++) {
                for (int dx = -r; dx <= r && !encontrou; dx++) {
                    if (posicaoValida(mapa, destinoX + dx, destinoY + dy)) {
                        destinoX += dx;
                        destinoY += dy;
                        encontrou = true;
                    }
                }
            }
        }
        if (!encontrou) {
            caminhoSaida->valido = false;
            return false;
        }
    }

    if (inicioX == destinoX && inicioY == destinoY) {
        caminhoSaida->tamanho = 1;
        caminhoSaida->indiceAtual = 0;
        caminhoSaida->pontos[0] = destino;
        caminhoSaida->valido = true;
        return true;
    }

    resetarGrid(grid);

    PathNode *noInicial = &grid->nos[inicioY][inicioX];
    noInicial->g = 0.0f;
    noInicial->h = calcularHeuristica(inicioX, inicioY, destinoX, destinoY);
    noInicial->f = noInicial->g + noInicial->h;
    noInicial->aberto = true;

    int iteracoes = 0;
    const int maxIteracoes = LARGURA_MAPA * ALTURA_MAPA;

    while (iteracoes < maxIteracoes) {
        iteracoes++;

        int atualX, atualY;
        if (!encontrarMenorF(grid, &atualX, &atualY)) {
            caminhoSaida->valido = false;
            return false;
        }

        if (atualX == destinoX && atualY == destinoY) {
            reconstruirCaminho(grid, destinoX, destinoY, caminhoSaida);
            return true;
        }

        PathNode *noAtual = &grid->nos[atualY][atualX];
        noAtual->fechado = true;
        noAtual->aberto = false;

        for (int i = 0; i < 8; i++) {
            int vizinhoX = atualX + dx[i];
            int vizinhoY = atualY + dy[i];

            if (!posicaoValida(mapa, vizinhoX, vizinhoY)) {
                continue;
            }

            if (i == 1 || i == 3 || i == 5 || i == 7) {
                int adj1X = atualX + dx[i];
                int adj1Y = atualY;
                int adj2X = atualX;
                int adj2Y = atualY + dy[i];

                if (!posicaoValida(mapa, adj1X, adj1Y) || !posicaoValida(mapa, adj2X, adj2Y)) {
                    continue;
                }
            }

            PathNode *noVizinho = &grid->nos[vizinhoY][vizinhoX];

            if (noVizinho->fechado) {
                continue;
            }

            float custoProximidade = custoPorProximidade(mapa, vizinhoX, vizinhoY);
            float novoG = noAtual->g + custos[i] + custoProximidade;

            if (novoG < noVizinho->g) {
                noVizinho->parentX = atualX;
                noVizinho->parentY = atualY;
                noVizinho->g = novoG;
                noVizinho->h = calcularHeuristica(vizinhoX, vizinhoY, destinoX, destinoY);
                noVizinho->f = noVizinho->g + noVizinho->h;
                noVizinho->aberto = true;
            }
        }
    }

    caminhoSaida->valido = false;
    return false;
}

Vector2 obterDirecaoCaminho(Caminho *caminho, Vector2 posicaoAtual) {
    Vector2 direcao = {0.0f, 0.0f};

    if (caminho == NULL || !caminho->valido || caminho->tamanho == 0) {
        return direcao;
    }

    int indice = caminho->indiceAtual;
    if (indice >= caminho->tamanho) {
        indice = caminho->tamanho - 1;
    }

    Vector2 alvo = caminho->pontos[indice];

    direcao.x = alvo.x - posicaoAtual.x;
    direcao.y = alvo.y - posicaoAtual.y;

    float magnitude = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
    if (magnitude > 0.0f) {
        direcao.x /= magnitude;
        direcao.y /= magnitude;
    }

    return direcao;
}

void atualizarSeguimentoCaminho(Caminho *caminho, Vector2 posicaoAtual, float distanciaMinima) {
    if (caminho == NULL || !caminho->valido || caminho->tamanho == 0) {
        return;
    }

    while (caminho->indiceAtual < caminho->tamanho) {
        Vector2 ponto = caminho->pontos[caminho->indiceAtual];
        float dx = ponto.x - posicaoAtual.x;
        float dy = ponto.y - posicaoAtual.y;
        float distancia = sqrtf(dx * dx + dy * dy);

        if (distancia <= distanciaMinima) {
            caminho->indiceAtual++;
        } else {
            break;
        }
    }

    if (caminho->indiceAtual >= caminho->tamanho) {
        caminho->valido = false;
    }
}

bool precisaRecalcularCaminho(Caminho *caminho, Vector2 destinoAtual, float deltaTime) {
    if (caminho == NULL) {
        return true;
    }

    caminho->tempoRecalculo += deltaTime;

    if (!caminho->valido || caminho->tempoRecalculo >= 1.2f) {
        caminho->tempoRecalculo = 0.0f;
        return true;
    }

    if (caminho->tamanho > 0) {
        Vector2 ultimoPonto = caminho->pontos[caminho->tamanho - 1];
        float dx = ultimoPonto.x - destinoAtual.x;
        float dy = ultimoPonto.y - destinoAtual.y;
        float distancia = sqrtf(dx * dx + dy * dy);

        if (distancia > TAMANHO_TILE * 2) {
            caminho->tempoRecalculo = 0.0f;
            return true;
        }
    }

    return false;
}
