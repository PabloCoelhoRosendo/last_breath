// src/main.c
// Arquivo principal do jogo Last Breath

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"
#include "mapa.h"
#include "recursos.h"
#include "pathfinding.h"

// Função auxiliar para detectar porta no mapa atual
// Procura tiles 10 (TILE_PORTA_MERCADO) ou 11 (TILE_PORTA_LAB)
// e cria uma Porta na posição correta
void detectarPortaNoMapa(Mapa* mapa, Porta* portaPtr) {
    portaPtr->ativa = false;  // Resetar porta

    // Procurar tile da porta no mapa
    for (int i = 0; i < mapa->altura; i++) {
        for (int j = 0; j < mapa->largura; j++) {
            if (mapa->tiles[i][j] == TILE_PORTA_MERCADO) {
                // Encontrou a porta do mercado (Fase 1 -> Fase 2)
                // A porta está em 7 tiles verticais, calcular centro
                Vector2 posPorta = {(j * 32) + 16, ((i + 3) * 32) + 16};  // Centro aproximado
                criarPorta(portaPtr, posPorta, 2);  // Porta para Fase 2
                printf("Porta do Mercado encontrada no mapa em tile (%d, %d) -> posicao (%.0f, %.0f)\n",
                       i, j, posPorta.x, posPorta.y);
                return;  // Encontrou, sair
            } else if (mapa->tiles[i][j] == TILE_PORTA_LAB) {
                // Encontrou a porta do laboratório (Fase 2 -> Fase 3)
                // É um tile único invisível abaixo do prédio do laboratório
                Vector2 posPorta = {(j * 32) + 16, (i * 32) + 16};  // Centro do tile
                criarPorta(portaPtr, posPorta, 3);  // Porta para Fase 3
                // Aumentar área de detecção para a porta invisível do laboratório
                portaPtr->largura = 100.0f;  // Área mais larga (aproximadamente 3 tiles)
                portaPtr->altura = 80.0f;    // Área mais alta (aproximadamente 2.5 tiles)
                printf("Porta do Laboratorio encontrada no mapa em tile (%d, %d) -> posicao (%.0f, %.0f) [Area: %.0fx%.0f]\n",
                       i, j, posPorta.x, posPorta.y, portaPtr->largura, portaPtr->altura);
                return;  // Encontrou, sair
            }
        }
    }

    // Se não encontrou nenhuma porta, avisar
    printf("Aviso: Nenhuma porta encontrada no mapa atual\n");
}

