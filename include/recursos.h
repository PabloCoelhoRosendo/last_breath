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

} Recursos;

Recursos* criarRecursos(void);
void carregarRecursos(Recursos* recursos);
void descarregarRecursos(Recursos* recursos);
void destruirRecursos(Recursos* recursos);

Texture2D obterTexturaTile(Recursos* recursos, int tipoTile);
bool texturaValida(Texture2D textura);

#endif
