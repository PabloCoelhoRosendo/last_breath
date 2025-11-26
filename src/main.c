#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"
#include "mapa.h"
#include "recursos.h"
#include "pathfinding.h"

void detectarPortaNoMapa(Mapa* mapa, Porta* portaPtr) {
    portaPtr->ativa = false;

    for (int i = 0; i < mapa->altura; i++) {
        for (int j = 0; j < mapa->largura; j++) {
            if (mapa->tiles[i][j] == TILE_PORTA_MERCADO) {
                Vector2 posPorta = {(j * 32) + 16, ((i + 3) * 32) + 16};
                criarPorta(portaPtr, posPorta, 2);
                printf("Porta do Mercado encontrada no mapa em tile (%d, %d) -> posicao (%.0f, %.0f)\n",
                       i, j, posPorta.x, posPorta.y);
                return;
            } else if (mapa->tiles[i][j] == TILE_PORTA_LAB) {
                Vector2 posPorta = {(j * 32) + 16, (i * 32) + 16};
                criarPorta(portaPtr, posPorta, 3);
                portaPtr->largura = 100.0f;
                portaPtr->altura = 80.0f;
                printf("Porta do Laboratorio encontrada no mapa em tile (%d, %d) -> posicao (%.0f, %.0f) [Area: %.0fx%.0f]\n",
                       i, j, posPorta.x, posPorta.y, portaPtr->largura, portaPtr->altura);
                return;
            }
        }
    }

    printf("Aviso: Nenhuma porta encontrada no mapa atual\n");
}

