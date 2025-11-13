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

    // NOTA: As texturas antigas foram substituídas pelo sistema de recursos
    // Agora acessamos via recursos->jogadorFrente, recursos->zumbis[tipo][dir], etc.

    // Referências de compatibilidade (mantidas temporariamente)
    Texture2D texturaMapa = recursos->fundoMapa;
    Texture2D spriteFrenteDireita = LoadTexture("avatar/direita frente.png");
    Texture2D spriteFrenteEsquerda = LoadTexture("avatar/esquerda frente.png");
    Texture2D spriteCostasDireita = LoadTexture("avatar/costas direita.png");
    Texture2D spriteCostasEsquerda = LoadTexture("avatar/costas esquerda.png");

    // Verificar se os sprites foram carregados
    if (spriteFrenteDireita.id == 0 || spriteFrenteEsquerda.id == 0 ||
        spriteCostasDireita.id == 0 || spriteCostasEsquerda.id == 0) {
        printf("Aviso: Alguns sprites do jogador nao foram carregados. Usando circulo.\n");
    }

    // Carregar sprites dos zumbis (5 tipos × 4 direções = 20 sprites)
    // Array: [tipo][direção] onde direção: 0=frenteDireita, 1=frenteEsquerda, 2=costasDireita, 3=costasEsquerda
    Texture2D spritesZumbis[5][4];

    spritesZumbis[0][0] = LoadTexture("zumbis/zumbi 1/zumbi1 frente direita.png");
    spritesZumbis[0][1] = LoadTexture("zumbis/zumbi 1/zumbi1 frente esquerda.png");
    spritesZumbis[0][2] = LoadTexture("zumbis/zumbi 1/zumbi1 costas direita.png");
    spritesZumbis[0][3] = LoadTexture("zumbis/zumbi 1/zumbi1 costas esquerda.png");

    spritesZumbis[1][0] = LoadTexture("zumbis/zumbi 2/zumbi2 frente direita.png");
    spritesZumbis[1][1] = LoadTexture("zumbis/zumbi 2/zumbi2 frente esquerda.png");
    spritesZumbis[1][2] = LoadTexture("zumbis/zumbi 2/zumbi2 costas direita.png");
    spritesZumbis[1][3] = LoadTexture("zumbis/zumbi 2/zumbi2 costas esquerda.png");

    spritesZumbis[2][0] = LoadTexture("zumbis/zumbi 3/zumbi3 frente direita.png");
    spritesZumbis[2][1] = LoadTexture("zumbis/zumbi 3/zumbi3 frente esquerda.png");
    spritesZumbis[2][2] = LoadTexture("zumbis/zumbi 3/zumbi3 costas direita.png");
    spritesZumbis[2][3] = LoadTexture("zumbis/zumbi 3/zumbi3 costas esquerda.png");

    spritesZumbis[3][0] = LoadTexture("zumbis/zumbi 4/zumbi4 frente direita.png");
    spritesZumbis[3][1] = LoadTexture("zumbis/zumbi 4/zumbi4 frente esquerda.png");
    spritesZumbis[3][2] = LoadTexture("zumbis/zumbi 4/zumbi4 costas direita.png");
    spritesZumbis[3][3] = LoadTexture("zumbis/zumbi 4/zumbi4 costas esquerda.png");

    spritesZumbis[4][0] = LoadTexture("zumbis/zumbi 5/zumbi5 frente direita.png");
    spritesZumbis[4][1] = LoadTexture("zumbis/zumbi 5/zumbi5 frente esquerda.png");
    spritesZumbis[4][2] = LoadTexture("zumbis/zumbi 5/zumbi5 costas direita.png");
    spritesZumbis[4][3] = LoadTexture("zumbis/zumbi 5/zumbi5 costas esquerda.png");

    // Carregar sprites dos bosses
    // Prowler (Fase 1)
    Texture2D prowlerFrente = LoadTexture("bosses/prowler/frente.PNG");
    Texture2D prowlerCostas = LoadTexture("bosses/prowler/costas.PNG");
    Texture2D prowlerDireita = LoadTexture("bosses/prowler/direita.PNG");
    Texture2D prowlerEsquerda = LoadTexture("bosses/prowler/esquerda.PNG");

    // Verificar se os sprites do Prowler foram carregados
    if (prowlerFrente.id == 0 || prowlerCostas.id == 0 ||
        prowlerDireita.id == 0 || prowlerEsquerda.id == 0) {
        printf("Aviso: Alguns sprites do Prowler nao foram carregados. Usando circulo.\n");
    }

    // Hunter (Fase 2)
    Texture2D hunterFrente = LoadTexture("bosses/hunter/frente.png");
    Texture2D hunterCostas = LoadTexture("bosses/hunter/costas.png");
    Texture2D hunterDireita = LoadTexture("bosses/hunter/direita.png");
    Texture2D hunterEsquerda = LoadTexture("bosses/hunter/esquerda.png");

    // Verificar se os sprites do Hunter foram carregados
    if (hunterFrente.id == 0 || hunterCostas.id == 0 ||
        hunterDireita.id == 0 || hunterEsquerda.id == 0) {
        printf("Aviso: Alguns sprites do Hunter nao foram carregados. Usando circulo.\n");
    }

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

    // Inicializar o jogador
    iniciarJogo(&jogador);

    // Garantir que o jogador spawne em posição válida (fora de tiles sólidos)
    jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);

    // Inicializar o sprite do jogador (começa olhando para frente/direita)
    jogador.spriteAtual = spriteFrenteDireita;

    // Adicionar zumbis normais iniciais (número fixo por enquanto)
    for (int i = 0; i < 5; i++) {
        // Usar nova função de spawn que respeita o mapa
        Vector2 posSpawn = gerarPosicaoValidaSpawn(mapaAtual, 20.0f);
        adicionarZumbi(&listaZumbis, posSpawn, spritesZumbis);
    }

    // Loop principal do jogo
    while (!WindowShouldClose()) {
        // Verificar se pressionou R para reiniciar (quando morreu ou venceu)
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
            
            // Reiniciar jogador
            iniciarJogo(&jogador);

            // Recarregar mapa da Fase 1
            if (!carregarMapaDeArquivo(mapaAtual, "assets/maps/fase1.txt")) {
                inicializarMapaPadrao(mapaAtual);
            }

            // Garantir spawn válido do jogador
            jogador.posicao = gerarPosicaoValidaSpawn(mapaAtual, 15.0f);
            jogador.spriteAtual = spriteFrenteDireita;

            // Resetar itens e porta
            itemProgresso.ativo = false;
            itemArma.ativo = false;
            criarPorta(&porta, (Vector2){960, 384}, 2);

            // Adicionar zumbis iniciais novamente
            for (int i = 0; i < 5; i++) {
                Vector2 posSpawn = gerarPosicaoValidaSpawn(mapaAtual, 20.0f);
                adicionarZumbi(&listaZumbis, posSpawn, spritesZumbis);
            }
            
            printf("Jogo reiniciado!\n");
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
        if (!jogador.bossSpawnado && jogador.vida > 0) {
            jogador.timerBoss += GetFrameTime();
            
            // Verificar se passou 45 segundos ou se todos os zumbis foram mortos
            int numZumbis = 0;
            Zumbi *z = listaZumbis;
            while (z != NULL) {
                numZumbis++;
                z = z->proximo;
            }
            
            if (jogador.timerBoss >= 45.0f || numZumbis == 0) {
                // Spawnar boss baseado na fase atual em posição válida
                
                switch (jogador.fase) {
                    case 1: {
                        Vector2 posicaoBoss = gerarPosicaoValidaSpawn(mapaAtual, 30.0f);
                        criarBoss(&listaBosses, BOSS_PROWLER, posicaoBoss, prowlerFrente, prowlerCostas, prowlerDireita, prowlerEsquerda);
                        printf("BOSS APARECEU: PROWLER!\n");
                        break;
                    }
                    case 2: {
                        // Spawnar 2 Hunters em posições válidas
                        Vector2 posicaoBoss1 = gerarPosicaoValidaSpawn(mapaAtual, 25.0f);
                        Vector2 posicaoBoss2 = gerarPosicaoValidaSpawn(mapaAtual, 25.0f);
                        criarBoss(&listaBosses, BOSS_HUNTER, posicaoBoss1, hunterFrente, hunterCostas, hunterDireita, hunterEsquerda);
                        criarBoss(&listaBosses, BOSS_HUNTER, posicaoBoss2, hunterFrente, hunterCostas, hunterDireita, hunterEsquerda);
                        printf("BOSS APARECEU: 2x HUNTERS!\n");
                        break;
                    }
                    case 3: {
                        // Boss ABOMINATION centralizado no mapa (32x24 tiles * 32px = centro em 512, 384)
                        Vector2 posicaoBoss = {512.0f, 384.0f};
                        criarBoss(&listaBosses, BOSS_ABOMINATION, posicaoBoss, (Texture2D){0}, (Texture2D){0}, (Texture2D){0}, (Texture2D){0});
                        printf("BOSS APARECEU: ABOMINATION!\n");
                        break;
                    }
                }
                
                jogador.bossSpawnado = true;
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
        
        // Verificar colisões boss vs balas (passa os 2 itens para dropar) e boss vs jogador
        verificarColisoesBossBala(&listaBosses, &listaBalas, &itemProgresso, &itemArma);
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
        //             adicionarZumbi(&listaZumbis, (Vector2){x, y}, spritesZumbis);
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
            
            // Desenhar timer de boss se ainda não spawnou
            if (!jogador.bossSpawnado) {
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

    // Descarregar texturas de compatibilidade temporária
    UnloadTexture(spriteFrenteDireita);
    UnloadTexture(spriteFrenteEsquerda);
    UnloadTexture(spriteCostasDireita);
    UnloadTexture(spriteCostasEsquerda);

    // Descarregar sprites dos zumbis (compatibilidade)
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            UnloadTexture(spritesZumbis[i][j]);
        }
    }

    // Descarregar sprites dos bosses (compatibilidade)
    UnloadTexture(prowlerFrente);
    UnloadTexture(prowlerCostas);
    UnloadTexture(prowlerDireita);
    UnloadTexture(prowlerEsquerda);
    UnloadTexture(hunterFrente);
    UnloadTexture(hunterCostas);
    UnloadTexture(hunterDireita);
    UnloadTexture(hunterEsquerda);

    CloseWindow();

    return 0;
}
