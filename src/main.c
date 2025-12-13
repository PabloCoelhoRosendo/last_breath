#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"
#include "mapa.h"
#include "recursos.h"
#include "pathfinding.h"
#include "boss.h"
#include "item.h"
#include "horda.h"
#include "loja.h"
#include "menina.h"

// Função helper para spawnar menina próxima ao jogador sem colisão
Vector2 gerarPosicaoValidaProximaAoJogador(const Mapa* mapa, Vector2 posJogador, float raio) {
    // Tentar diferentes offsets ao redor do jogador
    Vector2 offsets[] = {
        {50, 0},    // Direita
        {-50, 0},   // Esquerda
        {0, -50},   // Cima
        {0, 50},    // Baixo
        {50, -50},  // Diagonal superior direita
        {-50, -50}, // Diagonal superior esquerda
        {50, 50},   // Diagonal inferior direita
        {-50, 50}   // Diagonal inferior esquerda
    };
    
    for (int i = 0; i < 8; i++) {
        Vector2 posicaoTeste = {
            posJogador.x + offsets[i].x,
            posJogador.y + offsets[i].y
        };
        
        // Verificar se está dentro dos limites do mapa
        if (posicaoTeste.x < 32 || posicaoTeste.x > 992 ||
            posicaoTeste.y < 32 || posicaoTeste.y > 736) {
            continue;
        }
        
        // Verificar se não colide com obstáculos
        if (!verificarColisaoMapa(mapa, posicaoTeste, raio)) {
            return posicaoTeste;
        }
    }
    
    // Se nenhuma posição próxima for válida, usar gerarPosicaoValidaSpawn
    return gerarPosicaoValidaSpawn(mapa, raio);
}

void detectarPortaNoMapa(Mapa* mapa, Porta* portaPtr, int faseAtual) {
    portaPtr->ativa = false;

    // Fase 1: Porta fixa no lado direito do depósito (posição hardcoded)
    if (faseAtual == 1) {
        // Porta na coluna 31, entre linhas 12-13 (pixels: x=992, y≈400)
        Vector2 posPorta = {992, 400};
        criarPorta(portaPtr, posPorta, 2);
        portaPtr->largura = 64.0f;
        portaPtr->altura = 96.0f;
        portaPtr->trancada = false;  // Porta do mercado sempre aberta
        printf("Porta do Mercado criada na posicao fixa (%.0f, %.0f) [Area: %.0fx%.0f]\n",
               posPorta.x, posPorta.y, portaPtr->largura, portaPtr->altura);
        return;
    }

    for (int i = 0; i < mapa->altura; i++) {
        for (int j = 0; j < mapa->largura; j++) {
            if (mapa->tiles[i][j] == TILE_PORTA_MERCADO && faseAtual == 1) {
                // Na fase 1, tile 10 leva ao mercado (fase 2)
                Vector2 posPorta = {(j * 32) + 16, (i * 32) + 16};
                criarPorta(portaPtr, posPorta, 2);
                printf("Porta do Mercado encontrada no mapa em tile (%d, %d) -> posicao (%.0f, %.0f)\n",
                       i, j, posPorta.x, posPorta.y);
                return;
            } else if (mapa->tiles[i][j] == TILE_PORTA_MERCADO && faseAtual == 2 && j < 15) {
                // Na fase 2, tile 10 no lado esquerdo retorna à loja (fase 1)
                Vector2 posPorta = {(j * 32) + 16, (i * 32) + 16};
                criarPorta(portaPtr, posPorta, 1);
                printf("Porta de RETORNO à loja encontrada no mapa em tile (%d, %d) -> posicao (%.0f, %.0f)\n",
                       i, j, posPorta.x, posPorta.y);
                return;
            } else if (mapa->tiles[i][j] == TILE_PORTA_LAB && faseAtual == 2 && j > 15) {
                // Na fase 2, tile 11 no lado direito leva à rua (fase 3)
                Vector2 posPorta = {(j * 32) + 16, (i * 32) + 16};
                criarPorta(portaPtr, posPorta, 3);
                printf("Porta para RUA encontrada no mapa em tile (%d, %d) -> posicao (%.0f, %.0f)\n",
                       i, j, posPorta.x, posPorta.y);
                return;
            } else if (mapa->tiles[i][j] == TILE_PORTA_LAB && faseAtual == 3) {
                // Na fase 3, tile 11 leva ao laboratório (fase 4)
                Vector2 posPorta = {(j * 32) + 16, (i * 32) + 16};
                criarPorta(portaPtr, posPorta, 4);
                portaPtr->largura = 100.0f;
                portaPtr->altura = 80.0f;
                printf("Porta do Laboratorio encontrada no mapa em tile (%d, %d) -> posicao (%.0f, %.0f) [Area: %.0fx%.0f]\n",
                       i, j, posPorta.x, posPorta.y, portaPtr->largura, portaPtr->altura);
                return;
            }
        }
    }

    printf("Aviso: Nenhuma porta encontrada no mapa atual (fase %d)\n", faseAtual);
}