int main(void) {
    const int larguraTela = 1024;
    const int alturaTela = 768;

    InitWindow(larguraTela, alturaTela, "Last Breath - Zumbi Survival Game");
    SetTargetFPS(60);

    Texture2D logoTexture = LoadTexture("assets/logo/imagem.png");
    if (logoTexture.id == 0) {
        printf("Aviso: Logo nao foi carregada de assets/logo/imagem.png\n");
    }

    Texture2D backgroundTexture = LoadTexture("assets/background/background.png");
    if (backgroundTexture.id == 0) {
        printf("Aviso: Background nao foi carregado de assets/background/background.png\n");
    }

    Recursos* recursos = criarRecursos();
    carregarRecursos(recursos);

    Mapa* mapaAtual = criarMapa();
    if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
        printf("Aviso: Nao foi possivel carregar fase1.txt. Usando mapa padrao.\n");
        inicializarMapaPadrao(mapaAtual);
    }

    Texture2D texturaMapa = recursos->fundoMapa;
    Texture2D spriteFrenteDireita = recursos->jogadorDireita;
    Texture2D spriteFrenteEsquerda = recursos->jogadorEsquerda;
    Texture2D spriteCostasDireita = recursos->jogadorTras;
    Texture2D spriteCostasEsquerda = recursos->jogadorTras;

    Texture2D prowlerFrente = recursos->bosses[0][0];
    Texture2D prowlerCostas = recursos->bosses[0][1];
    Texture2D prowlerDireita = recursos->bosses[0][3];
    Texture2D prowlerEsquerda = recursos->bosses[0][2];

    Texture2D hunterFrente = recursos->bosses[1][0];
    Texture2D hunterCostas = recursos->bosses[1][1];
    Texture2D hunterDireita = recursos->bosses[1][3];
    Texture2D hunterEsquerda = recursos->bosses[1][2];

    Texture2D abominationFrente = recursos->bosses[2][0];
    Texture2D abominationCostas = recursos->bosses[2][1];
    Texture2D abominationDireita = recursos->bosses[2][3];
    Texture2D abominationEsquerda = recursos->bosses[2][2];

    Player jogador;
    Zumbi *listaZumbis = NULL;
    Boss *listaBosses = NULL;
    Bala *listaBalas = NULL;

    Item itemProgresso;
    itemProgresso.ativo = false;
    Item itemArma;
    itemArma.ativo = false;

    PathfindingGrid pathfindingGrid;
    inicializarPathfinding(&pathfindingGrid);
    printf("Sistema de Pathfinding A* inicializado!\n");

    Porta porta;
    porta.ativa = false;

    detectarPortaNoMapa(mapaAtual, &porta);

    jogador.estadoJogo = ESTADO_MENU;
    bool jogoIniciado = false;

    jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);

    jogador.spriteAtual = spriteFrenteDireita;

    while (!WindowShouldClose()) {
        if (jogador.estadoJogo == ESTADO_MENU) {
            Rectangle botaoJogar = {
                larguraTela / 2 - 100,
                alturaTela / 2 + 150,
                200,
                60
            };

            Vector2 mousePos = GetMousePosition();
            bool mouseNoBot = CheckCollisionPointRec(mousePos, botaoJogar);

            if (mouseNoBot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (!jogoIniciado) {
                    iniciarJogo(&jogador);

                    if (jogador.fase == 3) {
                        jogador.posicao = (Vector2){16 * 32 + 16, 21 * 32 + 16};
                        printf("Spawn inicial Fase 3: (linha=21, coluna=16 - meio) -> (%.0f, %.0f)\n",
                               jogador.posicao.x, jogador.posicao.y);
                    } else {
                        jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
                    }

                    jogador.spriteAtual = spriteFrenteDireita;
                    jogoIniciado = true;
                }
                jogador.estadoJogo = ESTADO_JOGANDO;
                printf("Jogo iniciado!\n");
            }

            BeginDrawing();
            ClearBackground(BLACK);

            if (backgroundTexture.id > 0) {
                DrawTexturePro(
                    backgroundTexture,
                    (Rectangle){0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                    (Rectangle){0, 0, (float)larguraTela, (float)alturaTela},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE
                );
            }

            if (logoTexture.id > 0) {
                float escalaLogo = 0.8f;
                float larguraLogo = logoTexture.width * escalaLogo;
                float alturaLogo = logoTexture.height * escalaLogo;

                DrawTexturePro(
                    logoTexture,
                    (Rectangle){0, 0, (float)logoTexture.width, (float)logoTexture.height},
                    (Rectangle){larguraTela / 2 - larguraLogo / 2, 100, larguraLogo, alturaLogo},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE
                );
            } else {
                DrawText("LAST BREATH", larguraTela / 2 - 200, 150, 60, RED);
            }

            Color corBotao = mouseNoBot ? GREEN : DARKGREEN;
            DrawRectangleRec(botaoJogar, corBotao);
            DrawRectangleLinesEx(botaoJogar, 3, LIME);

            const char* textoBotao = "JOGAR";
            int larguraTexto = MeasureText(textoBotao, 30);
            DrawText(
                textoBotao,
                botaoJogar.x + botaoJogar.width / 2 - larguraTexto / 2,
                botaoJogar.y + 15,
                30,
                WHITE
            );

            EndDrawing();
            continue;
        }

        if ((jogador.vida <= 0 || jogador.jogoVencido) && IsKeyPressed(KEY_R)) {
            liberarZumbis(&listaZumbis);
            listaZumbis = NULL;

            while (listaBosses != NULL) {
                Boss *temp = listaBosses;
                listaBosses = listaBosses->proximo;
                free(temp);
            }

            while (listaBalas != NULL) {
                Bala *temp = listaBalas;
                listaBalas = listaBalas->proximo;
                free(temp);
            }


            if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
                inicializarMapaPadrao(mapaAtual);
            }

            itemProgresso.ativo = false;
            itemArma.ativo = false;
            criarPorta(&porta, (Vector2){960, 384}, 2);

            jogador.estadoJogo = ESTADO_MENU;
            jogoIniciado = false;

            printf("Voltando ao menu principal...\n");
        }

        Vector2 posicaoAnteriorJogador = jogador.posicao;

        atualizarJogoComPathfinding(&jogador, &listaZumbis, &listaBalas, mapaAtual, &pathfindingGrid);

        if (jogador.fase == 1 || jogador.fase == 2) {
            atualizarHorda(&jogador, &listaZumbis, &listaBosses, GetFrameTime());

            if (jogador.estadoHorda == HORDA_EM_PROGRESSO &&
                jogador.zumbisSpawnados < jogador.zumbisTotaisHorda) {

                jogador.tempoSpawn += GetFrameTime();

                if (jogador.tempoSpawn >= 1.0f) {
                    Vector2 posSpawn = gerarPosicaoValidaSpawn(mapaAtual, 20.0f);
                    adicionarZumbi(&listaZumbis, posSpawn, recursos->zumbis);
                    jogador.zumbisSpawnados++;
                    jogador.tempoSpawn = 0.0f;
                    printf("Zumbi spawnado! (%d/%d)\n", jogador.zumbisSpawnados, jogador.zumbisTotaisHorda);
                }
            }

            if (jogador.estadoHorda == HORDA_EM_PROGRESSO &&
                jogador.bossesSpawnados < jogador.bossesTotaisHorda) {

                jogador.tempoSpawnBoss += GetFrameTime();

                if (jogador.tempoSpawnBoss >= 2.0f) {
                    Vector2 posSpawn = gerarPosicaoValidaSpawn(mapaAtual, 25.0f);

                    if (jogador.fase == 1) {
                        criarBoss(&listaBosses, BOSS_PROWLER, posSpawn, prowlerFrente, prowlerCostas, prowlerDireita, prowlerEsquerda);
                        printf("PROWLER spawnado! (%d/%d)\n", jogador.bossesSpawnados + 1, jogador.bossesTotaisHorda);
                    } else if (jogador.fase == 2) {
                        criarBoss(&listaBosses, BOSS_HUNTER, posSpawn, hunterFrente, hunterCostas, hunterDireita, hunterEsquerda);
                        printf("HUNTER spawnado! (%d/%d)\n", jogador.bossesSpawnados + 1, jogador.bossesTotaisHorda);
                    }

                    jogador.bossesSpawnados++;
                    jogador.tempoSpawnBoss = 0.0f;
                }
            }
        }

        atualizarBalas(&listaBalas, mapaAtual);

        if (verificarColisaoMapa(mapaAtual, jogador.posicao, 15.0f)) {
            jogador.posicao = posicaoAnteriorJogador;
        }

        Zumbi *zumbiAtual = listaZumbis;
        while (zumbiAtual != NULL) {
            if (verificarColisaoMapa(mapaAtual, zumbiAtual->posicao, 20.0f)) {
                Vector2 posOriginal = zumbiAtual->posicaoAnterior;
                Vector2 movimento = {
                    zumbiAtual->posicao.x - posOriginal.x,
                    zumbiAtual->posicao.y - posOriginal.y
                };

                bool encontrouCaminho = false;

                Vector2 tentativaX = {posOriginal.x + movimento.x, posOriginal.y};
                if (!verificarColisaoMapa(mapaAtual, tentativaX, 20.0f)) {
                    zumbiAtual->posicao = tentativaX;
                    encontrouCaminho = true;
                }

                if (!encontrouCaminho) {
                    Vector2 tentativaY = {posOriginal.x, posOriginal.y + movimento.y};
                    if (!verificarColisaoMapa(mapaAtual, tentativaY, 20.0f)) {
                        zumbiAtual->posicao = tentativaY;
                        encontrouCaminho = true;
                    }
                }

                if (!encontrouCaminho) {
                    float velocidade = sqrtf(movimento.x * movimento.x + movimento.y * movimento.y);
                    if (velocidade > 0.1f) {
                        Vector2 desvioDir = {posOriginal.x - movimento.y * 0.5f, posOriginal.y + movimento.x * 0.5f};
                        if (!verificarColisaoMapa(mapaAtual, desvioDir, 20.0f)) {
                            zumbiAtual->posicao = desvioDir;
                            encontrouCaminho = true;
                        }

                        if (!encontrouCaminho) {
                            Vector2 desvioEsq = {posOriginal.x + movimento.y * 0.5f, posOriginal.y - movimento.x * 0.5f};
                            if (!verificarColisaoMapa(mapaAtual, desvioEsq, 20.0f)) {
                                zumbiAtual->posicao = desvioEsq;
                                encontrouCaminho = true;
                            }
                        }
                    }
                }

                if (!encontrouCaminho) {
                    zumbiAtual->posicao = posOriginal;
                    zumbiAtual->caminho.valido = false;
                    zumbiAtual->caminho.tempoRecalculo = 1.0f;
                }
            }
            zumbiAtual = zumbiAtual->proximo;
        }

        if (!jogador.bossSpawnado && jogador.vida > 0) {
            if (jogador.fase == 1) {
            }
            else if (jogador.fase == 2) {
            }
            else if (jogador.fase == 3) {
                jogador.timerBoss += GetFrameTime();

                int numZumbis = 0;
                Zumbi *z = listaZumbis;
                while (z != NULL) {
                    numZumbis++;
                    z = z->proximo;
                }

                if (jogador.timerBoss >= 45.0f || numZumbis == 0) {
                    Vector2 posicaoBoss = {512.0f, 384.0f};
                    criarBoss(&listaBosses, BOSS_ABOMINATION, posicaoBoss, abominationFrente, abominationCostas, abominationDireita, abominationEsquerda);
                    printf("BOSS APARECEU: ABOMINATION!\n");

                    jogador.bossSpawnado = true;
                }
            }
        }

        atualizarBossComPathfinding(&listaBosses, &jogador, &listaBalas, GetFrameTime(), mapaAtual, &pathfindingGrid);

        Boss *bossAtual = listaBosses;
        while (bossAtual != NULL) {
            if (verificarColisaoMapa(mapaAtual, bossAtual->posicao, bossAtual->raio)) {
                Vector2 posOriginal = bossAtual->posicaoAnterior;
                Vector2 movimento = {
                    bossAtual->posicao.x - posOriginal.x,
                    bossAtual->posicao.y - posOriginal.y
                };

                bool encontrouCaminho = false;

                Vector2 tentativaX = {posOriginal.x + movimento.x, posOriginal.y};
                if (!verificarColisaoMapa(mapaAtual, tentativaX, bossAtual->raio)) {
                    bossAtual->posicao = tentativaX;
                    encontrouCaminho = true;
                }

                if (!encontrouCaminho) {
                    Vector2 tentativaY = {posOriginal.x, posOriginal.y + movimento.y};
                    if (!verificarColisaoMapa(mapaAtual, tentativaY, bossAtual->raio)) {
                        bossAtual->posicao = tentativaY;
                        encontrouCaminho = true;
                    }
                }

                if (!encontrouCaminho) {
                    bossAtual->posicao = posOriginal;
                    bossAtual->caminho.valido = false;
                    bossAtual->caminho.tempoRecalculo = 1.0f;
                }
            }
            bossAtual = bossAtual->proximo;
        }

        verificarColisoesBossBala(&listaBosses, &listaBalas, &itemProgresso, &itemArma, &jogador);
        verificarColisoesBossJogador(listaBosses, &jogador);

        if (itemProgresso.ativo) {
            verificarColetaItem(&itemProgresso, &jogador);
        }
        if (itemArma.ativo) {
            verificarColetaItem(&itemArma, &jogador);
        }

        if (porta.ativa && verificarInteracaoPorta(&porta, &jogador)) {
            printf("Usando porta! Indo para Fase %d\n", porta.faseDestino);
            jogador.fase = porta.faseDestino;
            jogador.timerBoss = 0.0f;
            jogador.bossSpawnado = false;

            printf("Limpando zumbis da fase anterior...\n");
            liberarZumbis(&listaZumbis);
            listaZumbis = NULL;

            printf("Limpando bosses da fase anterior...\n");
            int bossCount = 0;
            while (listaBosses != NULL) {
                Boss *temp = listaBosses;
                listaBosses = listaBosses->proximo;
                free(temp);
                bossCount++;
            }
            listaBosses = NULL;
            printf("Bosses liberados: %d\n", bossCount);

            printf("Limpando balas da fase anterior...\n");
            int balaCount = 0;
            while (listaBalas != NULL) {
                Bala *temp = listaBalas;
                listaBalas = listaBalas->proximo;
                free(temp);
                balaCount++;
            }
            listaBalas = NULL;
            printf("Balas liberadas: %d\n", balaCount);
            printf("Limpeza concluida! Iniciando nova fase...\n");

            jogador.hordaAtual = 0;
            jogador.estadoHorda = HORDA_NAO_INICIADA;
            jogador.zumbisRestantes = 0;
            jogador.zumbisTotaisHorda = 0;
            jogador.zumbisSpawnados = 0;
            jogador.bossesTotaisHorda = 0;
            jogador.bossesSpawnados = 0;
            jogador.tempoIntervalo = 0.0f;
            jogador.tempoSpawn = 0.0f;
            jogador.tempoSpawnBoss = 0.0f;

            char caminhoMapa[64];
            snprintf(caminhoMapa, sizeof(caminhoMapa), "assets/maps/fase%d.txt", jogador.fase);
            if (!carregarMapaDeArquivo(mapaAtual, caminhoMapa)) {
                printf("Aviso: Nao foi possivel carregar %s. Usando mapa padrao.\n", caminhoMapa);
                inicializarMapaPadrao(mapaAtual);
            }

            if (jogador.fase == 2) {
                jogador.posicao = (Vector2){8 * 32 + 16, 9 * 32 + 16};
                printf("Jogador spawnado na Fase 2 em posicao fixa: (linha=9, coluna=8) -> (%.0f, %.0f)\n",
                       jogador.posicao.x, jogador.posicao.y);
            } else if (jogador.fase == 3) {
                jogador.posicao = (Vector2){16 * 32 + 16, 21 * 32 + 16};
                printf("Jogador spawnado na Fase 3 em posicao fixa: (linha=21, coluna=16 - meio) -> (%.0f, %.0f)\n",
                       jogador.posicao.x, jogador.posicao.y);
            } else {
                jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
            }

            if (jogador.fase == 2) {
                jogador.velocidadeBase = 5.0f;
                printf("UPGRADE! Velocidade aumentada para 5.0 m/s\n");
            }

            detectarPortaNoMapa(mapaAtual, &porta);
        }

        if (jogador.direcaoVertical == 0) {
            if (jogador.direcaoHorizontal == 0) {
                jogador.spriteAtual = spriteFrenteEsquerda;
            } else {
                jogador.spriteAtual = spriteFrenteDireita;
            }
        } else {
            if (jogador.direcaoHorizontal == 0) {
                jogador.spriteAtual = spriteCostasEsquerda;
            } else {
                jogador.spriteAtual = spriteCostasDireita;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        desenharMapaTiles(mapaAtual, recursos->texturasTiles);

        desenharJogo(&jogador, listaZumbis, listaBalas, texturaMapa, recursos);

        if (jogador.vida > 0 && !jogador.jogoVencido) {
            desenharBoss(listaBosses);

            if (jogador.fase == 1 || jogador.fase == 2) {
                if (jogador.estadoHorda == HORDA_INTERVALO) {
                    int segundosIntervalo = (int)jogador.tempoIntervalo + 1;
                    const char* mensagem = TextFormat("PROXIMA HORDA EM %ds", segundosIntervalo);

                    int larguraTexto = MeasureText(mensagem, 32);
                    int posX = (1024 - larguraTexto) / 2;

                    DrawRectangle(posX - 20, 200, larguraTexto + 40, 50, (Color){0, 0, 0, 180});
                    DrawText(mensagem, posX, 210, 32, YELLOW);
                } else if (jogador.estadoHorda == HORDA_EM_PROGRESSO) {
                    const char* faseNome = (jogador.fase == 1) ? "FASE 1" : "FASE 2";
                    DrawText(TextFormat("%s - HORDA %d/3", faseNome, jogador.hordaAtual), 10, 150, 20, YELLOW);

                    int inimigosTotais = jogador.zumbisRestantes + contarBossesVivos(listaBosses);
                    DrawText(TextFormat("Inimigos: %d", inimigosTotais), 10, 175, 18, WHITE);

                    if (jogador.bossesTotaisHorda > 0) {
                        int bossesVivos = contarBossesVivos(listaBosses);
                        DrawText(TextFormat("Bosses: %d/%d", bossesVivos, jogador.bossesTotaisHorda), 10, 195, 18, RED);
                    }
                } else if (jogador.estadoHorda == HORDA_COMPLETA && jogador.hordaAtual == 3) {
                    const char* mensagem = "TODAS AS HORDAS COMPLETAS!";
                    int larguraTexto = MeasureText(mensagem, 24);
                    int posX = (1024 - larguraTexto) / 2;
                    DrawText(mensagem, posX, 210, 24, GREEN);
                }
            }

            if (!jogador.bossSpawnado && jogador.fase == 3) {
                int segundosRestantes = (int)(45.0f - jogador.timerBoss);
                if (segundosRestantes < 0) segundosRestantes = 0;
                DrawText(TextFormat("BOSS EM: %ds", segundosRestantes), 320, 10, 24, RED);
            }

            if (porta.ativa && jogador.fase == 1) {
                desenharPorta(&porta, recursos->texturasTiles[TILE_PORTA_MERCADO]);
            }

            if (itemProgresso.ativo) {
                desenharItem(&itemProgresso, recursos);
            }
            if (itemArma.ativo) {
                desenharItem(&itemArma, recursos);
            }

            int offsetX = 1024 - 10;
            if (jogador.temCure) {
                int largura = MeasureText("CURE", 18);
                offsetX -= largura;
                DrawText("CURE", offsetX, 10, 18, GREEN);
                offsetX -= 15;
            }
            if (jogador.temMapa) {
                int largura = MeasureText("MAPA", 18);
                offsetX -= largura;
                DrawText("MAPA", offsetX, 10, 18, SKYBLUE);
                offsetX -= 15;
            }
            if (jogador.temChave) {
                int largura = MeasureText("CHAVE", 18);
                offsetX -= largura;
                DrawText("CHAVE", offsetX, 10, 18, GOLD);
            }

            if ((jogador.fase == 1 && jogador.temChave && porta.ativa) ||
                (jogador.fase == 2 && jogador.temMapa && porta.ativa)) {
                float tempoTotal = GetTime();
                int mostrar = ((int)(tempoTotal * 2)) % 2;

                if (mostrar) {
                    const char* mensagem = (jogador.fase == 1) ?
                        ">> VA PARA A PORTA! <<" :
                        ">> VA PARA A PORTA DO LABORATORIO! <<";
                    int larguraMensagem = MeasureText(mensagem, 20);
                    int posX = 1024 - larguraMensagem - 10;
                    int posY = 40;

                    DrawRectangle(posX - 5, posY - 5, larguraMensagem + 10, 30, (Color){0, 0, 0, 150});
                    DrawText(mensagem, posX, posY, 20, YELLOW);
                }
            }
        }

        EndDrawing();
    }

    liberarZumbis(&listaZumbis);

    while (listaBosses != NULL) {
        Boss *temp = listaBosses;
        listaBosses = listaBosses->proximo;
        free(temp);
    }
    while (listaBalas != NULL) {
        Bala *temp = listaBalas;
        listaBalas = listaBalas->proximo;
        free(temp);
    }

    descarregarRecursos(recursos);
    destruirRecursos(recursos);

    destruirMapa(mapaAtual);

    UnloadTexture(logoTexture);
    UnloadTexture(backgroundTexture);

    CloseWindow();

    return 0;
}
