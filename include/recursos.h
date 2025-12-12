#ifndef RECURSOS_H
#define RECURSOS_H

#include "raylib.h"
#include "mapa.h"

typedef struct {
    Texture2D texturasTiles[MAX_TIPOS_TILE];

    Texture2D chaoMercado;
    Texture2D chaoRua;
    Texture2D chaoLab;

    Texture2D jogadorFrente;
    Texture2D jogadorTras;
    Texture2D jogadorTrasCostasEsquerda;
    Texture2D jogadorEsquerda;
    Texture2D jogadorDireita;

    Texture2D zumbis[5][4];

    Texture2D bosses[3][4];

    Texture2D fundoMapa;

    Texture2D texturaBala;

    Texture2D texturaChave;

    Texture2D texturaShotgun;
    Texture2D texturaSMG;

    // Sprites da menina
    Texture2D meninaDeitada;
    Texture2D meninaFrenteDireita;
    Texture2D meninaFrenteEsquerda;
    Texture2D meninaCostas;
    Texture2D meninaAtirandoDireita;
    Texture2D meninaAtirandoEsquerda;

    // Texturas da escrivaninha e relatório
    Texture2D mesaComFolha;
    Texture2D mesaSemFolha;
    Texture2D relatorio;

    // Sons do jogo
    Sound sfxTiroPistol;
    Sound sfxTiroShotgun;
    Sound sfxZumbiMorte;
    Sound sfxJogadorDano;
    Sound sfxJogadorMorte;
    Sound sfxPorta;
    Sound sfxMoeda;
    Sound sfxCompra;
    Sound sfxCompraNegado;
    Sound sfxReloadPistol;
    Sound sfxReloadShotgun;
    Sound sfxGunClick;
    Sound sfxBossFase1;
    Sound sfxBossFase2;
    Sound sfxBossFase3;
    Sound sfxBossMorte;
    Sound sfxGarotaGrito;
    Sound sfxGarotaSuspiro;
    
    // Músicas do jogo
    Music musicMenu;
    Music musicGameplay;
    Music musicBossFight;

} Recursos;

Recursos* criarRecursos(void);
void carregarRecursos(Recursos* recursos);
void descarregarRecursos(Recursos* recursos);
void destruirRecursos(Recursos* recursos);

Texture2D obterTexturaTile(Recursos* recursos, int tipoTile);
bool texturaValida(Texture2D textura);

#endif