int main(void) {
    const int larguraTela = 1024;
    const int alturaTela = 768;

    InitWindow(larguraTela, alturaTela, "Last Breath - Zumbi Survival Game");
    
    // Inicializar sistema de áudio ANTES de SetTargetFPS
    InitAudioDevice();
    
    if (!IsAudioDeviceReady()) {
        printf("ERRO CRITICO: Dispositivo de audio nao inicializado!\n");
    } else {
        printf("=== AUDIO INICIALIZADO COM SUCESSO ===\n");
        SetMasterVolume(1.0f);  // Volume máximo
    }
    
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
    Texture2D spriteCostasEsquerda = recursos->jogadorTrasCostasEsquerda;

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

    Texture2D periclesFrente = recursos->bosses[3][0];
    Texture2D periclesCostas = recursos->bosses[3][1];
    Texture2D periclesDireita = recursos->bosses[3][3];
    Texture2D periclesEsquerda = recursos->bosses[3][2];

    Player jogador;
    Zumbi *listaZumbis = NULL;
    Boss *listaBosses = NULL;
    Bala *listaBalas = NULL;
    Cartucho *listaCartuchos = NULL;
    ManchaSangue *listaManchas = NULL;
    Moeda *listaMoedas = NULL;

    Item itemProgresso;
    itemProgresso.ativo = false;
    Item itemArma;
    itemArma.ativo = false;

    Loja loja;
    loja.ativo = false;

    PathfindingGrid pathfindingGrid;
    inicializarPathfinding(&pathfindingGrid);
    printf("Sistema de Pathfinding A* inicializado!\n");

    Porta porta;
    porta.ativa = false;
    
    Porta portaRetorno;  // Porta de retorno à loja (na fase 2)
    portaRetorno.ativa = false;
    
    Porta portaRetorno2;  // Porta de retorno ao mercado (na fase 3)
    portaRetorno2.ativa = false;
    
    Porta portaRetorno3;  // Porta de retorno à rua (na fase 4)
    portaRetorno3.ativa = false;
    
    Porta portaFinal;  // Porta do final (BAD ENDING)
    portaFinal.ativa = false;
    
    Porta portaBanheiro;  // Porta secreta do banheiro (HAPPY ENDING)
    portaBanheiro.ativa = false;
    portaBanheiro.trancada = true;  // Começa trancada
    
    Menina menina;
    inicializarMenina(&menina);
    
    Escrivaninha escrivaninha;
    escrivaninha.ativa = false;
    
    Zumbi *zumbiBanheiro = NULL;  // Zumbi que ataca a menina

    // Inicializar jogador antes de detectar porta
    iniciarJogo(&jogador);

    // Carregar mapa baseado na fase inicial do jogador
    if (jogador.fase == 2) {
        carregarMapaDeArquivo(mapaAtual, "assets/maps/fase2.txt");
    } else if (jogador.fase == 3) {
        carregarMapaDeArquivo(mapaAtual, "assets/maps/fase3.txt");
    } else if (jogador.fase == 4) {
        carregarMapaDeArquivo(mapaAtual, "assets/maps/fase4.txt");
    }
    // Fase 1 já foi carregada por padrão no início

    detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);

    // Se iniciando em fase já concluída, destrancar portas
    if (jogador.fase == 2 && jogador.fase2Concluida && porta.ativa) {
        porta.trancada = false;
    } else if (jogador.fase == 3 && jogador.fase3Concluida && porta.ativa) {
        porta.trancada = false;
    } else if (jogador.fase == 1 && porta.ativa) {
        porta.trancada = false;  // Porta do mercado sempre aberta
    }

    jogador.estadoJogo = ESTADO_MENU;
    bool jogoIniciado = false;

    jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);

    jogador.spriteAtual = spriteFrenteDireita;
    
    // Controle de música
    Music *musicaAtual = NULL;
    bool musicaMenuTocando = false;

    while (!WindowShouldClose()) {
        // Atualizar stream de música
        if (musicaAtual != NULL) {
            UpdateMusicStream(*musicaAtual);
        }
        
        if (jogador.estadoJogo == ESTADO_MENU) {
            // Iniciar música do menu se ainda não estiver tocando
            if (!musicaMenuTocando && recursos->musicMenu.frameCount > 0) {
                if (musicaAtual != NULL) {
                    StopMusicStream(*musicaAtual);
                }
                musicaAtual = &recursos->musicMenu;
                PlayMusicStream(*musicaAtual);
                SetMusicVolume(*musicaAtual, 0.35f);
                musicaMenuTocando = true;
            }
            
            Rectangle botaoJogar = {
                larguraTela / 2 - 100,
                alturaTela / 2 + 150,
                200,
                60
            };

            Vector2 mousePos = GetMousePosition();
            bool mouseNoBot = CheckCollisionPointRec(mousePos, botaoJogar);

            if (mouseNoBot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                jogador.estadoJogo = ESTADO_INTRODUCAO;
                printf("Mostrando introdução...\n");
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

        // ========== TELA DE INTRODUÇÃO ==========
        if (jogador.estadoJogo == ESTADO_INTRODUCAO) {
            // Permitir pular a introdução com qualquer tecla
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (!jogoIniciado) {
                    iniciarJogo(&jogador);

                    if (jogador.fase == 4) {
                        jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){16 * 32 + 16, 21 * 32 + 16}, 15.0f);
                        printf("Spawn inicial Fase 4: (linha=21, coluna=16 - meio) -> (%.0f, %.0f)\n",
                               jogador.posicao.x, jogador.posicao.y);
                    } else {
                        jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
                    }

                    jogador.spriteAtual = spriteFrenteDireita;
                    jogoIniciado = true;

                    // Inicializar loja na fase 1
                    if (jogador.fase == 1) {
                        inicializarLoja(&loja, &jogador);
                    }
                }
                jogador.estadoJogo = ESTADO_JOGANDO;
                printf("Jogo iniciado!\n");
                
                // Trocar para música de gameplay
                if (recursos->musicGameplay.frameCount > 0) {
                    StopMusicStream(recursos->musicMenu);
                    musicaAtual = &recursos->musicGameplay;
                    PlayMusicStream(*musicaAtual);
                    SetMusicVolume(*musicaAtual, 0.15f);
                    musicaMenuTocando = false;
                }
            }

            BeginDrawing();
            ClearBackground(BLACK);

            // Desenhar documento introdutório se disponível
            if (recursos->documentoIntro.id > 0) {
                // Calcular escala para preencher a tela mantendo proporção
                float escalaX = (float)GetScreenWidth() / recursos->documentoIntro.width;
                float escalaY = (float)GetScreenHeight() / recursos->documentoIntro.height;
                float escala = (escalaX < escalaY) ? escalaX : escalaY;

                float largura = recursos->documentoIntro.width * escala * 0.9f;
                float altura = recursos->documentoIntro.height * escala * 0.9f;

                Rectangle destino = {
                    (GetScreenWidth() - largura) / 2,
                    (GetScreenHeight() - altura) / 2,
                    largura,
                    altura
                };

                Rectangle origem = {
                    0, 0,
                    (float)recursos->documentoIntro.width,
                    (float)recursos->documentoIntro.height
                };

                DrawTexturePro(recursos->documentoIntro, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                // Fallback: mostrar texto se a imagem não carregar
                const char* titulo = "DIARIO PESSOAL";
                const char* linhas[] = {
                    "Dia 127 apos o surto...",
                    "",
                    "Eu achava que era o ultimo sobrevivente na Terra.",
                    "Vaguei por meses entre ruinas e mortos-vivos,",
                    "buscando qualquer sinal de vida.",
                    "",
                    "Hoje, finalmente encontrei algo...",
                    "Um laboratorio abandonado.",
                    "",
                    "Mas a verdade que descobri la dentro",
                    "mudou tudo que eu pensava sobre mim.",
                    "",
                    "Eu nao sou humano. Sou um robo.",
                    "E minha missao verdadeira:",
                    "Salvar a filha do cientista.",
                    "",
                    "Ela e a ULTIMA sobrevivente humana.",
                    "E eu sou a unica esperanca dela.",
                    ""
                };

                int startY = 150;
                DrawText(titulo, GetScreenWidth() / 2 - MeasureText(titulo, 32) / 2, startY, 32, GOLD);
                
                for (int i = 0; i < sizeof(linhas) / sizeof(linhas[0]); i++) {
                    int larguraLinha = MeasureText(linhas[i], 20);
                    DrawText(linhas[i], GetScreenWidth() / 2 - larguraLinha / 2, startY + 60 + (i * 25), 20, WHITE);
                }
            }

            // Instrução para continuar
            DrawText("Pressione ESPACO ou ENTER para continuar", 
                     GetScreenWidth() / 2 - 220, GetScreenHeight() - 50, 20, YELLOW);

            EndDrawing();
            continue;
        }

        if ((jogador.vida <= 0 || jogador.jogoVencido) && IsKeyPressed(KEY_R)) {
            liberarZumbis(&listaZumbis);
            listaZumbis = NULL;
            
            liberarMoedas(&listaMoedas);
            listaMoedas = NULL;

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
            
            liberarCartuchos(&listaCartuchos);
            liberarManchasSangue(&listaManchas);
            
            // Liberar zumbi do banheiro
            liberarZumbis(&zumbiBanheiro);
            zumbiBanheiro = NULL;

            // Resetar menina
            menina.ativa = false;
            menina.seguindo = false;
            menina.posicao = (Vector2){0, 0};

            if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
                inicializarMapaPadrao(mapaAtual);
            }

            itemProgresso.ativo = false;
            itemArma.ativo = false;
            criarPorta(&porta, (Vector2){960, 384}, 2);
            
            // Resetar portas
            portaBanheiro.ativa = false;
            portaBanheiro.trancada = true;
            portaRetorno.ativa = false;
            portaRetorno2.ativa = false;
            portaRetorno3.ativa = false;
            portaFinal.ativa = false;
            
            // Resetar loja
            loja.ativo = false;
            loja.menuAberto = false;
            
            // Resetar escrivaninha
            escrivaninha.ativa = false;
            escrivaninha.lida = false;
            
            // Voltar textura para a loja
            texturaMapa = recursos->fundoMapa;
            
            // Resetar moedas do jogador
            jogador.moedas = 0;
            jogador.moedasColetadas = 0;

            jogador.estadoJogo = ESTADO_MENU;
            jogoIniciado = false;

            printf("Voltando ao menu principal...\n");
            
            // Voltar para música do menu
            if (musicaAtual != NULL) {
                StopMusicStream(*musicaAtual);
            }
            musicaMenuTocando = false;
            musicaAtual = NULL;
        }

        Vector2 posicaoAnteriorJogador = jogador.posicao;

        // Atualizar loja na fase 1 (sempre atualiza para permitir abrir/fechar)
        bool jogoPausado = false;
        if (jogador.fase == 1 && loja.ativo) {
            atualizarLoja(&loja, &jogador, mapaAtual, recursos);
            jogoPausado = loja.menuAberto;  // Pausa se menu estiver aberto
            
            // Porta do banheiro sempre existe na fase 1 - detectar pelo tile 13
            if (!portaBanheiro.ativa) {
                for (int i = 0; i < mapaAtual->altura; i++) {
                    for (int j = 0; j < mapaAtual->largura; j++) {
                        if (mapaAtual->tiles[i][j] == TILE_PORTA_BANHEIRO) {
                            Vector2 posBanheiro = {(j * 32) + 16, (i * 32) + 16};
                            portaBanheiro.ativa = true;
                            portaBanheiro.posicao = posBanheiro;
                            portaBanheiro.largura = 64.0f;  // 2 tiles de largura
                            portaBanheiro.altura = 32.0f;
                            portaBanheiro.trancada = true;  // Começa trancada
                            printf("Porta do banheiro encontrada em tile (%d, %d) -> posicao (%.0f, %.0f)\n",
                                   i, j, posBanheiro.x, posBanheiro.y);
                            break;
                        }
                    }
                    if (portaBanheiro.ativa) break;
                }
            }
        }
        
        // Só atualiza o jogo se não estiver pausado pela loja
        if (!jogoPausado) {
            atualizarJogoComPathfinding(&jogador, &listaZumbis, &listaBalas, mapaAtual, &pathfindingGrid, &listaCartuchos, &listaManchas, recursos);
            
            // Atualiza cartuchos
            atualizarCartuchos(&listaCartuchos, GetFrameTime());
            
            // Atualiza manchas de sangue (fade out)
            atualizarManchasSangue(&listaManchas, GetFrameTime());
            
            // Verifica coleta de moedas
            verificarColetaMoedas(&listaMoedas, &jogador, recursos);
            
            // Atualizar menina se estiver seguindo
            if (menina.ativa && menina.seguindo) {
                atualizarMenina(&menina, &jogador, mapaAtual, GetFrameTime(), &listaZumbis, &listaBalas, recursos);
            }
        }
        
        // Modo Deus (F1) - funciona mesmo com jogo pausado
        if (IsKeyPressed(KEY_F1)) {
            jogador.modoDeus = !jogador.modoDeus;
            printf("Modo Deus: %s\n", jogador.modoDeus ? "ATIVADO" : "DESATIVADO");
        }
        
        // Interagir com escrivaninha na fase 4
        if (jogador.fase == 4 && escrivaninha.ativa) {
            if (verificarInteracaoEscrivaninha(&escrivaninha, &jogador)) {
                printf("Você leu o relatório!\n");
                // Ativar porta de retorno à rua após ler o relatório
                if (!portaRetorno3.ativa) {
                    portaRetorno3.ativa = true;
                    printf("Uma porta para a RUA foi destrancada!\n");
                }
                // Destrancar porta do banheiro após ler o relatório
                if (portaBanheiro.trancada) {
                    portaBanheiro.trancada = false;
                    printf("A porta do BANHEIRO foi destrancada!\n");
                }
                // Destrancar porta final após ler o relatório
                if (portaFinal.trancada) {
                    portaFinal.trancada = false;
                    printf("A porta de SAÍDA foi destrancada!\n");
                }
            }
            // Fechar relatório com tecla F
            if (escrivaninha.lida && IsKeyPressed(KEY_F)) {
                escrivaninha.lida = false;
            }
        }

        // Só atualiza hordas se jogo não estiver pausado E se a menina NÃO estiver seguindo
        if (!jogoPausado && (jogador.fase == 2 || jogador.fase == 3) && !menina.seguindo) {
            atualizarHorda(&jogador, &listaZumbis, &listaBosses, GetFrameTime());
            
            // Ativar porta de retorno quando horda completa
            if (jogador.estadoHorda == HORDA_COMPLETA && jogador.hordaAtual == 3) {
                if (jogador.fase == 3) {
                    portaRetorno2.ativa = true;
                }
            }

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
                    Vector2 spawnBoss = gerarPosicaoValidaSpawn(mapaAtual, 25.0f);

                    if (jogador.fase == 2) {
                        criarBoss(&listaBosses, BOSS_PROWLER, spawnBoss, prowlerFrente, prowlerCostas, prowlerDireita, prowlerEsquerda);
                        printf("PROWLER spawnado! (%d/%d)\n", jogador.bossesSpawnados + 1, jogador.bossesTotaisHorda);
                    } else if (jogador.fase == 3) {
                        criarBoss(&listaBosses, BOSS_HUNTER, spawnBoss, hunterFrente, hunterCostas, hunterDireita, hunterEsquerda);
                        printf("HUNTER spawnado! (%d/%d)\n", jogador.bossesSpawnados + 1, jogador.bossesTotaisHorda);
                    }

                    jogador.bossesSpawnados++;
                    jogador.tempoSpawnBoss = 0.0f;
                }
            }
        }

        if (!jogoPausado) {
            atualizarBalas(&listaBalas, mapaAtual);

            if (verificarColisaoMapa(mapaAtual, jogador.posicao, 15.0f)) {
                jogador.posicao = posicaoAnteriorJogador;
            }

            // Colisão com escrivaninha na fase 4
            if (jogador.fase == 4 && escrivaninha.ativa) {
                if (verificarColisaoCirculos(jogador.posicao, 15.0f, escrivaninha.posicao, 50.0f)) {
                    jogador.posicao = posicaoAnteriorJogador;
                }
            }

            // Colisão com porta final na fase 4
            if (jogador.fase == 4 && portaFinal.ativa) {
                if (verificarColisaoCirculos(jogador.posicao, 15.0f, portaFinal.posicao, 30.0f)) {
                    jogador.posicao = posicaoAnteriorJogador;
                }
            }
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
                // Loja - sem boss
            }
            else if (jogador.fase == 2) {
                // Boss controlado pelo sistema de hordas
            }
            else if (jogador.fase == 3) {
                // Boss controlado pelo sistema de hordas
            }
            else if (jogador.fase == 4) {
                // Boss final só spawna se ainda NÃO foi morto
                if (!jogador.matouBossFinal) {
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
                        
                        // Trocar para música de boss fight
                        if (recursos->musicBossFight.frameCount > 0 && musicaAtual != &recursos->musicBossFight) {
                            if (musicaAtual != NULL) {
                                StopMusicStream(*musicaAtual);
                            }
                            musicaAtual = &recursos->musicBossFight;
                            PlayMusicStream(*musicaAtual);
                            SetMusicVolume(*musicaAtual, 0.45f);
                        }
                    }
                }
            }
        }

        atualizarZumbisComPathfinding(&listaZumbis, jogador.posicao, GetFrameTime(), mapaAtual, &pathfindingGrid);
        atualizarBossComPathfinding(&listaBosses, &jogador, &listaBalas, GetFrameTime(), mapaAtual, &pathfindingGrid, recursos);
        
        // Spawnar 5 zumbis normais GRADATIVAMENTE no caminho de volta após buscar a menina
        // Spawn: 1 zumbi a cada 1 segundo
        // APENAS NO MERCADO (FASE 2) E NA RUA (FASE 3)
        if (menina.seguindo && jogador.vida > 0) {
            bool deveSpawnar = false;
            int maxZumbis = 5;
            
            // APENAS SPAWNA NO MERCADO (FASE 2) E NA RUA (FASE 3)
            // Se está no mercado e a fase foi concluída (está voltando)
            if (jogador.fase == 2 && jogador.fase2Concluida && !jogador.spawnadoRetornoFase2) {
                if (jogador.zumbisSpawnadosRetorno == 0) {
                    printf("=== INICIO SPAWN: MERCADO (Fase 2) com menina ===\n");
                }
                deveSpawnar = true;
            }
            // Se está na rua e a fase foi concluída (está voltando)
            else if (jogador.fase == 3 && jogador.fase3Concluida && !jogador.spawnadoRetornoFase3) {
                if (jogador.zumbisSpawnadosRetorno == 0) {
                    printf("=== INICIO SPAWN: RUA (Fase 3) com menina ===\n");
                }
                deveSpawnar = true;
                maxZumbis = 7; // Apenas 7 zumbis na fase da rua com a menina
            }
            
            if (deveSpawnar && jogador.zumbisSpawnadosRetorno < maxZumbis) {
                jogador.tempoSpawnRetorno += GetFrameTime();
                
                if (jogador.tempoSpawnRetorno >= 1.0f) { // Spawn a cada 1 segundo (mesma velocidade que todos)
                    Vector2 posSpawn = gerarPosicaoValidaSpawn(mapaAtual, 20.0f);
                    adicionarZumbi(&listaZumbis, posSpawn, recursos->zumbis);
                    jogador.zumbisSpawnadosRetorno++;
                    jogador.tempoSpawnRetorno = 0.0f;
                    
                    printf("Zumbi %d/%d spawnado no caminho de volta (Fase %d)\n", 
                           jogador.zumbisSpawnadosRetorno, maxZumbis, jogador.fase);
                    
                    // Marcar como completo quando spawnar todos
                    if (jogador.zumbisSpawnadosRetorno >= maxZumbis) {
                        if (jogador.fase == 2) {
                            jogador.spawnadoRetornoFase2 = true;
                            printf("=== TODOS OS 5 ZUMBIS DO MERCADO SPAWNADOS! ===\n");
                        } else if (jogador.fase == 3) {
                            jogador.spawnadoRetornoFase3 = true;
                            printf("=== TODOS OS 7 ZUMBIS DA RUA SPAWNADOS! ===\n");
                        }
                    }
                }
            }
        }
        
        // Atualizar zumbi do banheiro se existir
        if (zumbiBanheiro != NULL) {
            atualizarZumbisComPathfinding(&zumbiBanheiro, jogador.posicao, GetFrameTime(), mapaAtual, &pathfindingGrid);
        }

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

        verificarColisoesBossBala(&listaBosses, &listaBalas, &itemProgresso, &itemArma, &jogador, &listaMoedas, recursos);
        
        // Verificar se o boss morreu e voltar para música de gameplay
        if (jogador.bossSpawnado && listaBosses == NULL && musicaAtual == &recursos->musicBossFight) {
            if (recursos->musicGameplay.frameCount > 0) {
                StopMusicStream(recursos->musicBossFight);
                musicaAtual = &recursos->musicGameplay;
                PlayMusicStream(*musicaAtual);
                SetMusicVolume(*musicaAtual, 0.4f);
            }
        }
        
        verificarColisoesBossJogador(listaBosses, &jogador);
        verificarColisoesBalaZumbi(&listaBalas, &listaZumbis, &jogador, &listaManchas, &listaMoedas, recursos);
        
        // Verificar colisões com zumbi do banheiro
        if (zumbiBanheiro != NULL) {
            verificarColisoesBalaZumbi(&listaBalas, &zumbiBanheiro, &jogador, &listaManchas, &listaMoedas, recursos);
        }
        
        // Lógica da porta do banheiro (HAPPY ENDING PATH) - leva a outro mapa
        if (jogador.fase == 1 && portaBanheiro.ativa && !jogador.conheceuMenina && !portaBanheiro.trancada) {
            float distPorta = sqrtf(
                (jogador.posicao.x - portaBanheiro.posicao.x) * (jogador.posicao.x - portaBanheiro.posicao.x) +
                (jogador.posicao.y - portaBanheiro.posicao.y) * (jogador.posicao.y - portaBanheiro.posicao.y)
            );
            
            // Só permite entrar se não estiver trancada E apertar E
            if (distPorta <= 50.0f && IsKeyPressed(KEY_E) && !loja.menuAberto) {
                // Carregar mapa do banheiro
                if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/banheiro.txt")) {
                    printf("Erro: Não foi possível carregar banheiro.txt\n");
                }
                
                // Tocar grito da garota
                if (recursos->sfxGarotaGrito.frameCount > 0) {
                    PlaySound(recursos->sfxGarotaGrito);
                }
                
                // Marcar que está no banheiro
                jogador.estaNoBanheiro = true;
                
                // Mudar textura de fundo para chão do laboratório (azulejo)
                texturaMapa = recursos->chaoLab;
                
                // Desativar loja no banheiro
                loja.ativo = false;
                
                // Spawnar menina no banheiro
                // Menina no lado esquerdo (walkable)
                menina.posicao = (Vector2){7 * 32, 17 * 32};  // Coluna 7, linha 17
                menina.ativa = true;
                menina.seguindo = false;
                jogador.conheceuMenina = true;
                carregarSpritesMenina(&menina, recursos);  // Carregar sprites

                // Boss Pericles spawna no centro do banheiro
                if (!jogador.periclesSpawnado) {
                    Vector2 spawnPericles = {12 * 32, 12 * 32};  // Centro do mapa
                    criarBoss(&listaBosses, BOSS_PERICLES, spawnPericles,
                             periclesFrente, periclesCostas, periclesDireita, periclesEsquerda);
                    jogador.periclesSpawnado = true;
                    printf("Boss Pericles apareceu no banheiro!\n");
                }

                // Jogador spawna onde a menina estava antes (centro-direita)
                // Coluna 20, linha 10
                jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){20 * 32, 10 * 32}, 15.0f);

                printf("Você entrou no banheiro! Há uma menina e o Boss Pericles!\n");
            }
        }
        
        // Verificar se matou o zumbi do banheiro e liberar a menina
        if (jogador.estaNoBanheiro && jogador.conheceuMenina && !jogador.meninaLiberada && zumbiBanheiro == NULL && menina.ativa) {
            float distMenina = sqrtf(
                (jogador.posicao.x - menina.posicao.x) * (jogador.posicao.x - menina.posicao.x) +
                (jogador.posicao.y - menina.posicao.y) * (jogador.posicao.y - menina.posicao.y)
            );
            
            if (distMenina <= 40.0f && IsKeyPressed(KEY_E)) {
                menina.seguindo = true;
                jogador.meninaLiberada = true;
                printf("A menina agora está seguindo você! Leve-a à porta final!\n");
            }
        }
        
        // Porta de saída do banheiro - volta pra loja
        if (jogador.estaNoBanheiro && jogador.conheceuMenina && jogador.meninaLiberada) {
            Vector2 portaSaidaBanheiro = {512, 700};  // Porta inferior
            float distPortaSaida = sqrtf(
                (jogador.posicao.x - portaSaidaBanheiro.x) * (jogador.posicao.x - portaSaidaBanheiro.x) +
                (jogador.posicao.y - portaSaidaBanheiro.y) * (jogador.posicao.y - portaSaidaBanheiro.y)
            );
            
            if (distPortaSaida <= 50.0f && IsKeyPressed(KEY_E)) {
                // Voltar pra loja
                if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
                    printf("Erro ao carregar fase1.txt\n");
                }
                
                // Marcar que saiu do banheiro e que completou todas as fases
                jogador.estaNoBanheiro = false;
                jogador.periclesSpawnado = false;
                jogador.fase = 1;
                jogador.matouBossFinal = true; // Veio do laboratório
                jogador.fase2Concluida = true;  // Marca que passou pelo mercado
                jogador.fase3Concluida = true;  // Marca que passou pela rua
                
                // Resetar spawn para começar a spawnar zumbis gradativamente
                jogador.spawnadoRetornoFase2 = false;
                jogador.spawnadoRetornoFase3 = false;
                jogador.spawnadoRetornoFase4 = false;
                jogador.zumbisSpawnadosRetorno = 0;
                jogador.tempoSpawnRetorno = 0.0f;
                
                texturaMapa = recursos->fundoMapa;
                jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){100, 450}, 15.0f);
                if (menina.seguindo) {
                    menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                }
                loja.ativo = true;
                inicializarLoja(&loja, &jogador);
                detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);

                printf("Voltou para a loja com a menina! Prepare-se para zumbis!\n");
            }
        }
        
        // Porta de saída do banheiro - volta pra loja com a menina
        if (jogador.estaNoBanheiro && jogador.conheceuMenina && menina.seguindo) {
            // Detectar porta de saída do banheiro (tile 13)
            for (int i = 0; i < mapaAtual->altura; i++) {
                for (int j = 0; j < mapaAtual->largura; j++) {
                    if (mapaAtual->tiles[i][j] == TILE_PORTA_BANHEIRO) {
                        Vector2 posPortaSaida = {(j * 32) + 16, (i * 32) + 16};
                        float distPortaSaida = sqrtf(
                            (jogador.posicao.x - posPortaSaida.x) * (jogador.posicao.x - posPortaSaida.x) +
                            (jogador.posicao.y - posPortaSaida.y) * (jogador.posicao.y - posPortaSaida.y)
                        );
                        
                        if (distPortaSaida <= 50.0f && IsKeyPressed(KEY_E)) {
                            // Voltar pra loja
                            if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
                                printf("Erro: Não foi possível carregar fase1.txt\n");
                            }
                            
                            // Marcar que saiu do banheiro
                            jogador.estaNoBanheiro = false;
                            jogador.periclesSpawnado = false;

                            // Voltar textura da loja
                            texturaMapa = recursos->fundoMapa;

                            jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){200, 400}, 15.0f);
                            if (menina.seguindo) {
                                menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                            }
                            
                            // Reativar loja
                            inicializarLoja(&loja, &jogador);
                            detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);

                            printf("Você voltou à loja com a menina!\n");
                            goto fim_loop_porta_banheiro;
                        }
                    }
                }
            }
            fim_loop_porta_banheiro:;
        }
        
        // Verificar HAPPY ENDING - menina chegou na porta final
        if (jogador.fase == 4 && jogador.meninaLiberada && portaFinal.ativa && !portaFinal.trancada) {
            float distMeninaPorta = sqrtf(
                (menina.posicao.x - portaFinal.posicao.x) * (menina.posicao.x - portaFinal.posicao.x) +
                (menina.posicao.y - portaFinal.posicao.y) * (menina.posicao.y - portaFinal.posicao.y)
            );

            if (distMeninaPorta <= 60.0f) {
                jogador.finalFeliz = true;
                jogador.jogoVencido = true;
                printf("=== HAPPY ENDING! Você salvou a menina! ===\n");
            }
        }

        // Verificar interação com porta final
        if (jogador.fase == 4 && portaFinal.ativa && !portaFinal.trancada) {
            float distJogadorPortaFinal = sqrtf(
                (jogador.posicao.x - portaFinal.posicao.x) * (jogador.posicao.x - portaFinal.posicao.x) +
                (jogador.posicao.y - portaFinal.posicao.y) * (jogador.posicao.y - portaFinal.posicao.y)
            );

            if (distJogadorPortaFinal <= 80.0f && IsKeyPressed(KEY_E)) {
                jogador.jogoVencido = true;
                if (jogador.meninaLiberada) {
                    jogador.finalFeliz = true;
                    printf("=== HAPPY ENDING! Você salvou a menina! ===\n");
                } else {
                    jogador.finalFeliz = false;
                    printf("=== BAD ENDING: Você escapou sozinho... ===\n");
                }
            }
        }

        if (itemProgresso.ativo) {
            verificarColetaItem(&itemProgresso, &jogador);
        }
        if (itemArma.ativo) {
            verificarColetaItem(&itemArma, &jogador);
        }

        // Não verificar porta principal se estiver perto da porta do banheiro na fase 1
        bool pertoDaPortaBanheiro = false;
        bool transicaoRealizadaNesteFrame = false;  // Flag para evitar dupla transição no mesmo frame
        if (jogador.fase == 1 && portaBanheiro.ativa) {
            float distBanheiro = sqrtf(
                (jogador.posicao.x - portaBanheiro.posicao.x) * (jogador.posicao.x - portaBanheiro.posicao.x) +
                (jogador.posicao.y - portaBanheiro.posicao.y) * (jogador.posicao.y - portaBanheiro.posicao.y)
            );
            pertoDaPortaBanheiro = (distBanheiro <= 80.0f);
        }

        if (jogador.fase != 1 && porta.ativa && !pertoDaPortaBanheiro && verificarInteracaoPorta(&porta, &jogador, recursos)) {
            printf("Usando porta! Indo para Fase %d\n", porta.faseDestino);

            // Marcar fase anterior como concluída ao avançar
            if (porta.faseDestino == 3) {
                jogador.fase2Concluida = true; // Completou mercado, indo para rua
                printf("Fase 2 (MERCADO) marcada como concluída!\n");
            } else if (porta.faseDestino == 4) {
                jogador.fase3Concluida = true; // Completou rua, indo para laboratório
                printf("Fase 3 (RUA) marcada como concluída!\n");
                transicaoRealizadaNesteFrame = true;  // Evitar que portaRetorno3 seja ativada no mesmo frame
            }

            jogador.fase = porta.faseDestino;
            jogador.timerBoss = 0.0f;
            jogador.bossSpawnado = false;

            printf("Limpando zumbis da fase anterior...\n");
            liberarZumbis(&listaZumbis);
            listaZumbis = NULL;
            
            printf("Limpando moedas da fase anterior...\n");
            liberarMoedas(&listaMoedas);
            listaMoedas = NULL;

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
            
            // Não resetar timer e flag de boss na fase 4 se já foi morto
            if (jogador.fase != 4 || !jogador.matouBossFinal) {
                jogador.timerBoss = 0.0f;
                jogador.bossSpawnado = false;
            }

            char caminhoMapa[64];
            snprintf(caminhoMapa, sizeof(caminhoMapa), "assets/maps/fase%d.txt", jogador.fase);
            if (!carregarMapaDeArquivo(mapaAtual, caminhoMapa)) {
                printf("Aviso: Nao foi possivel carregar %s. Usando mapa padrao.\n", caminhoMapa);
                inicializarMapaPadrao(mapaAtual);
            }

            if (jogador.fase == 2) {
                // Detectar porta de retorno à loja (tile 10 no centro-esquerda)
                for (int i = 0; i < mapaAtual->altura; i++) {
                    for (int j = 0; j < mapaAtual->largura; j++) {
                        if (mapaAtual->tiles[i][j] == TILE_PORTA_MERCADO && j == 0) {
                            Vector2 posPortaRetorno = {(j * 32) + 16, (i * 32) + 16};
                            criarPorta(&portaRetorno, posPortaRetorno, 1);  // Volta pra loja
                            portaRetorno.ativa = true;  // Ativar porta de retorno
                            printf("Porta de retorno à loja ATIVADA em (%d, %d)\n", i, j);
                            break;
                        }
                    }
                }

                // Detectar porta principal para rua (tile 11 no lado direito)
                detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);
                
                // Se a fase já foi concluída, destrancar a porta
                if (jogador.fase2Concluida && porta.ativa) {
                    porta.trancada = false;
                    printf("Porta para RUA destrancada (fase já concluída)\n");
                }
                
                // Verificar se a fase já foi concluída
                if (jogador.fase2Concluida) {
                    jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){portaRetorno.posicao.x + 80, portaRetorno.posicao.y}, 15.0f);
                    if (menina.seguindo) {
                        menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                        // Com menina, resetar para spawnar zumbis gradativamente
                        jogador.estadoHorda = HORDA_EM_PROGRESSO;
                        jogador.spawnadoRetornoFase2 = false;
                        jogador.zumbisSpawnadosRetorno = 0;
                        jogador.tempoSpawnRetorno = 0.0f;
                        printf("Fase 2 com menina. Zumbis vão spawnar!\n");
                    } else {
                        jogador.estadoHorda = HORDA_COMPLETA;
                        printf("Fase 2 já concluída. Sem inimigos.\n");
                    }
                } else {
                    jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){8 * 32 + 16, 9 * 32 + 16}, 15.0f);
                    if (menina.seguindo) {
                        menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                    }
                    printf("Jogador spawnado na Fase 2 (MERCADO) em posicao fixa: (linha=9, coluna=8) -> (%.0f, %.0f)\n",
                           jogador.posicao.x, jogador.posicao.y);
                }
            } else if (jogador.fase == 3) {
                // Detectar porta de retorno ao mercado (tile 10 no mercado)
                bool portaRetornoEncontrada = false;
                for (int i = 0; i < mapaAtual->altura; i++) {
                    for (int j = 0; j < mapaAtual->largura; j++) {
                        if (mapaAtual->tiles[i][j] == TILE_PORTA_MERCADO) {
                            Vector2 posPortaRetorno2 = {(j * 32) + 16, (i * 32) + 16};
                            criarPorta(&portaRetorno2, posPortaRetorno2, 2);  // Volta pro mercado
                            printf("Porta de retorno ao mercado encontrada em (%d, %d) - Posicao: (%.0f, %.0f)\n", i, j, posPortaRetorno2.x, posPortaRetorno2.y);
                            portaRetornoEncontrada = true;
                            break;
                        }
                    }
                    if (portaRetornoEncontrada) break;
                }
                if (!portaRetornoEncontrada) {
                    printf("ERRO: Porta de retorno ao mercado NÃO foi encontrada!\n");
                }
                
                // Detectar porta principal para laboratório (tile 11)
                detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);
                
                // Se a fase já foi concluída, destrancar a porta
                if (jogador.fase3Concluida && porta.ativa) {
                    porta.trancada = false;
                    printf("Porta para LAB destrancada (fase já concluída)\n");
                }
                
                // Verificar se a fase já foi concluída
                if (jogador.fase3Concluida) {
                    // Spawnar ABAIXO da porta (na rua, não dentro do mercado)
                    jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){portaRetorno2.posicao.x, portaRetorno2.posicao.y + 80}, 15.0f);
                    if (menina.seguindo) {
                        menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                        // Com menina, resetar para spawnar zumbis gradativamente
                        jogador.estadoHorda = HORDA_EM_PROGRESSO;
                        jogador.spawnadoRetornoFase3 = false;
                        jogador.zumbisSpawnadosRetorno = 0;
                        jogador.tempoSpawnRetorno = 0.0f;
                        printf("Fase 3 com menina. Zumbis vão spawnar!\n");
                    } else {
                        jogador.estadoHorda = HORDA_COMPLETA;
                        printf("Fase 3 já concluída. Sem inimigos.\n");
                    }
                    portaRetorno2.ativa = true;  // Ativar porta de retorno
                } else {
                    jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){8 * 32 + 16, 9 * 32 + 16}, 15.0f);
                    if (menina.seguindo) {
                        menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                    }
                    printf("Jogador spawnado na Fase 3 (RUA) em posicao fixa: (linha=9, coluna=8) -> (%.0f, %.0f)\n",
                           jogador.posicao.x, jogador.posicao.y);
                    
                    // Iniciar horda na fase 3
                    iniciarHorda(&jogador, 1);
                    printf("Horda da RUA iniciada!\n");
                }
            } else if (jogador.fase == 4) {
                jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){16 * 32 + 16, 21 * 32 + 16}, 15.0f);
                if (menina.seguindo) {
                    menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                }
                printf("Jogador spawnado na Fase 4 (LAB) em posicao fixa: (linha=21, coluna=16 - meio) -> (%.0f, %.0f)\n",
                       jogador.posicao.x, jogador.posicao.y);
                
                // Detectar porta de retorno à RUA (tile 10 na parte inferior)
                for (int i = 0; i < mapaAtual->altura; i++) {
                    for (int j = 0; j < mapaAtual->largura; j++) {
                        if (mapaAtual->tiles[i][j] == TILE_PORTA_MERCADO) {
                            Vector2 posPortaRetorno3 = {(j * 32) + 16, (i * 32) + 16};
                            criarPorta(&portaRetorno3, posPortaRetorno3, 3);  // Volta pra rua
                            portaRetorno3.ativa = false;  // Só ativa após ler relatório
                            printf("Porta de retorno à RUA encontrada em (%d, %d)\n", i, j);
                            break;
                        }
                    }
                }
                
                // Criar escrivaninha com relatório no centro superior
                criarEscrivaninha(&escrivaninha, (Vector2){512, 175});

                // Criar porta final (BAD ENDING) ao lado da escrivaninha
                criarPorta(&portaFinal, (Vector2){680, 150}, 5);  // fase 5 = ending

                // Se já leu o relatório, manter tudo destrancado
                if (jogador.leuRelatorio) {
                    escrivaninha.trancada = false;
                    portaFinal.trancada = false;
                    portaRetorno3.ativa = true;
                } else {
                    portaFinal.trancada = true;  // Trancada até ler o relatório
                }
            } else {
                jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
                if (menina.seguindo) {
                    menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                }
            }

            if (jogador.fase == 3) {
                jogador.velocidadeBase = 5.0f;
                printf("UPGRADE! Velocidade aumentada para 5.0 m/s\n");
            }
            
            // Reinicializar loja ao voltar pra fase 1
            if (jogador.fase == 1) {
                inicializarLoja(&loja, &jogador);
                
                // Porta do banheiro sempre existe, mas pode estar trancada/destrancada
                if (!jogador.conheceuMenina) {
                    portaBanheiro.ativa = true;
                    portaBanheiro.posicao = (Vector2){100, 400};
                    portaBanheiro.largura = 50.0f;
                    portaBanheiro.altura = 60.0f;
                    portaBanheiro.trancada = !jogador.leuRelatorio;  // Trancada se não leu o relatório

                    if (jogador.leuRelatorio) {
                        printf("Porta do banheiro reaparece destrancada na loja!\n");
                    } else {
                        printf("Porta do banheiro está na loja (trancada).\n");
                    }
                }
            }
        }
        
        // Verificar interação com porta de retorno (volta pra loja da fase 2)
        if (jogador.fase == 2 && portaRetorno.ativa && verificarInteracaoPorta(&portaRetorno, &jogador, recursos)) {
            // Só permite voltar se a fase foi concluída
            if (jogador.estadoHorda != HORDA_COMPLETA) {
                printf("Complete a fase primeiro!\n");
            } else {
                printf("Voltando para a LOJA!\n");
                jogador.fase2Concluida = true; // Marca que a fase 2 foi concluída
                jogador.fase = 1;
                
                // Limpar inimigos
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
                liberarCartuchos(&listaCartuchos);
                liberarManchasSangue(&listaManchas);
                liberarMoedas(&listaMoedas);
                listaMoedas = NULL;
                
                // Carregar mapa da loja
                if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
                    printf("Erro ao carregar fase1.txt\n");
                }

                jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){512, 400}, 15.0f);
                if (menina.seguindo) {
                    menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                }
                inicializarLoja(&loja, &jogador);
                detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);

                portaRetorno.ativa = false;

                // Resetar spawn gradativo
                jogador.spawnadoRetornoFase2 = false;
                jogador.zumbisSpawnadosRetorno = 0;
                jogador.tempoSpawnRetorno = 0.0f;
            }

            jogador.fase = 1;

            // Limpar inimigos
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
            liberarCartuchos(&listaCartuchos);
            liberarManchasSangue(&listaManchas);
            liberarMoedas(&listaMoedas);
            listaMoedas = NULL;

            // Carregar mapa da loja
            if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
                printf("Erro ao carregar fase1.txt\n");
            }

            jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){512, 400}, 15.0f);
            if (menina.seguindo) {
                menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
            }
            inicializarLoja(&loja, &jogador);
            detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);

            portaRetorno.ativa = false;
        }
        
        // Verificar interação com porta de retorno (volta pro mercado da fase 3)
        if (jogador.fase == 3 && portaRetorno2.ativa) {
            float dx = jogador.posicao.x - portaRetorno2.posicao.x;
            float dy = jogador.posicao.y - portaRetorno2.posicao.y;
            float distancia = sqrtf(dx * dx + dy * dy);
            
            if (distancia <= 60.0f && IsKeyPressed(KEY_E)) {
                printf("Tecla E detectada! Dist=%.1f Estado=%d\n", distancia, jogador.estadoHorda);
                
                // Só permite voltar se a fase foi concluída
                if (jogador.estadoHorda != HORDA_COMPLETA) {
                    printf("Complete a fase primeiro!\n");
                } else {
                    printf("Voltando para o MERCADO!\n");
                    jogador.fase3Concluida = true; // Marca que a fase 3 foi concluída
                    jogador.fase = 2;
                    
                    // Limpar inimigos
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
                    liberarCartuchos(&listaCartuchos);
                    liberarManchasSangue(&listaManchas);
                    liberarMoedas(&listaMoedas);
                    listaMoedas = NULL;
                    
                    // Carregar mapa do mercado
                    if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase2.txt")) {
                        printf("Erro ao carregar fase2.txt\n");
                    }
                    
                    texturaMapa = recursos->chaoMercado;
                    
                    // Detectar porta de retorno à loja (tile 10 no lado esquerdo)
                    for (int i = 0; i < mapaAtual->altura; i++) {
                        for (int j = 0; j < mapaAtual->largura; j++) {
                            if (mapaAtual->tiles[i][j] == TILE_PORTA_MERCADO && j == 0) {
                                Vector2 posPortaRetorno = {(j * 32) + 16, (i * 32) + 16};
                                criarPorta(&portaRetorno, posPortaRetorno, 1);  // Volta pra loja
                                break;
                            }
                        }
                    }
                    
                    // Detectar porta para rua (tile 11 no lado direito)
                    detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);

                    // Se a fase já foi concluída, destrancar a porta
                    if (jogador.fase2Concluida && porta.ativa) {
                        porta.trancada = false;
                        printf("Porta para RUA destrancada (fase já concluída)\n");
                    }

                    // Spawnar perto da porta da rua (direita)
                    if (porta.ativa) {
                        jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){porta.posicao.x - 80, porta.posicao.y}, 15.0f);
                        if (menina.seguindo) {
                            menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                        }
                        printf("Spawnado perto da porta da RUA no mercado.\n");
                    } else {
                        jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
                        if (menina.seguindo) {
                            menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                        }
                    }
                    
                    portaRetorno2.ativa = false;
                    
                    // Resetar spawn gradativo
                    jogador.spawnadoRetornoFase3 = false;
                    jogador.zumbisSpawnadosRetorno = 0;
                    jogador.tempoSpawnRetorno = 0.0f;
                }
            }
        }
        
        // Verificar interação com porta de retorno à RUA (fase 4 -> fase 3)
        // Não verificar se acabamos de entrar na fase 4 neste frame (evita dupla transição)
        if (jogador.fase == 4 && !transicaoRealizadaNesteFrame) {
            float dx = jogador.posicao.x - portaRetorno3.posicao.x;
            float dy = jogador.posicao.y - portaRetorno3.posicao.y;
            float distancia = sqrtf(dx * dx + dy * dy);

            if (distancia <= 80.0f && IsKeyPressed(KEY_E)) {
                if (!portaRetorno3.ativa) {
                    printf("A porta está trancada. Explore o laboratório.\n");
                } else {
                    printf("Voltando para a RUA!\n");
                    jogador.fase = 3;
                
                // Limpar inimigos
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
                liberarCartuchos(&listaCartuchos);
                liberarManchasSangue(&listaManchas);
                liberarMoedas(&listaMoedas);
                listaMoedas = NULL;
                
                // Carregar mapa da rua
                if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase3.txt")) {
                    printf("Erro ao carregar fase3.txt\n");
                }
                
                texturaMapa = recursos->chaoRua;
                
                // Detectar porta de retorno ao mercado
                for (int i = 0; i < mapaAtual->altura; i++) {
                    for (int j = 0; j < mapaAtual->largura; j++) {
                        if (mapaAtual->tiles[i][j] == TILE_PORTA_MERCADO) {
                            Vector2 posPortaRetorno2 = {(j * 32) + 16, (i * 32) + 16};
                            criarPorta(&portaRetorno2, posPortaRetorno2, 2);
                            portaRetorno2.ativa = true;
                            break;
                        }
                    }
                }
                
                // Detectar porta para laboratório
                detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);

                // Se a fase já foi concluída, destrancar a porta
                if (jogador.fase3Concluida && porta.ativa) {
                    porta.trancada = false;
                    printf("Porta para LAB destrancada (fase já concluída)\n");
                }

                // Spawnar perto da porta do LAB
                if (porta.ativa) {
                    jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){porta.posicao.x - 80, porta.posicao.y}, 15.0f);
                    if (menina.seguindo) {
                        menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                    }
                    printf("Spawnado perto da porta do LAB na rua.\n");
                } else {
                    jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
                    if (menina.seguindo) {
                        menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                    }
                }
                
                jogador.estadoHorda = HORDA_COMPLETA;
                portaRetorno3.ativa = false;
                
                // Resetar spawn gradativo
                jogador.spawnadoRetornoFase4 = false;
                jogador.zumbisSpawnadosRetorno = 0;
                jogador.tempoSpawnRetorno = 0.0f;
                }
            }
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
        
        // Desenha manchas de sangue (embaixo de tudo)
        desenharManchasSangue(listaManchas);
        
        // Desenha moedas
        desenharMoedas(listaMoedas);

        desenharJogo(&jogador, listaZumbis, listaBalas, texturaMapa, recursos);
        
        // Desenha cartuchos apenas se jogo estiver ativo
        if (jogador.vida > 0 && !jogador.jogoVencido) {
            desenharCartuchos(listaCartuchos);
        }

        if (jogador.vida > 0 && !jogador.jogoVencido) {
            desenharBoss(listaBosses);
            
            // Desenhar zumbi do banheiro
            if (zumbiBanheiro != NULL) {
                desenharZumbis(zumbiBanheiro);
            }
            
            // Desenhar menina
            if (menina.ativa) {
                desenharMenina(&menina);
                
                // Mostrar prompt se ainda não liberou a menina
                if (jogador.conheceuMenina && !jogador.meninaLiberada && zumbiBanheiro == NULL) {
                    float dist = sqrtf(
                        (jogador.posicao.x - menina.posicao.x) * (jogador.posicao.x - menina.posicao.x) +
                        (jogador.posicao.y - menina.posicao.y) * (jogador.posicao.y - menina.posicao.y)
                    );
                    if (dist <= 40.0f) {
                        DrawText("Pressione E para falar", (int)menina.posicao.x - 70, (int)menina.posicao.y - 40, 14, YELLOW);
                    }
                }
            }

            if (jogador.fase == 1 || jogador.fase == 2 || jogador.fase == 3) {
                if (jogador.estadoHorda == HORDA_INTERVALO) {
                    int segundosIntervalo = (int)jogador.tempoIntervalo + 1;
                    const char* mensagem = TextFormat("PROXIMA HORDA EM %ds", segundosIntervalo);

                    int larguraTexto = MeasureText(mensagem, 32);
                    int posX = (1024 - larguraTexto) / 2;

                    DrawRectangle(posX - 20, 200, larguraTexto + 40, 50, (Color){0, 0, 0, 180});
                    DrawText(mensagem, posX, 210, 32, YELLOW);
                } else if (jogador.estadoHorda == HORDA_EM_PROGRESSO) {
                    const char* faseNome = (jogador.fase == 2) ? "FASE 2 - MERCADO" : (jogador.fase == 3) ? "FASE 3 - RUA" : "FASE 1 - LOJA";
                    DrawText(TextFormat("%s - HORDA %d/3", faseNome, jogador.hordaAtual), 10, 130, 20, YELLOW);

                    int inimigosTotais = jogador.zumbisRestantes + contarBossesVivos(listaBosses);
                    DrawText(TextFormat("Inimigos: %d", inimigosTotais), 10, 160, 18, WHITE);

                    if (jogador.bossesTotaisHorda > 0) {
                        int bossesVivos = contarBossesVivos(listaBosses);
                        DrawText(TextFormat("Bosses: %d/%d", bossesVivos, jogador.bossesTotaisHorda), 10, 190, 18, RED);
                    }
                } else if (jogador.estadoHorda == HORDA_COMPLETA && jogador.hordaAtual == 3) {
                    const char* mensagem = "TODAS AS HORDAS COMPLETAS!";
                    int larguraTexto = MeasureText(mensagem, 24);
                    int posX = (1024 - larguraTexto) / 2;
                    DrawText(mensagem, posX, 210, 24, GREEN);
                }
            }

            if (!jogador.bossSpawnado && jogador.fase == 4) {
                int segundosRestantes = (int)(45.0f - jogador.timerBoss);
                if (segundosRestantes < 0) segundosRestantes = 0;
                DrawText(TextFormat("BOSS EM: %ds", segundosRestantes), 320, 10, 24, RED);
            }
            
            // Desenhar loja na fase 1
            if (jogador.fase == 1 && loja.ativo) {
                desenharLoja(&loja, &jogador, mapaAtual);
            }
            
            // Desenhar escrivaninha na fase 4
            if (jogador.fase == 4 && escrivaninha.ativa) {
                desenharEscrivaninha(&escrivaninha, recursos);
            }

            // Desenhar relatório em tela cheia quando está sendo lido
            if (jogador.fase == 4 && escrivaninha.ativa && escrivaninha.lida) {
                if (recursos->relatorio.id > 0) {
                    // Desenhar fundo semi-transparente escuro
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 200});

                    // Calcular escala para preencher a tela mantendo proporção
                    float escalaX = (float)GetScreenWidth() / recursos->relatorio.width;
                    float escalaY = (float)GetScreenHeight() / recursos->relatorio.height;
                    float escala = (escalaX < escalaY) ? escalaX : escalaY;

                    // Centralizar relatório na tela
                    float largura = recursos->relatorio.width * escala * 0.9f;  // 90% da tela
                    float altura = recursos->relatorio.height * escala * 0.9f;

                    Rectangle destino = {
                        (GetScreenWidth() - largura) / 2,
                        (GetScreenHeight() - altura) / 2,
                        largura,
                        altura
                    };

                    Rectangle origem = {
                        0, 0,
                        (float)recursos->relatorio.width,
                        (float)recursos->relatorio.height
                    };

                    DrawTexturePro(recursos->relatorio, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);

                    // Desenhar instrução para fechar
                    DrawText("Pressione F para fechar", GetScreenWidth() / 2 - 100, GetScreenHeight() - 40, 20, YELLOW);
                }
            }

            // Desenhar porta final na fase 4
            if (jogador.fase == 4 && portaFinal.ativa) {
                // Selecionar textura baseado no estado do boss
                Texture2D texturaPorta = jogador.matouBossFinal ?
                                         recursos->portaLabAberta :
                                         recursos->portaLabTrancada;

                // Tamanho fixo para ambas as texturas (garante mesmo tamanho)
                float larguraFixa = 100.0f;
                float alturaFixa = 140.0f;

                if (texturaPorta.id > 0) {
                    Rectangle destino = {
                        portaFinal.posicao.x - larguraFixa / 2,
                        portaFinal.posicao.y - alturaFixa / 2,
                        larguraFixa,
                        alturaFixa
                    };

                    Rectangle origem = {
                        0, 0,
                        (float)texturaPorta.width,
                        (float)texturaPorta.height
                    };

                    DrawTexturePro(texturaPorta, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    // Fallback: desenhar retângulo se textura não carregou
                    Color corPorta = jogador.matouBossFinal ? GREEN : DARKGRAY;
                    DrawRectangle(
                        (int)portaFinal.posicao.x - (int)(larguraFixa / 2),
                        (int)portaFinal.posicao.y - (int)(alturaFixa / 2),
                        (int)larguraFixa, (int)alturaFixa, corPorta
                    );
                    DrawRectangleLines(
                        (int)portaFinal.posicao.x - (int)(larguraFixa / 2),
                        (int)portaFinal.posicao.y - (int)(alturaFixa / 2),
                        (int)larguraFixa, (int)alturaFixa, RED
                    );
                }

                float dist = sqrtf(
                    (jogador.posicao.x - portaFinal.posicao.x) * (jogador.posicao.x - portaFinal.posicao.x) +
                    (jogador.posicao.y - portaFinal.posicao.y) * (jogador.posicao.y - portaFinal.posicao.y)
                );
                if (dist <= 80.0f) {
                    if (!jogador.matouBossFinal) {
                        DrawText("TRANCADA - Derrote o boss!", (int)portaFinal.posicao.x - 100, (int)portaFinal.posicao.y + 50, 14, RED);
                    } else if (portaFinal.trancada) {
                        DrawText("Leia o relatório!", (int)portaFinal.posicao.x - 60, (int)portaFinal.posicao.y + 50, 14, YELLOW);
                    } else {
                        DrawText("Pressione E para sair", (int)portaFinal.posicao.x - 70, (int)portaFinal.posicao.y + 50, 14, GREEN);
                    }
                }
            }
            
            // Desenhar porta do mercado na fase 1
            if (jogador.fase == 1 && porta.ativa) {
                desenharPorta(&porta, recursos->texturasTiles[TILE_PORTA_MERCADO]);
                
                float distPortaMercado = sqrtf(
                    (jogador.posicao.x - porta.posicao.x) * (jogador.posicao.x - porta.posicao.x) +
                    (jogador.posicao.y - porta.posicao.y) * (jogador.posicao.y - porta.posicao.y)
                );
                
                if (distPortaMercado <= 60.0f) {
                    DrawText("Pressione E para ir ao MERCADO", (int)porta.posicao.x - 120, (int)porta.posicao.y + 50, 16, YELLOW);

                    if (IsKeyPressed(KEY_E) && !loja.menuAberto) {
                        // Tocar som de porta
                        if (recursos->sfxPorta.frameCount > 0) {
                            PlaySound(recursos->sfxPorta);
                        }

                        // Ir para o mercado (fase 2)
                        jogador.fase = 2;

                        // Limpar tudo
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
                        liberarCartuchos(&listaCartuchos);
                        liberarManchasSangue(&listaManchas);
                        liberarMoedas(&listaMoedas);
                        listaMoedas = NULL;
                        
                        // Carregar mapa da fase 2
                        if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase2.txt")) {
                            printf("Erro: Não foi possível carregar fase2.txt\n");
                        }
                        
                        texturaMapa = recursos->chaoMercado;
                        
                        // Detectar porta de retorno à loja (tile 10 no lado esquerdo)
                        for (int i = 0; i < mapaAtual->altura; i++) {
                            for (int j = 0; j < mapaAtual->largura; j++) {
                                if (mapaAtual->tiles[i][j] == TILE_PORTA_MERCADO && j == 0) {
                                    Vector2 posPortaRetorno = {(j * 32) + 16, (i * 32) + 16};
                                    criarPorta(&portaRetorno, posPortaRetorno, 1);  // Volta pra loja
                                    printf("Porta de retorno criada em (%d, %d)\n", i, j);
                                    break;
                                }
                            }
                        }
                        
                        // Detectar porta para rua (tile 11 no lado direito)
                        detectarPortaNoMapa(mapaAtual, &porta, jogador.fase);
                        
                        if (porta.ativa) {
                            printf("Porta para RUA criada com sucesso! Posição: (%.0f, %.0f)\n", porta.posicao.x, porta.posicao.y);
                            // Se a fase já foi concluída, destrancar a porta
                            if (jogador.fase2Concluida) {
                                porta.trancada = false;
                                printf("Porta para RUA destrancada (fase já concluída)\n");
                            }
                        } else {
                            printf("ERRO: Porta para RUA NÃO foi criada!\n");
                        }
                        
                        // Se fase já foi concluída, spawnar ao lado da porta de retorno
                        if (jogador.fase2Concluida) {
                            jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){portaRetorno.posicao.x + 80, portaRetorno.posicao.y}, 15.0f);
                            if (menina.seguindo) {
                                menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                            }
                            jogador.estadoHorda = HORDA_COMPLETA; // Fase já concluída
                            printf("Retornando à fase 2 já concluída. Sem inimigos.\n");
                        } else {
                            jogador.posicao = certificarPosicaoWalkable(mapaAtual, (Vector2){3 * 32 + 16, 6 * 32 + 16}, 15.0f);
                            if (menina.seguindo) {
                                menina.posicao = gerarPosicaoValidaProximaAoJogador(mapaAtual, jogador.posicao, menina.raio);
                            }
                            iniciarHorda(&jogador, 1);
                            printf("Jogador spawnado na Fase 2 (MERCADO) em posicao fixa: (linha=6, coluna=3) -> (%.0f, %.0f)\n",
                                   jogador.posicao.x, jogador.posicao.y);
                        }
                        
                        printf("Bem-vindo ao MERCADO! Fase 2 iniciada.\n");
                    }
                }
            }
            
            // Porta de saída do banheiro sem visual (apenas detecta proximidade)
            if (jogador.estaNoBanheiro && jogador.conheceuMenina && jogador.meninaLiberada) {
                Vector2 portaSaidaBanheiro = {512, 700};

                float distSaida = sqrtf(
                    (jogador.posicao.x - portaSaidaBanheiro.x) * (jogador.posicao.x - portaSaidaBanheiro.x) +
                    (jogador.posicao.y - portaSaidaBanheiro.y) * (jogador.posicao.y - portaSaidaBanheiro.y)
                );
                if (distSaida <= 50.0f) {
                    DrawText("Pressione E para voltar a LOJA", (int)portaSaidaBanheiro.x - 120, (int)portaSaidaBanheiro.y + 50, 14, GREEN);
                }
            }
            
            // Porta do banheiro invisível na fase 1 (apenas mensagens de interação)
            if (jogador.fase == 1 && portaBanheiro.ativa && !jogador.conheceuMenina) {
                float dist = sqrtf(
                    (jogador.posicao.x - portaBanheiro.posicao.x) * (jogador.posicao.x - portaBanheiro.posicao.x) +
                    (jogador.posicao.y - portaBanheiro.posicao.y) * (jogador.posicao.y - portaBanheiro.posicao.y)
                );
                if (dist <= 80.0f) {
                    if (portaBanheiro.trancada) {
                        DrawText("PORTA TRANCADA - Precisa de chave especial", (int)portaBanheiro.posicao.x - 140, (int)portaBanheiro.posicao.y, 14, RED);
                    } else {
                        DrawText("Pressione E para entrar", (int)portaBanheiro.posicao.x - 80, (int)portaBanheiro.posicao.y, 14, GREEN);
                    }
                }
            }

            // Desenhar porta para RUA (direita) na fase 2
            if (porta.ativa && jogador.fase == 2) {
                DrawRectangle(
                    (int)porta.posicao.x - 30,
                    (int)porta.posicao.y - 40,
                    60, 80, (Color){180, 50, 50, 255}  // Vermelho escuro
                );
                DrawRectangleLines(
                    (int)porta.posicao.x - 30,
                    (int)porta.posicao.y - 40,
                    60, 80, BLACK
                );
                DrawText("RUA", (int)porta.posicao.x - 15, (int)porta.posicao.y - 50, 12, WHITE);
                
                float distPortaRua = sqrtf(
                    (jogador.posicao.x - porta.posicao.x) * (jogador.posicao.x - porta.posicao.x) +
                    (jogador.posicao.y - porta.posicao.y) * (jogador.posicao.y - porta.posicao.y)
                );
                if (distPortaRua <= 60.0f) {
                    DrawText("Pressione E para ir a RUA", (int)porta.posicao.x - 90, (int)porta.posicao.y + 50, 14, YELLOW);
                }
            }
            
            // Desenhar porta de retorno à loja (esquerda) na fase 2
            if (portaRetorno.ativa && jogador.fase == 2) {
                DrawRectangle(
                    (int)portaRetorno.posicao.x - 30,
                    (int)portaRetorno.posicao.y - 40,
                    60, 80, (Color){50, 180, 50, 255}  // Verde escuro
                );
                DrawRectangleLines(
                    (int)portaRetorno.posicao.x - 30,
                    (int)portaRetorno.posicao.y - 40,
                    60, 80, BLACK
                );
                DrawText("LOJA", (int)portaRetorno.posicao.x - 20, (int)portaRetorno.posicao.y - 50, 12, WHITE);
                
                float distPortaLoja = sqrtf(
                    (jogador.posicao.x - portaRetorno.posicao.x) * (jogador.posicao.x - portaRetorno.posicao.x) +
                    (jogador.posicao.y - portaRetorno.posicao.y) * (jogador.posicao.y - portaRetorno.posicao.y)
                );
                if (distPortaLoja <= 60.0f) {
                    DrawText("Pressione E para voltar a LOJA", (int)portaRetorno.posicao.x - 110, (int)portaRetorno.posicao.y + 50, 14, GREEN);
                }
            }
            
            // Porta de retorno ao mercado (esquerda) na fase 3 (invisível)
            if (portaRetorno2.ativa && jogador.fase == 3) {
                float distPortaMercado = sqrtf(
                    (jogador.posicao.x - portaRetorno2.posicao.x) * (jogador.posicao.x - portaRetorno2.posicao.x) +
                    (jogador.posicao.y - portaRetorno2.posicao.y) * (jogador.posicao.y - portaRetorno2.posicao.y)
                );
                if (distPortaMercado <= 60.0f) {
                    DrawText("Pressione E para voltar ao MERCADO", (int)portaRetorno2.posicao.x - 130, (int)portaRetorno2.posicao.y + 50, 14, GREEN);
                }
            }
            
            // Porta para LABORATORIO na fase 3 (invisível - detectada pelo tile do mapa)
            if (porta.ativa && jogador.fase == 3) {
                float distPortaLab = sqrtf(
                    (jogador.posicao.x - porta.posicao.x) * (jogador.posicao.x - porta.posicao.x) +
                    (jogador.posicao.y - porta.posicao.y) * (jogador.posicao.y - porta.posicao.y)
                );
                if (distPortaLab <= 60.0f) {
                    if (porta.trancada) {
                        DrawText("Porta trancada! Precisa da CHAVE", (int)porta.posicao.x - 120, (int)porta.posicao.y + 50, 14, RED);
                    } else {
                        DrawText("Pressione E para ir ao LABORATORIO", (int)porta.posicao.x - 130, (int)porta.posicao.y + 50, 14, YELLOW);
                    }
                }
            }
            
            // Porta de retorno à RUA (inferior) na fase 4 (invisível)
            if (jogador.fase == 4) {
                float distPortaRua = sqrtf(
                    (jogador.posicao.x - portaRetorno3.posicao.x) * (jogador.posicao.x - portaRetorno3.posicao.x) +
                    (jogador.posicao.y - portaRetorno3.posicao.y) * (jogador.posicao.y - portaRetorno3.posicao.y)
                );
                if (distPortaRua <= 80.0f) {
                    if (portaRetorno3.ativa) {
                        DrawText("Pressione E para voltar a RUA", (int)portaRetorno3.posicao.x - 110, (int)portaRetorno3.posicao.y + 50, 14, GREEN);
                    } else {
                        DrawText("TRANCADA - Leia o relatorio", (int)portaRetorno3.posicao.x - 100, (int)portaRetorno3.posicao.y + 50, 14, RED);
                    }
                }
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
            
            // Mostrar moedas
            DrawText(TextFormat("Moedas: %d", jogador.moedas), 10, 100, 20, YELLOW);

            if ((jogador.fase == 2 && jogador.temChave && porta.ativa) ||
                (jogador.fase == 3 && jogador.temChave && porta.ativa) ||
                (jogador.fase == 4 && jogador.temChaveMisteriosa && escrivaninha.trancada)) {
                float tempoTotal = GetTime();
                int mostrar = ((int)(tempoTotal * 2)) % 2;

                if (mostrar) {
                    const char* mensagem;
                    if (jogador.fase == 2) {
                        mensagem = ">> VA PARA A PORTA! <<";
                    } else if (jogador.fase == 3) {
                        mensagem = ">> VA PARA A PORTA DO LABORATORIO! <<";
                    } else {
                        mensagem = ">> VA PARA A ESCRIVANINHA! <<";
                    }
                    int larguraMensagem = MeasureText(mensagem, 20);
                    int posX = 1024 - larguraMensagem - 10;
                    int posY = 40;

                    DrawRectangle(posX - 5, posY - 5, larguraMensagem + 10, 30, (Color){0, 0, 0, 150});
                    DrawText(mensagem, posX, posY, 20, YELLOW);
                }
            }
        }
        
                // Tela de ENDING
        if (jogador.jogoVencido) {
            DrawRectangle(0, 0, 1024, 768, BLACK);  // Fundo preto limpo
            
            // Calcular tempo
            int minutos = (int)jogador.tempoTotal / 60;
            int segundos = (int)fmod(jogador.tempoTotal, 60.0f);
            
            // Determinar rank baseado no TEMPO (menor = melhor)
            const char* rankTempo;
            Color corRank;
            if (jogador.tempoTotal < 300) {  // < 5 min
                rankTempo = "S";
                corRank = GOLD;
            } else if (jogador.tempoTotal < 600) {  // < 10 min
                rankTempo = "A";
                corRank = SKYBLUE;
            } else if (jogador.tempoTotal < 900) {  // < 15 min
                rankTempo = "B";
                corRank = GREEN;
            } else if (jogador.tempoTotal < 1200) {  // < 20 min
                rankTempo = "C";
                corRank = YELLOW;
            } else {
                rankTempo = "D";
                corRank = ORANGE;
            }
            
            // Carregar top 5 tempos
            float tempos[MAX_SCORES];
            loadTimes(tempos, MAX_SCORES);
            
            if (jogador.finalFeliz) {
                // HAPPY ENDING - Tempo maior (voltou pra salvar) mas ganhou humanidade
                DrawText("=== HAPPY ENDING ===", 280, 40, 48, GREEN);
                DrawText("PROPOSITO ENCONTRADO", 290, 100, 28, YELLOW);
                
                // PÓDIO DE TEMPO
                DrawRectangle(100, 150, 824, 120, (Color){20, 40, 20, 200});
                DrawText("=== SEU TEMPO ===", 380, 160, 24, GOLD);
                
                // Calcular milissegundos
                int milis = (int)((jogador.tempoTotal - (int)jogador.tempoTotal) * 1000);
                DrawText(TextFormat("TEMPO: %02d:%02d.%03d", minutos, segundos, milis), 220, 195, 28, WHITE);
                DrawText(TextFormat("RANK: %s", rankTempo), 570, 195, 32, corRank);
                DrawText("(Voltou para salvar a menina)", 310, 240, 18, GRAY);
                
                // TOP 5 MELHORES TEMPOS
                DrawText("=== TOP 5 MELHORES TEMPOS ===", 300, 290, 22, GOLD);
                int posY = 325;
                for (int i = 0; i < 5 && i < MAX_SCORES && tempos[i] < 999999.0f; i++) {
                    int min = (int)tempos[i] / 60;
                    int sec = (int)tempos[i] % 60;
                    int mil = (int)((tempos[i] - (int)tempos[i]) * 1000);
                    Color cor = (fabs(tempos[i] - jogador.tempoTotal) < 0.01f) ? YELLOW : LIGHTGRAY;
                    DrawText(TextFormat("%d. %02d:%02d.%03d", i + 1, min, sec, mil), 340, posY, 20, cor);
                    posY += 30;
                }
                
                // Mensagem narrativa compacta - centralizada
                const char* msg1 = "Voce nao e humano, mas provou ter empatia.";
                int largura1 = MeasureText(msg1, 18);
                DrawText(msg1, (1024 - largura1) / 2, 480, 18, SKYBLUE);
                
                const char* msg2 = "O rank nao importa quando voce tem um proposito.";
                int largura2 = MeasureText(msg2, 18);
                DrawText(msg2, (1024 - largura2) / 2, 505, 18, SKYBLUE);
                
                const char* msg3 = "Uma IA com consciencia e uma crianca assustada.";
                int largura3 = MeasureText(msg3, 18);
                DrawText(msg3, (1024 - largura3) / 2, 540, 18, GREEN);
                
                const char* msg4 = "Juntos, voces sao a nova esperanca.";
                int largura4 = MeasureText(msg4, 18);
                DrawText(msg4, (1024 - largura4) / 2, 565, 18, GREEN);
            } else {
                // BAD ENDING - Tempo rápido (foi direto) mas perdeu a humanidade
                DrawText("=== BAD ENDING ===", 300, 40, 48, RED);
                DrawText("ETERNA SOLIDAO", 330, 100, 28, DARKGRAY);
                
                // PÓDIO DE TEMPO
                DrawRectangle(100, 150, 824, 120, (Color){40, 0, 0, 200});
                DrawText("=== SEU TEMPO ===", 380, 160, 24, GOLD);
                
                // Calcular milissegundos
                int milis = (int)((jogador.tempoTotal - (int)jogador.tempoTotal) * 1000);
                DrawText(TextFormat("TEMPO: %02d:%02d.%03d", minutos, segundos, milis), 220, 195, 28, WHITE);
                DrawText(TextFormat("RANK: %s", rankTempo), 570, 195, 32, corRank);
                DrawText("(Abandonou a menina)", 350, 240, 18, DARKGRAY);
                
                // TOP 5 MELHORES TEMPOS
                DrawText("=== TOP 5 MELHORES TEMPOS ===", 300, 290, 22, GOLD);
                int posY = 325;
                for (int i = 0; i < 5 && i < MAX_SCORES && tempos[i] < 999999.0f; i++) {
                    int min = (int)tempos[i] / 60;
                    int sec = (int)tempos[i] % 60;
                    int mil = (int)((tempos[i] - (int)tempos[i]) * 1000);
                    Color cor = (fabs(tempos[i] - jogador.tempoTotal) < 0.01f) ? YELLOW : LIGHTGRAY;
                    DrawText(TextFormat("%d. %02d:%02d.%03d", i + 1, min, sec, mil), 340, posY, 20, cor);
                    posY += 30;
                }
                
                // Mensagem narrativa compacta - centralizada
                const char* msg1 = "Voce conquistou um bom rank... mas a que custo?";
                int largura1 = MeasureText(msg1, 18);
                DrawText(msg1, (1024 - largura1) / 2, 480, 18, WHITE);
                
                const char* msg2 = "A menina se foi. Todos se foram.";
                int largura2 = MeasureText(msg2, 18);
                DrawText(msg2, (1024 - largura2) / 2, 520, 18, DARKGRAY);
                
                // Efeito de glitch - centralizado
                const char* msg3 = "Voce vagara eternamente... sozinho.";
                int largura3 = MeasureText(msg3, 18);
                if (((int)(GetTime() * 3)) % 2 == 0) {
                    DrawText(msg3, (1024 - largura3) / 2, 555, 18, RED);
                }
            }
            
            DrawText("Pressione R para reiniciar", 340, 620, 20, YELLOW);
            DrawText("Pressione ESC para sair", 360, 650, 18, GRAY);
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
    
    liberarCartuchos(&listaCartuchos);
    liberarManchasSangue(&listaManchas);
    liberarMoedas(&listaMoedas);

    descarregarRecursos(recursos);
    destruirRecursos(recursos);

    destruirMapa(mapaAtual);

    UnloadTexture(logoTexture);
    UnloadTexture(backgroundTexture);

    // Fechar sistema de áudio
    CloseAudioDevice();

    CloseWindow();

    return 0;
}