int main(void) {
    // Configurações da janela (nova resolução para tiles 32x32)
    const int larguraTela = 1024;
    const int alturaTela = 768;

    InitWindow(larguraTela, alturaTela, "Last Breath - Zumbi Survival Game");
    SetTargetFPS(60);

    // Carregar logo e background do menu
    Texture2D logoTexture = LoadTexture("assets/logo/imagem.png");
    if (logoTexture.id == 0) {
        printf("Aviso: Logo nao foi carregada de assets/logo/imagem.png\n");
    }

    Texture2D backgroundTexture = LoadTexture("assets/background/background.png");
    if (backgroundTexture.id == 0) {
        printf("Aviso: Background nao foi carregado de assets/background/background.png\n");
    }

    // ===== NOVO SISTEMA DE RECURSOS =====
    // Criar e carregar recursos centralizados
    Recursos* recursos = criarRecursos();
    carregarRecursos(recursos);

    // ===== NOVO SISTEMA DE MAPA =====
    Mapa* mapaAtual = criarMapa();
    // Carregar mapa da Fase 1
    if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
        printf("Aviso: Nao foi possivel carregar fase1.txt. Usando mapa padrao.\n");
        inicializarMapaPadrao(mapaAtual);
    }

    // NOTA: Todas as texturas agora são gerenciadas pelo sistema de recursos
    // Acessamos via recursos->jogadorFrente, recursos->jogadorDireita, recursos->zumbis[tipo][dir], etc.

    // Referências de compatibilidade (mantidas temporariamente para o código legado)
    Texture2D texturaMapa = recursos->fundoMapa;
    Texture2D spriteFrenteDireita = recursos->jogadorDireita;
    Texture2D spriteFrenteEsquerda = recursos->jogadorEsquerda;
    Texture2D spriteCostasDireita = recursos->jogadorTras;
    Texture2D spriteCostasEsquerda = recursos->jogadorTras;  // Mesmo sprite para ambos os lados

    // NOTA: Sprites dos bosses agora são gerenciados pelo sistema de recursos
    // Acessamos via recursos->bosses[tipo][direcao]
    // Índices: [0]=Prowler, [1]=Hunter, [2]=Abomination
    // Direções: [0]=frente, [1]=costas, [2]=esquerda, [3]=direita

    // Referências de compatibilidade para o código legado
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

    // Inicializar estruturas do jogo
    Player jogador;
    Zumbi *listaZumbis = NULL;        // Lista encadeada de zumbis normais
    Boss *listaBosses = NULL;         // Lista encadeada de bosses
    // ZumbiForte *listaZumbisFortes = NULL;  // Lista encadeada de zumbis fortes (TODO: Implementar)
    Bala *listaBalas = NULL;          // Lista encadeada de balas

    // Sistema de Itens e Porta
    Item itemProgresso;               // Item de progressão (Chave, Mapa, CURE)
    itemProgresso.ativo = false;      // Começa sem item no mapa
    Item itemArma;                    // Item de arma (Shotgun, SMG)
    itemArma.ativo = false;           // Começa sem arma no mapa

    // Sistema de Pathfinding A*
    PathfindingGrid pathfindingGrid;
    inicializarPathfinding(&pathfindingGrid);
    printf("Sistema de Pathfinding A* inicializado!\n");

    // Detectar porta no mapa (tile 10 = TILE_PORTA_MERCADO, tile 11 = TILE_PORTA_LAB)
    // A função detectarPortaNoMapa() será chamada sempre que o mapa for carregado
    Porta porta;
    porta.ativa = false;  // Será ativada quando encontrarmos o tile

    // Detectar porta no mapa inicial (Fase 1)
    detectarPortaNoMapa(mapaAtual, &porta);

    // Inicializar o jogador no estado de menu
    jogador.estadoJogo = ESTADO_MENU;
    bool jogoIniciado = false;  // Flag para controlar se o jogo já foi iniciado

    // Garantir que o jogador spawne em posição válida (fora de tiles sólidos)
    jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);

    // Inicializar o sprite do jogador (começa olhando para frente/direita)
    jogador.spriteAtual = spriteFrenteDireita;

    // NÃO spawnar zumbis inicialmente - o sistema de hordas irá gerenciar isso
    // O sistema de hordas começará automaticamente quando o jogo iniciar

    // Loop principal do jogo
    while (!WindowShouldClose()) {
        // ===== TELA DE MENU =====
        if (jogador.estadoJogo == ESTADO_MENU) {
            // Definir retângulo do botão "Jogar" (centralizado)
            Rectangle botaoJogar = {
                larguraTela / 2 - 100,  // x (centralizado)
                alturaTela / 2 + 150,   // y (abaixo da logo)
                200,                     // largura
                60                       // altura
            };

            // Verificar se o mouse está sobre o botão
            Vector2 mousePos = GetMousePosition();
            bool mouseNoBot = CheckCollisionPointRec(mousePos, botaoJogar);

            // Verificar clique no botão
            if (mouseNoBot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Iniciar o jogo
                if (!jogoIniciado) {
                    iniciarJogo(&jogador);

                    // Definir posição inicial baseada na fase
                    if (jogador.fase == 3) {
                        // Fase 3: Spawnar em posição específica (linha=21, coluna=16 - meio)
                        jogador.posicao = (Vector2){16 * 32 + 16, 21 * 32 + 16};  // (528, 688)
                        printf("Spawn inicial Fase 3: (linha=21, coluna=16 - meio) -> (%.0f, %.0f)\n",
                               jogador.posicao.x, jogador.posicao.y);
                    } else {
                        // Fase 1 ou 2: spawn aleatório válido
                        jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
                    }

                    jogador.spriteAtual = spriteFrenteDireita;
                    jogoIniciado = true;
                }
                jogador.estadoJogo = ESTADO_JOGANDO;
                printf("Jogo iniciado!\n");
            }

            // Desenhar tela de menu
            BeginDrawing();
            ClearBackground(BLACK);

            // Desenhar background (fullscreen)
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

            // Desenhar logo (centralizada)
            if (logoTexture.id > 0) {
                float escalaLogo = 0.8f;  // Ajuste conforme necessário
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
                // Fallback: texto se logo não carregou
                DrawText("LAST BREATH", larguraTela / 2 - 200, 150, 60, RED);
            }

            // Desenhar botão "Jogar"
            Color corBotao = mouseNoBot ? GREEN : DARKGREEN;
            DrawRectangleRec(botaoJogar, corBotao);
            DrawRectangleLinesEx(botaoJogar, 3, LIME);

            // Texto do botão centralizado
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
            continue;  // Pular o resto do loop e voltar para o início
        }

        // Verificar se pressionou R para voltar ao menu (quando morreu ou venceu)
        if ((jogador.vida <= 0 || jogador.jogoVencido) && IsKeyPressed(KEY_R)) {
            // Limpar todos os recursos do jogo atual
            liberarZumbis(&listaZumbis);
            listaZumbis = NULL;

            // Limpar bosses
            while (listaBosses != NULL) {
                Boss *temp = listaBosses;
                listaBosses = listaBosses->proximo;
                free(temp);
            }

            // Limpar balas
            while (listaBalas != NULL) {
                Bala *temp = listaBalas;
                listaBalas = listaBalas->proximo;
                free(temp);
            }


            // Recarregar mapa da Fase 1
            if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
                inicializarMapaPadrao(mapaAtual);
            }

            // Resetar itens e porta
            itemProgresso.ativo = false;
            itemArma.ativo = false;
            criarPorta(&porta, (Vector2){960, 384}, 2);

            // Voltar ao menu
            jogador.estadoJogo = ESTADO_MENU;
            jogoIniciado = false;

            printf("Voltando ao menu principal...\n");
        }
        
        // Salvar posição anterior do jogador para colisão
        Vector2 posicaoAnteriorJogador = jogador.posicao;

        // Atualizar a lógica do jogo com pathfinding A* para zumbis
        atualizarJogoComPathfinding(&jogador, &listaZumbis, &listaBalas, mapaAtual, &pathfindingGrid);
        
        // ===== SISTEMA DE HORDAS =====
        // Atualizar sistema de hordas (Fase 1 e Fase 2)
        if (jogador.fase == 1 || jogador.fase == 2) {
            atualizarHorda(&jogador, &listaZumbis, &listaBosses, GetFrameTime());
            
            // Spawnar zumbis conforme a horda progride (com delay de 1 segundo entre cada spawn)
            if (jogador.estadoHorda == HORDA_EM_PROGRESSO && 
                jogador.zumbisSpawnados < jogador.zumbisTotaisHorda) {
                
                jogador.tempoSpawn += GetFrameTime();
                
                // Spawnar um zumbi a cada 1 segundo
                if (jogador.tempoSpawn >= 1.0f) {
                    Vector2 posSpawn = gerarPosicaoValidaSpawn(mapaAtual, 20.0f);
                    adicionarZumbi(&listaZumbis, posSpawn, recursos->zumbis);
                    jogador.zumbisSpawnados++;
                    jogador.tempoSpawn = 0.0f;  // Resetar timer
                    printf("Zumbi spawnado! (%d/%d)\n", jogador.zumbisSpawnados, jogador.zumbisTotaisHorda);
                }
            }
            
            // Spawnar bosses conforme a horda progride (com delay de 2 segundos entre cada spawn)
            if (jogador.estadoHorda == HORDA_EM_PROGRESSO && 
                jogador.bossesSpawnados < jogador.bossesTotaisHorda) {
                
                jogador.tempoSpawnBoss += GetFrameTime();
                
                // Spawnar um boss a cada 2 segundos
                if (jogador.tempoSpawnBoss >= 2.0f) {
                    Vector2 posSpawn = gerarPosicaoValidaSpawn(mapaAtual, 25.0f);
                    
                    // Spawnar tipo correto de boss baseado na fase
                    if (jogador.fase == 1) {
                        // Fase 1: Prowler
                        criarBoss(&listaBosses, BOSS_PROWLER, posSpawn, prowlerFrente, prowlerCostas, prowlerDireita, prowlerEsquerda);
                        printf("PROWLER spawnado! (%d/%d)\n", jogador.bossesSpawnados + 1, jogador.bossesTotaisHorda);
                    } else if (jogador.fase == 2) {
                        // Fase 2: Hunter
                        criarBoss(&listaBosses, BOSS_HUNTER, posSpawn, hunterFrente, hunterCostas, hunterDireita, hunterEsquerda);
                        printf("HUNTER spawnado! (%d/%d)\n", jogador.bossesSpawnados + 1, jogador.bossesTotaisHorda);
                    }
                    
                    jogador.bossesSpawnados++;
                    jogador.tempoSpawnBoss = 0.0f;  // Resetar timer
                }
            }
        }
        
        // Atualizar balas com verificação de colisão no mapa
        atualizarBalas(&listaBalas, mapaAtual);

        // Verificar colisão do jogador com o mapa (novo sistema)
        if (verificarColisaoMapa(mapaAtual, jogador.posicao, 15.0f)) {
            // Se colidiu, reverter para posição anterior
            jogador.posicao = posicaoAnteriorJogador;
        }

        // Verificar colisão dos zumbis com o mapa e tentar deslizar/contornar
        Zumbi *zumbiAtual = listaZumbis;
        while (zumbiAtual != NULL) {
            if (verificarColisaoMapa(mapaAtual, zumbiAtual->posicao, 20.0f)) {
                // Tentar deslizar ao longo do obstáculo em vez de simplesmente reverter
                Vector2 posOriginal = zumbiAtual->posicaoAnterior;
                Vector2 movimento = {
                    zumbiAtual->posicao.x - posOriginal.x,
                    zumbiAtual->posicao.y - posOriginal.y
                };

                bool encontrouCaminho = false;

                // Tentar mover apenas no eixo X
                Vector2 tentativaX = {posOriginal.x + movimento.x, posOriginal.y};
                if (!verificarColisaoMapa(mapaAtual, tentativaX, 20.0f)) {
                    zumbiAtual->posicao = tentativaX;
                    encontrouCaminho = true;
                }

                // Tentar mover apenas no eixo Y
                if (!encontrouCaminho) {
                    Vector2 tentativaY = {posOriginal.x, posOriginal.y + movimento.y};
                    if (!verificarColisaoMapa(mapaAtual, tentativaY, 20.0f)) {
                        zumbiAtual->posicao = tentativaY;
                        encontrouCaminho = true;
                    }
                }

                // Se não conseguiu deslizar, tentar desviar perpendicular
                if (!encontrouCaminho) {
                    float velocidade = sqrtf(movimento.x * movimento.x + movimento.y * movimento.y);
                    if (velocidade > 0.1f) {
                        // Tentar desviar para a direita perpendicular
                        Vector2 desvioDir = {posOriginal.x - movimento.y * 0.5f, posOriginal.y + movimento.x * 0.5f};
                        if (!verificarColisaoMapa(mapaAtual, desvioDir, 20.0f)) {
                            zumbiAtual->posicao = desvioDir;
                            encontrouCaminho = true;
                        }

                        // Tentar desviar para a esquerda perpendicular
                        if (!encontrouCaminho) {
                            Vector2 desvioEsq = {posOriginal.x + movimento.y * 0.5f, posOriginal.y - movimento.x * 0.5f};
                            if (!verificarColisaoMapa(mapaAtual, desvioEsq, 20.0f)) {
                                zumbiAtual->posicao = desvioEsq;
                                encontrouCaminho = true;
                            }
                        }
                    }
                }

                // Se ainda não encontrou caminho, reverter e forçar recálculo do pathfinding
                if (!encontrouCaminho) {
                    zumbiAtual->posicao = posOriginal;
                    // Invalidar caminho para forçar recálculo
                    zumbiAtual->caminho.valido = false;
                    zumbiAtual->caminho.tempoRecalculo = 1.0f; // Forçar recálculo imediato
                }
            }
            zumbiAtual = zumbiAtual->proximo;
        }

        // Atualizar timer de boss e spawnar quando necessário
        // MODIFICADO: Na Fase 1, o boss spawna apenas na terceira horda via sistema de hordas
        // Na Fase 2, os Hunters spawnam via sistema de hordas
        // Na Fase 3, o Abomination spawna por timer ou quando zumbis acabam
        if (!jogador.bossSpawnado && jogador.vida > 0) {
            // Na Fase 1, boss spawna via sistema de hordas (não usar timer)
            if (jogador.fase == 1) {
                // Boss já é spawnado pelo sistema de hordas na horda 3
                // Nada a fazer aqui
            }
            // Na Fase 2, Hunters spawnam via sistema de hordas (não usar timer)
            else if (jogador.fase == 2) {
                // Hunters já são spawnados pelo sistema de hordas
                // Nada a fazer aqui
            }
            // Fase 3 mantém o sistema de timer original
            else if (jogador.fase == 3) {
                jogador.timerBoss += GetFrameTime();
                
                // Verificar se passou 45 segundos ou se todos os zumbis foram mortos
                int numZumbis = 0;
                Zumbi *z = listaZumbis;
                while (z != NULL) {
                    numZumbis++;
                    z = z->proximo;
                }
                
                if (jogador.timerBoss >= 45.0f || numZumbis == 0) {
                    // Boss ABOMINATION centralizado no mapa (32x24 tiles * 32px = centro em 512, 384)
                    Vector2 posicaoBoss = {512.0f, 384.0f};
                    criarBoss(&listaBosses, BOSS_ABOMINATION, posicaoBoss, abominationFrente, abominationCostas, abominationDireita, abominationEsquerda);
                    printf("BOSS APARECEU: ABOMINATION!\n");
                    
                    jogador.bossSpawnado = true;
                }
            }
        }
        
        // Atualizar bosses COM PATHFINDING A*
        atualizarBossComPathfinding(&listaBosses, &jogador, &listaBalas, GetFrameTime(), mapaAtual, &pathfindingGrid);

        // Verificar colisão dos bosses com o mapa com deslizamento
        Boss *bossAtual = listaBosses;
        while (bossAtual != NULL) {
            if (verificarColisaoMapa(mapaAtual, bossAtual->posicao, bossAtual->raio)) {
                // Tentar deslizar ao longo do obstáculo
                Vector2 posOriginal = bossAtual->posicaoAnterior;
                Vector2 movimento = {
                    bossAtual->posicao.x - posOriginal.x,
                    bossAtual->posicao.y - posOriginal.y
                };

                bool encontrouCaminho = false;

                // Tentar mover apenas no eixo X
                Vector2 tentativaX = {posOriginal.x + movimento.x, posOriginal.y};
                if (!verificarColisaoMapa(mapaAtual, tentativaX, bossAtual->raio)) {
                    bossAtual->posicao = tentativaX;
                    encontrouCaminho = true;
                }

                // Tentar mover apenas no eixo Y
                if (!encontrouCaminho) {
                    Vector2 tentativaY = {posOriginal.x, posOriginal.y + movimento.y};
                    if (!verificarColisaoMapa(mapaAtual, tentativaY, bossAtual->raio)) {
                        bossAtual->posicao = tentativaY;
                        encontrouCaminho = true;
                    }
                }

                // Se não conseguiu deslizar, reverter e forçar recálculo do pathfinding
                if (!encontrouCaminho) {
                    bossAtual->posicao = posOriginal;
                    bossAtual->caminho.valido = false;
                    bossAtual->caminho.tempoRecalculo = 1.0f;
                }
            }
            bossAtual = bossAtual->proximo;
        }
        
        // Verificar colisões boss vs balas (passa os 2 itens para dropar e o jogador) e boss vs jogador
        verificarColisoesBossBala(&listaBosses, &listaBalas, &itemProgresso, &itemArma, &jogador);
        verificarColisoesBossJogador(listaBosses, &jogador);
        
        // Verificar coleta de itens
        if (itemProgresso.ativo) {
            verificarColetaItem(&itemProgresso, &jogador);
        }
        if (itemArma.ativo) {
            verificarColetaItem(&itemArma, &jogador);
        }
        
        // Verificar interação com porta
        if (porta.ativa && verificarInteracaoPorta(&porta, &jogador)) {
            printf("Usando porta! Indo para Fase %d\n", porta.faseDestino);
            jogador.fase = porta.faseDestino;
            jogador.timerBoss = 0.0f;
            jogador.bossSpawnado = false;
            
            // IMPORTANTE: Limpar todos os inimigos da fase anterior
            printf("Limpando zumbis da fase anterior...\n");
            liberarZumbis(&listaZumbis);
            listaZumbis = NULL;
            
            // Limpar bosses da fase anterior
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
            
            // Limpar balas da fase anterior
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
            
            // Resetar sistema de hordas para nova fase
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

            // Carregar o mapa da nova fase
            char caminhoMapa[64];
            snprintf(caminhoMapa, sizeof(caminhoMapa), "assets/maps/fase%d.txt", jogador.fase);
            if (!carregarMapaDeArquivo(mapaAtual, caminhoMapa)) {
                printf("Aviso: Nao foi possivel carregar %s. Usando mapa padrao.\n", caminhoMapa);
                inicializarMapaPadrao(mapaAtual);
            }

            // Spawn do jogador na nova fase
            if (jogador.fase == 2) {
                // Fase 2: Spawnar em posição específica da matriz (linha=9, coluna=8)
                // Converter para coordenadas do mundo (centro do tile)
                jogador.posicao = (Vector2){8 * 32 + 16, 9 * 32 + 16};  // (272, 304)
                printf("Jogador spawnado na Fase 2 em posicao fixa: (linha=9, coluna=8) -> (%.0f, %.0f)\n",
                       jogador.posicao.x, jogador.posicao.y);
            } else if (jogador.fase == 3) {
                // Fase 3: Spawnar em posição específica (linha=21, coluna=16 - meio)
                // Converter para coordenadas do mundo (centro do tile)
                jogador.posicao = (Vector2){16 * 32 + 16, 21 * 32 + 16};  // (528, 688)
                printf("Jogador spawnado na Fase 3 em posicao fixa: (linha=21, coluna=16 - meio) -> (%.0f, %.0f)\n",
                       jogador.posicao.x, jogador.posicao.y);
            } else {
                // Fase 1: spawn aleatório em posição válida
                jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
            }

            // Aplicar upgrade de velocidade após Fase 1
            if (jogador.fase == 2) {
                jogador.velocidadeBase = 5.0f;
                printf("UPGRADE! Velocidade aumentada para 5.0 m/s\n");
            }

            // Detectar porta no novo mapa (busca automática por tiles 10 ou 11)
            detectarPortaNoMapa(mapaAtual, &porta);
        }
        
        // Atualizar sprite do jogador baseado na direção
        if (jogador.direcaoVertical == 0) { // Frente
            if (jogador.direcaoHorizontal == 0) {
                jogador.spriteAtual = spriteFrenteEsquerda;
            } else {
                jogador.spriteAtual = spriteFrenteDireita;
            }
        } else { // Costas
            if (jogador.direcaoHorizontal == 0) {
                jogador.spriteAtual = spriteCostasEsquerda;
            } else {
                jogador.spriteAtual = spriteCostasDireita;
            }
        }

        // Desenhar tudo na tela
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Desenhar o novo sistema de mapa com tiles
        desenharMapaTiles(mapaAtual, recursos->texturasTiles);

        // Desenhar elementos do jogo (jogador, zumbis, balas)
        // A função antiga desenharJogo() também desenha o mapa antigo, mas vamos sobrescrever
        desenharJogo(&jogador, listaZumbis, listaBalas, texturaMapa, recursos);
        
        // Só desenhar elementos do jogo se estiver vivo e não tiver vencido
        if (jogador.vida > 0 && !jogador.jogoVencido) {
            // Desenhar bosses
            desenharBoss(listaBosses);
            
            // Desenhar informações de horda (Fase 1 e Fase 2)
            if (jogador.fase == 1 || jogador.fase == 2) {
                if (jogador.estadoHorda == HORDA_INTERVALO) {
                    // Mostrar countdown do intervalo com destaque (centralizado)
                    int segundosIntervalo = (int)jogador.tempoIntervalo + 1;
                    const char* mensagem = TextFormat("PROXIMA HORDA EM %ds", segundosIntervalo);

                    // Calcular posição centralizada
                    int larguraTexto = MeasureText(mensagem, 32);
                    int posX = (1024 - larguraTexto) / 2;

                    // Desenhar com fundo semi-transparente (posição ajustada para não sobrepor)
                    DrawRectangle(posX - 20, 200, larguraTexto + 40, 50, (Color){0, 0, 0, 180});
                    DrawText(mensagem, posX, 210, 32, YELLOW);
                } else if (jogador.estadoHorda == HORDA_EM_PROGRESSO) {
                    // Mostrar informações da horda atual (abaixo do HUD principal)
                    const char* faseNome = (jogador.fase == 1) ? "FASE 1" : "FASE 2";
                    DrawText(TextFormat("%s - HORDA %d/3", faseNome, jogador.hordaAtual), 10, 150, 20, YELLOW);

                    // Mostrar contadores de inimigos
                    int inimigosTotais = jogador.zumbisRestantes + contarBossesVivos(listaBosses);
                    DrawText(TextFormat("Inimigos: %d", inimigosTotais), 10, 175, 18, WHITE);

                    if (jogador.bossesTotaisHorda > 0) {
                        int bossesVivos = contarBossesVivos(listaBosses);
                        DrawText(TextFormat("Bosses: %d/%d", bossesVivos, jogador.bossesTotaisHorda), 10, 195, 18, RED);
                    }
                } else if (jogador.estadoHorda == HORDA_COMPLETA && jogador.hordaAtual == 3) {
                    // Mensagem de conclusão (centralizada)
                    const char* mensagem = "TODAS AS HORDAS COMPLETAS!";
                    int larguraTexto = MeasureText(mensagem, 24);
                    int posX = (1024 - larguraTexto) / 2;
                    DrawText(mensagem, posX, 210, 24, GREEN);
                }
            }
            
            // Desenhar timer de boss se ainda não spawnou (apenas para Fase 3)
            if (!jogador.bossSpawnado && jogador.fase == 3) {
                int segundosRestantes = (int)(45.0f - jogador.timerBoss);
                if (segundosRestantes < 0) segundosRestantes = 0;
                DrawText(TextFormat("BOSS EM: %ds", segundosRestantes), 320, 10, 24, RED);
            }
            
            // Desenhar porta (apenas na Fase 1 - porta do mercado é visível)
            // Na Fase 2, a porta é invisível (apenas ponto de interação)
            if (porta.ativa && jogador.fase == 1) {
                desenharPorta(&porta, recursos->texturasTiles[TILE_PORTA_MERCADO]);
            }
            
            // Desenhar itens coletáveis
            if (itemProgresso.ativo) {
                desenharItem(&itemProgresso, recursos);
            }
            if (itemArma.ativo) {
                desenharItem(&itemArma, recursos);
            }
            
            // HUD adicional - Itens coletados (canto superior direito)
            int offsetX = 1024 - 10;  // Começar do canto direito
            if (jogador.temCure) {
                int largura = MeasureText("CURE", 18);
                offsetX -= largura;
                DrawText("CURE", offsetX, 10, 18, GREEN);
                offsetX -= 15;  // Espaço entre itens
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

            // Mensagem piscante quando tiver chave na Fase 1 ou mapa na Fase 2
            if ((jogador.fase == 1 && jogador.temChave && porta.ativa) ||
                (jogador.fase == 2 && jogador.temMapa && porta.ativa)) {
                // Piscar a cada 0.5 segundos (usando tempo do jogo)
                float tempoTotal = GetTime();
                int mostrar = ((int)(tempoTotal * 2)) % 2;  // Alterna entre 0 e 1

                if (mostrar) {
                    // Mensagem diferente para cada fase
                    const char* mensagem = (jogador.fase == 1) ?
                        ">> VA PARA A PORTA! <<" :
                        ">> VA PARA A PORTA DO LABORATORIO! <<";
                    int larguraMensagem = MeasureText(mensagem, 20);
                    int posX = 1024 - larguraMensagem - 10;  // Canto superior direito
                    int posY = 40;  // Abaixo dos itens coletados

                    // Desenhar com fundo semi-transparente
                    DrawRectangle(posX - 5, posY - 5, larguraMensagem + 10, 30, (Color){0, 0, 0, 150});
                    DrawText(mensagem, posX, posY, 20, YELLOW);
                }
            }
        }
        
        // TODO: Implementar desenho de zumbis fortes
        
        // DrawText(TextFormat("Mapa: %d", idMapaAtual), 10, 110, 20, WHITE);

        EndDrawing();
    }

    // Limpar recursos antes de fechar
    liberarZumbis(&listaZumbis);

    // Limpar bosses e balas
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

    // Descarregar o novo sistema de recursos
    descarregarRecursos(recursos);
    destruirRecursos(recursos);

    // Destruir o mapa
    destruirMapa(mapaAtual);

    // NOTA: Sprites do jogador, zumbis e bosses são descarregados pelo sistema de recursos
    // em descarregarRecursos(recursos) - não precisamos descarregar as referências

    // Descarregar logo e background do menu
    UnloadTexture(logoTexture);
    UnloadTexture(backgroundTexture);

    CloseWindow();

    return 0;
}
