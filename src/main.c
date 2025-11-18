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

int main(void) {
    // Configurações da janela (nova resolução para tiles 32x32)
    const int larguraTela = 1024;
    const int alturaTela = 768;

    InitWindow(larguraTela, alturaTela, "Last Breath - Zumbi Survival Game");
    SetTargetFPS(60);

    // Carregar logo do menu
    Texture2D logoTexture = LoadTexture("assets/logo/imagem.png");
    if (logoTexture.id == 0) {
        printf("Aviso: Logo nao foi carregada de assets/logo/imagem.png\n");
    }

    // ===== NOVO SISTEMA DE RECURSOS =====
    // Criar e carregar recursos centralizados
    Recursos* recursos = criarRecursos();
    carregarRecursos(recursos);

    // ===== NOVO SISTEMA DE MAPA =====
    // Criar mapa e carregar Fase 1
    Mapa* mapaAtual = criarMapa();
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
    Porta porta;                      // Porta para transição de fase
    criarPorta(&porta, (Vector2){960, 384}, 2); // Porta para Fase 2 (ajustada para 1024x768)

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
                    jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
                    jogador.spriteAtual = spriteFrenteDireita;
                    jogoIniciado = true;
                }
                jogador.estadoJogo = ESTADO_JOGANDO;
                printf("Jogo iniciado!\n");
            }

            // Desenhar tela de menu
            BeginDrawing();
            ClearBackground(BLACK);

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
        
        // Salvar posição anterior do jogador e zumbis para colisão
        Vector2 posicaoAnteriorJogador = jogador.posicao;

        // Salvar posições anteriores dos zumbis
        typedef struct PosicaoZumbi {
            Zumbi *zumbi;
            Vector2 posicaoAnterior;
            struct PosicaoZumbi *proximo;
        } PosicaoZumbi;

        PosicaoZumbi *listaPosicoesAnteriores = NULL;
        Zumbi *zTemp = listaZumbis;
        while (zTemp != NULL) {
            PosicaoZumbi *novaPosicao = (PosicaoZumbi*)malloc(sizeof(PosicaoZumbi));
            novaPosicao->zumbi = zTemp;
            novaPosicao->posicaoAnterior = zTemp->posicao;
            novaPosicao->proximo = listaPosicoesAnteriores;
            listaPosicoesAnteriores = novaPosicao;
            zTemp = zTemp->proximo;
        }

        // Salvar posições anteriores dos bosses
        typedef struct PosicaoBoss {
            Boss *boss;
            Vector2 posicaoAnterior;
            struct PosicaoBoss *proximo;
        } PosicaoBoss;

        PosicaoBoss *listaPosicoesBosses = NULL;
        Boss *bTemp = listaBosses;
        while (bTemp != NULL) {
            PosicaoBoss *novaPosicao = (PosicaoBoss*)malloc(sizeof(PosicaoBoss));
            novaPosicao->boss = bTemp;
            novaPosicao->posicaoAnterior = bTemp->posicao;
            novaPosicao->proximo = listaPosicoesBosses;
            listaPosicoesBosses = novaPosicao;
            bTemp = bTemp->proximo;
        }

        // Atualizar a lógica do jogo
        atualizarJogo(&jogador, &listaZumbis, &listaBalas);
        
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

        // Verificar colisão dos zumbis com o mapa e reverter se necessário
        PosicaoZumbi *posAtual = listaPosicoesAnteriores;
        while (posAtual != NULL) {
            if (verificarColisaoMapa(mapaAtual, posAtual->zumbi->posicao, 20.0f)) {
                // Reverter para posição anterior
                posAtual->zumbi->posicao = posAtual->posicaoAnterior;
            }
            posAtual = posAtual->proximo;
        }

        // Liberar lista de posições anteriores dos zumbis
        while (listaPosicoesAnteriores != NULL) {
            PosicaoZumbi *temp = listaPosicoesAnteriores;
            listaPosicoesAnteriores = listaPosicoesAnteriores->proximo;
            free(temp);
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
        
        // Atualizar bosses
        atualizarBoss(&listaBosses, &jogador, &listaBalas, GetFrameTime());
        
        // Verificar colisão dos bosses com o mapa e reverter se necessário
        PosicaoBoss *posAtualBoss = listaPosicoesBosses;
        while (posAtualBoss != NULL) {
            if (verificarColisaoMapa(mapaAtual, posAtualBoss->boss->posicao, posAtualBoss->boss->raio)) {
                // Reverter para posição anterior
                posAtualBoss->boss->posicao = posAtualBoss->posicaoAnterior;
            }
            posAtualBoss = posAtualBoss->proximo;
        }

        // Liberar lista de posições anteriores dos bosses
        while (listaPosicoesBosses != NULL) {
            PosicaoBoss *temp = listaPosicoesBosses;
            listaPosicoesBosses = listaPosicoesBosses->proximo;
            free(temp);
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

            // Garantir spawn válido do jogador na nova fase
            jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);

            // Aplicar upgrade de velocidade após Fase 1
            if (jogador.fase == 2) {
                jogador.velocidadeBase = 5.0f;
                printf("UPGRADE! Velocidade aumentada para 5.0 m/s\n");
            }

            // Atualizar porta para próxima fase
            if (porta.faseDestino == 2) {
                criarPorta(&porta, (Vector2){960, 384}, 3); // Porta para Fase 3
            } else if (porta.faseDestino == 3) {
                porta.ativa = false; // Última fase, sem mais portas
            }
        }
        
        // TODO: Implementar sistema de zumbis fortes
        // Atualizar zumbis fortes
        // atualizarZumbisFortes(&listaZumbisFortes, jogador.posicao, GetFrameTime());
        
        // // Verificar colisões com zumbis fortes
        // verificarColisoesBalaZumbiForte(&listaBalas, &listaZumbisFortes, &jogador);
        // verificarColisoesJogadorZumbiForte(&jogador, listaZumbisFortes);
        // verificarColisoesZumbiForteZumbiForte(listaZumbisFortes);
        
        // // Verificar se o zumbi forte morreu e dropar chave
        // if (mapaAtual.temChave && listaZumbisFortes == NULL && !jogadorTemChave && !chaveDropada) {
        //     criarChave(&chave, (Vector2){400, 300});
        //     chaveDropada = true;
        //     printf("Chave dropada! Colete-a para abrir a porta.\n");
        // }

        // // Verificar coleta de chave
        // if (mapaAtual.temChave && !jogadorTemChave && chaveDropada) {
        //     if (verificarColetaChave(&chave, &jogador)) {
        //         jogadorTemChave = true;
        //         printf("CHAVE COLETADA! Va ate a porta e pressione E.\n");
        //     }
        // }

        // // Verificar interação com porta
        // if (mapaAtual.temPorta) {
        //     if (verificarInteracaoPorta(&porta, &jogador, jogadorTemChave)) {
        //         idMapaAtual = 2;
        //         carregarMapa(&mapaAtual, idMapaAtual);
        //         printf("Entrando no Mapa %d!\n", idMapaAtual);
        //         jogador.posicao = mapaAtual.spawnJogador;
        //         jogadorTemChave = false;
        //         chaveDropada = false;
        //         liberarZumbis(&listaZumbis);
        //         liberarZumbisFortes(&listaZumbisFortes);
        //         listaZumbis = NULL;
        //         listaZumbisFortes = NULL;
        //         for (int i = 0; i < mapaAtual.numZumbis; i++) {
        //             float x = GetRandomValue(50, 750);
        //             float y = GetRandomValue(50, 550);
        //             adicionarZumbi(&listaZumbis, (Vector2){x, y}, recursos->zumbis);
        //         }
        //         for (int i = 0; i < mapaAtual.numZumbisFortes; i++) {
        //             float x = GetRandomValue(100, 700);
        //             float y = GetRandomValue(100, 200);
        //             adicionarZumbiForte(&listaZumbisFortes, (Vector2){x, y});
        //         }
        //     }
        // }

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
        desenharJogo(&jogador, listaZumbis, listaBalas, texturaMapa);
        
        // Só desenhar elementos do jogo se estiver vivo e não tiver vencido
        if (jogador.vida > 0 && !jogador.jogoVencido) {
            // Desenhar bosses
            desenharBoss(listaBosses);
            
            // Desenhar informações de horda (Fase 1 e Fase 2)
            if (jogador.fase == 1 || jogador.fase == 2) {
                if (jogador.estadoHorda == HORDA_INTERVALO) {
                    // Mostrar countdown do intervalo com destaque
                    int segundosIntervalo = (int)jogador.tempoIntervalo + 1;
                    const char* mensagem = TextFormat("PROXIMA HORDA EM %ds", segundosIntervalo);
                    
                    // Calcular posição centralizada
                    int larguraTexto = MeasureText(mensagem, 32);
                    int posX = (1024 - larguraTexto) / 2;
                    
                    // Desenhar com fundo semi-transparente
                    DrawRectangle(posX - 20, 90, larguraTexto + 40, 50, (Color){0, 0, 0, 180});
                    DrawText(mensagem, posX, 100, 32, YELLOW);
                } else if (jogador.estadoHorda == HORDA_EM_PROGRESSO) {
                    // Mostrar informações da horda atual
                    const char* faseNome = (jogador.fase == 1) ? "FASE 1" : "FASE 2";
                    DrawText(TextFormat("%s - HORDA %d/3", faseNome, jogador.hordaAtual), 10, 100, 20, YELLOW);
                    
                    // Mostrar contadores de inimigos
                    int inimigosTotais = jogador.zumbisRestantes + contarBossesVivos(listaBosses);
                    DrawText(TextFormat("Inimigos: %d", inimigosTotais), 10, 125, 18, WHITE);
                    
                    if (jogador.bossesTotaisHorda > 0) {
                        int bossesVivos = contarBossesVivos(listaBosses);
                        DrawText(TextFormat("Bosses: %d/%d", bossesVivos, jogador.bossesTotaisHorda), 10, 145, 18, RED);
                    }
                } else if (jogador.estadoHorda == HORDA_COMPLETA && jogador.hordaAtual == 3) {
                    // Mensagem de conclusão
                    const char* mensagem = "TODAS AS HORDAS COMPLETAS!";
                    int larguraTexto = MeasureText(mensagem, 24);
                    int posX = (1024 - larguraTexto) / 2;
                    DrawText(mensagem, posX, 100, 24, GREEN);
                }
            }
            
            // Desenhar timer de boss se ainda não spawnou (apenas para Fase 3)
            if (!jogador.bossSpawnado && jogador.fase == 3) {
                int segundosRestantes = (int)(45.0f - jogador.timerBoss);
                if (segundosRestantes < 0) segundosRestantes = 0;
                DrawText(TextFormat("BOSS EM: %ds", segundosRestantes), 320, 10, 24, RED);
            }
            
            // Desenhar porta
            if (porta.ativa) {
                desenharPorta(&porta);
            }
            
            // Desenhar itens coletáveis
            if (itemProgresso.ativo) {
                desenharItem(&itemProgresso);
            }
            if (itemArma.ativo) {
                desenharItem(&itemArma);
            }
            
            // HUD adicional - Itens coletados
            int offsetX = 600;
            if (jogador.temChave) {
                DrawText("CHAVE", offsetX, 10, 16, GOLD);
                offsetX += 70;
            }
            if (jogador.temMapa) {
                DrawText("MAPA", offsetX, 10, 16, SKYBLUE);
                offsetX += 60;
            }
            if (jogador.temCure) {
                DrawText("CURE", offsetX, 10, 16, GREEN);
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

    // Descarregar logo do menu
    UnloadTexture(logoTexture);

    CloseWindow();

    return 0;
}
