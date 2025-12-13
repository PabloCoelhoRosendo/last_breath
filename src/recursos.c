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

    for (int i = 0; i < 4; i++) {
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
    recursos->documentoIntro.id = 0;

    // Inicializar sons
    recursos->sfxTiroPistol.frameCount = 0;
    recursos->sfxTiroShotgun.frameCount = 0;
    recursos->sfxZumbiMorte.frameCount = 0;
    recursos->sfxJogadorDano.frameCount = 0;
    recursos->sfxJogadorMorte.frameCount = 0;
    recursos->sfxPorta.frameCount = 0;
    recursos->sfxMoeda.frameCount = 0;
    recursos->sfxCompra.frameCount = 0;
    recursos->sfxCompraNegado.frameCount = 0;
    recursos->sfxReloadPistol.frameCount = 0;
    recursos->sfxReloadShotgun.frameCount = 0;
    recursos->sfxGunClick.frameCount = 0;
    recursos->sfxBossFase1.frameCount = 0;
    recursos->sfxBossFase2.frameCount = 0;
    recursos->sfxBossFase3.frameCount = 0;
    recursos->sfxBossMorte.frameCount = 0;
    recursos->sfxGarotaGrito.frameCount = 0;
    recursos->sfxGarotaSuspiro.frameCount = 0;
    
    // Inicializar músicas
    recursos->musicMenu.frameCount = 0;
    recursos->musicGameplay.frameCount = 0;
    recursos->musicBossFight.frameCount = 0;

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
    carregarTextura(&recursos->texturasTiles[TILE_MERCADO], "assets/tiles/Mercado.png", "assets/tiles/predio_vermelho.png", "Mercado");
    carregarTextura(&recursos->texturasTiles[TILE_POSTO], "assets/tiles/Posto.png", "assets/tiles/predio_azul.png", "Posto");
    carregarTextura(&recursos->texturasTiles[TILE_LABORATORIO], "assets/tiles/Laboratorio.png", "assets/tiles/predio_verde.png", "Laboratorio");
    carregarTextura(&recursos->texturasTiles[TILE_PREDIO_AMARELO], "assets/tiles/predio_amarelo.png", NULL, "Predio Amarelo");

    carregarTextura(&recursos->texturasTiles[TILE_CHAO_MERCADO], "assets/tiles/Chao mercado.png", NULL, "Chao Mercado");
    recursos->chaoMercado = recursos->texturasTiles[TILE_CHAO_MERCADO];

    carregarTextura(&recursos->texturasTiles[TILE_PRATELEIRA_MERCADO], "assets/tiles/prateleira mercado.png", NULL, "Prateleira Mercado");
    carregarTextura(&recursos->texturasTiles[TILE_CAIXA_MERCADO], "assets/tiles/caixa mercado.png", NULL, "Caixa Mercado");
    carregarTextura(&recursos->texturasTiles[TILE_PORTA_MERCADO], "assets/tiles/porta do mercado.png", NULL, "Porta Mercado");
    carregarTextura(&recursos->texturasTiles[TILE_CHAO_DEPOSITO], "assets/tiles/Chao deposito.png", NULL, "Chao Deposito");
    carregarTextura(&recursos->texturasTiles[TILE_LOJA], "assets/tiles/Loja.png", NULL, "Loja");
    carregarTextura(&recursos->texturasTiles[TILE_CHAO_BANHEIRO], "assets/tiles/Banheiro.png", NULL, "Chao Banheiro");

    printf("    * Tile 11 (TILE_PORTA_LAB) configurado como ponto de interacao invisivel\n");
    printf("    * Tile 12 (TILE_PAREDE_INVISIVEL) configurado como parede de colisao invisivel\n");
    printf("    * Tile 13 (TILE_PORTA_BANHEIRO) configurado como porta invisivel\n");

    printf("  - Carregando texturas do jogador...\n");
    carregarTextura(&recursos->jogadorDireita, "assets/avatar/direita frente.png", NULL, "Direita/Frente");
    carregarTextura(&recursos->jogadorEsquerda, "assets/avatar/esquerda frente.png", NULL, "Esquerda/Frente");
    carregarTextura(&recursos->jogadorTras, "assets/avatar/costas direita.png", NULL, "Costas Direita");
    carregarTextura(&recursos->jogadorTrasCostasEsquerda, "assets/avatar/costas esquerda.png", NULL, "Costas Esquerda");
    carregarTextura(&recursos->jogadorFrente, "assets/avatar/direita frente.png", NULL, "Frente");

    printf("  - Carregando texturas da menina...\n");
    carregarTextura(&recursos->meninaDeitada, "assets/menina/menina deitada.png", NULL, "Menina Deitada");
    carregarTextura(&recursos->meninaFrenteDireita, "assets/menina/menina frente direita.png", NULL, "Menina Frente Direita");
    carregarTextura(&recursos->meninaFrenteEsquerda, "assets/menina/menina frente esquerda.png", NULL, "Menina Frente Esquerda");
    carregarTextura(&recursos->meninaCostas, "assets/menina/menina costas.png", NULL, "Menina Costas");
    carregarTextura(&recursos->meninaAtirandoDireita, "assets/menina/Menina atirando direita.png", NULL, "Menina Atirando Direita");
    carregarTextura(&recursos->meninaAtirandoEsquerda, "assets/menina/Menina atirando esquerda.png", NULL, "Menina Atirando Esquerda");

    printf("  - Carregando texturas da escrivaninha e relatório...\n");
    carregarTextura(&recursos->mesaComFolha, "assets/tiles/Mesa com folha.png", NULL, "Mesa com Folha");
    carregarTextura(&recursos->mesaSemFolha, "assets/tiles/Mesa sem folha.png", NULL, "Mesa sem Folha");
    carregarTextura(&recursos->relatorio, "assets/tiles/Relatorio.png", NULL, "Relatório");
    carregarTextura(&recursos->documentoIntro, "assets/tiles/DocumentoIntro.png", NULL, "Documento Introdutório");

    printf("  - Carregando texturas da porta do laboratório...\n");
    carregarTextura(&recursos->portaLabTrancada, "assets/tiles/Porta lab.png", NULL, "Porta Lab Trancada");
    carregarTextura(&recursos->portaLabAberta, "assets/tiles/Porta lab aberta.png", NULL, "Porta Lab Aberta");

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
    const char* tiposBoss[] = {"prowler", "hunter", "abomination", "pericles"};
    const char* direcoesBoss[] = {"frente", "costas", "esquerda", "direita"};

    for (int i = 0; i < 4; i++) {
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

        // Se Pericles não tiver sprites, usar as do Prowler como placeholder
        if (i == 3) {
            for (int j = 0; j < 4; j++) {
                if (!texturaValida(recursos->bosses[3][j]) && texturaValida(recursos->bosses[0][j])) {
                    recursos->bosses[3][j] = recursos->bosses[0][j];
                    printf("    * Usando sprite do Prowler como placeholder para Pericles %s\n", direcoesBoss[j]);
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

    printf("  - Carregando sons...\n");
    
    if (!IsAudioDeviceReady()) {
        printf("    ! AVISO: Audio nao esta pronto! Sons nao serao carregados.\n");
        return;
    }
    
    if (FileExists("assets/audio/sfx/tiro_pistol.wav")) {
        recursos->sfxTiroPistol = LoadSound("assets/audio/sfx/tiro_pistol.wav");
        printf("    * SFX Tiro Pistol: carregado (frames: %u)\n", recursos->sfxTiroPistol.frameCount);
    } else {
        printf("    ! SFX Tiro Pistol: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/tiro_shotgun.wav")) {
        recursos->sfxTiroShotgun = LoadSound("assets/audio/sfx/tiro_shotgun.wav");
        printf("    * SFX Tiro Shotgun: carregado (frames: %u)\n", recursos->sfxTiroShotgun.frameCount);
    } else {
        printf("    ! SFX Tiro Shotgun: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/zombie_morte.wav")) {
        recursos->sfxZumbiMorte = LoadSound("assets/audio/sfx/zombie_morte.wav");
        printf("    * SFX Zumbi Morte: carregado (frames: %u)\n", recursos->sfxZumbiMorte.frameCount);
    } else {
        printf("    ! SFX Zumbi Morte: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/dano_jogador.wav")) {
        recursos->sfxJogadorDano = LoadSound("assets/audio/sfx/dano_jogador.wav");
        printf("    * SFX Jogador Dano: carregado (frames: %u)\n", recursos->sfxJogadorDano.frameCount);
    } else {
        printf("    ! SFX Jogador Dano: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/jogador_morte.wav")) {
        recursos->sfxJogadorMorte = LoadSound("assets/audio/sfx/jogador_morte.wav");
        printf("    * SFX Jogador Morte: carregado (frames: %u)\n", recursos->sfxJogadorMorte.frameCount);
    } else {
        printf("    ! SFX Jogador Morte: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/abrir_porta.mp3")) {
        recursos->sfxPorta = LoadSound("assets/audio/sfx/abrir_porta.mp3");
        SetSoundVolume(recursos->sfxPorta, 1.5f);
        printf("    * SFX Porta (Transicao): carregado (frames: %u)\n", recursos->sfxPorta.frameCount);
    } else {
        printf("    ! SFX Porta (Transicao): arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/pegar_moeda.wav")) {
        recursos->sfxMoeda = LoadSound("assets/audio/sfx/pegar_moeda.wav");
        printf("    * SFX Moeda: carregado (frames: %u)\n", recursos->sfxMoeda.frameCount);
    } else {
        printf("    ! SFX Moeda: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/comprar_loja.wav")) {
        recursos->sfxCompra = LoadSound("assets/audio/sfx/comprar_loja.wav");
        printf("    * SFX Compra: carregado (frames: %u)\n", recursos->sfxCompra.frameCount);
    } else {
        printf("    ! SFX Compra: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/som_loja_negado.mp3")) {
        recursos->sfxCompraNegado = LoadSound("assets/audio/sfx/som_loja_negado.mp3");
        printf("    * SFX Compra Negado: carregado (frames: %u)\n", recursos->sfxCompraNegado.frameCount);
    } else {
        printf("    ! SFX Compra Negado: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/reload_pistola.wav")) {
        recursos->sfxReloadPistol = LoadSound("assets/audio/sfx/reload_pistola.wav");
        printf("    * SFX Reload Pistol: carregado (frames: %u)\n", recursos->sfxReloadPistol.frameCount);
    } else {
        printf("    ! SFX Reload Pistol: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/reload_shotgun.wav")) {
        recursos->sfxReloadShotgun = LoadSound("assets/audio/sfx/reload_shotgun.wav");
        printf("    * SFX Reload Shotgun: carregado (frames: %u)\n", recursos->sfxReloadShotgun.frameCount);
    } else {
        printf("    ! SFX Reload Shotgun: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/gun_click.wav")) {
        recursos->sfxGunClick = LoadSound("assets/audio/sfx/gun_click.wav");
        printf("    * SFX Gun Click: carregado (frames: %u)\n", recursos->sfxGunClick.frameCount);
    } else {
        printf("    ! SFX Gun Click: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/som_boss_da_fase1.mp3")) {
        recursos->sfxBossFase1 = LoadSound("assets/audio/sfx/som_boss_da_fase1.mp3");
        SetSoundVolume(recursos->sfxBossFase1, 0.7f);
        printf("    * SFX Boss Fase 1: carregado (frames: %u)\n", recursos->sfxBossFase1.frameCount);
    } else {
        printf("    ! SFX Boss Fase 1: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/som_boss_da_fase2.mp3")) {
        recursos->sfxBossFase2 = LoadSound("assets/audio/sfx/som_boss_da_fase2.mp3");
        SetSoundVolume(recursos->sfxBossFase2, 0.7f);
        printf("    * SFX Boss Fase 2: carregado (frames: %u)\n", recursos->sfxBossFase2.frameCount);
    } else {
        printf("    ! SFX Boss Fase 2: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/som_boss_da_fase3.wav")) {
        recursos->sfxBossFase3 = LoadSound("assets/audio/sfx/som_boss_da_fase3.wav");
        SetSoundVolume(recursos->sfxBossFase3, 0.7f);
        printf("    * SFX Boss Fase 3: carregado (frames: %u)\n", recursos->sfxBossFase3.frameCount);
    } else {
        printf("    ! SFX Boss Fase 3: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/som_morte_bosses.mp3")) {
        recursos->sfxBossMorte = LoadSound("assets/audio/sfx/som_morte_bosses.mp3");
        printf("    * SFX Boss Morte: carregado (frames: %u)\n", recursos->sfxBossMorte.frameCount);
    } else {
        printf("    ! SFX Boss Morte: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/som_garota_grito.mp3")) {
        recursos->sfxGarotaGrito = LoadSound("assets/audio/sfx/som_garota_grito.mp3");
        SetSoundVolume(recursos->sfxGarotaGrito, 0.9f);
        printf("    * SFX Garota Grito: carregado (frames: %u)\n", recursos->sfxGarotaGrito.frameCount);
    } else {
        printf("    ! SFX Garota Grito: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/sfx/garota_suspiro.mp3")) {
        recursos->sfxGarotaSuspiro = LoadSound("assets/audio/sfx/garota_suspiro.mp3");
        SetSoundVolume(recursos->sfxGarotaSuspiro, 0.7f);
        printf("    * SFX Garota Suspiro: carregado (frames: %u)\n", recursos->sfxGarotaSuspiro.frameCount);
    } else {
        printf("    ! SFX Garota Suspiro: arquivo nao encontrado\n");
    }
    
    printf("  - Carregando musicas...\n");
    if (FileExists("assets/audio/music/musica_menu.mp3")) {
        recursos->musicMenu = LoadMusicStream("assets/audio/music/musica_menu.mp3");
        printf("    * Musica Menu: carregada\n");
    } else {
        printf("    ! Musica Menu: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/music/musica_da_gameplay.mp3")) {
        recursos->musicGameplay = LoadMusicStream("assets/audio/music/musica_da_gameplay.mp3");
        printf("    * Musica Gameplay: carregada\n");
    } else {
        printf("    ! Musica Gameplay: arquivo nao encontrado\n");
    }
    if (FileExists("assets/audio/music/musica_bossfight.mp3")) {
        recursos->musicBossFight = LoadMusicStream("assets/audio/music/musica_bossfight.mp3");
        printf("    * Musica Boss Fight: carregada\n");
    } else {
        printf("    ! Musica Boss Fight: arquivo nao encontrado\n");
    }

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

    // Descarregar texturas da menina
    if (texturaValida(recursos->meninaDeitada)) {
        UnloadTexture(recursos->meninaDeitada);
        recursos->meninaDeitada.id = 0;
    }
    if (texturaValida(recursos->meninaFrenteDireita)) {
        UnloadTexture(recursos->meninaFrenteDireita);
        recursos->meninaFrenteDireita.id = 0;
    }
    if (texturaValida(recursos->meninaFrenteEsquerda)) {
        UnloadTexture(recursos->meninaFrenteEsquerda);
        recursos->meninaFrenteEsquerda.id = 0;
    }
    if (texturaValida(recursos->meninaCostas)) {
        UnloadTexture(recursos->meninaCostas);
        recursos->meninaCostas.id = 0;
    }
    if (texturaValida(recursos->meninaAtirandoDireita)) {
        UnloadTexture(recursos->meninaAtirandoDireita);
        recursos->meninaAtirandoDireita.id = 0;
    }
    if (texturaValida(recursos->meninaAtirandoEsquerda)) {
        UnloadTexture(recursos->meninaAtirandoEsquerda);
        recursos->meninaAtirandoEsquerda.id = 0;
    }

    // Descarregar texturas da escrivaninha e relatório
    if (texturaValida(recursos->mesaComFolha)) {
        UnloadTexture(recursos->mesaComFolha);
        recursos->mesaComFolha.id = 0;
    }
    if (texturaValida(recursos->mesaSemFolha)) {
        UnloadTexture(recursos->mesaSemFolha);
        recursos->mesaSemFolha.id = 0;
    }
    if (texturaValida(recursos->relatorio)) {
        UnloadTexture(recursos->relatorio);
        recursos->relatorio.id = 0;
    }
    if (texturaValida(recursos->documentoIntro)) {
        UnloadTexture(recursos->documentoIntro);
        recursos->documentoIntro.id = 0;
    }

    // Descarregar texturas da porta do laboratório
    if (texturaValida(recursos->portaLabTrancada)) {
        UnloadTexture(recursos->portaLabTrancada);
        recursos->portaLabTrancada.id = 0;
    }
    if (texturaValida(recursos->portaLabAberta)) {
        UnloadTexture(recursos->portaLabAberta);
        recursos->portaLabAberta.id = 0;
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

    for (int i = 0; i < 4; i++) {
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

    // Descarregar sons
    if (recursos->sfxTiroPistol.frameCount > 0) UnloadSound(recursos->sfxTiroPistol);
    if (recursos->sfxTiroShotgun.frameCount > 0) UnloadSound(recursos->sfxTiroShotgun);
    if (recursos->sfxZumbiMorte.frameCount > 0) UnloadSound(recursos->sfxZumbiMorte);
    if (recursos->sfxJogadorDano.frameCount > 0) UnloadSound(recursos->sfxJogadorDano);
    if (recursos->sfxJogadorMorte.frameCount > 0) UnloadSound(recursos->sfxJogadorMorte);
    if (recursos->sfxPorta.frameCount > 0) UnloadSound(recursos->sfxPorta);
    if (recursos->sfxMoeda.frameCount > 0) UnloadSound(recursos->sfxMoeda);
    if (recursos->sfxCompra.frameCount > 0) UnloadSound(recursos->sfxCompra);
    if (recursos->sfxCompraNegado.frameCount > 0) UnloadSound(recursos->sfxCompraNegado);
    if (recursos->sfxReloadPistol.frameCount > 0) UnloadSound(recursos->sfxReloadPistol);
    if (recursos->sfxReloadShotgun.frameCount > 0) UnloadSound(recursos->sfxReloadShotgun);
    if (recursos->sfxGunClick.frameCount > 0) UnloadSound(recursos->sfxGunClick);
    if (recursos->sfxBossFase1.frameCount > 0) UnloadSound(recursos->sfxBossFase1);
    if (recursos->sfxBossFase2.frameCount > 0) UnloadSound(recursos->sfxBossFase2);
    if (recursos->sfxBossFase3.frameCount > 0) UnloadSound(recursos->sfxBossFase3);
    if (recursos->sfxBossMorte.frameCount > 0) UnloadSound(recursos->sfxBossMorte);
    if (recursos->sfxGarotaGrito.frameCount > 0) UnloadSound(recursos->sfxGarotaGrito);
    if (recursos->sfxGarotaSuspiro.frameCount > 0) UnloadSound(recursos->sfxGarotaSuspiro);
    
    // Descarregar músicas
    if (recursos->musicMenu.frameCount > 0) UnloadMusicStream(recursos->musicMenu);
    if (recursos->musicGameplay.frameCount > 0) UnloadMusicStream(recursos->musicGameplay);
    if (recursos->musicBossFight.frameCount > 0) UnloadMusicStream(recursos->musicBossFight);

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
