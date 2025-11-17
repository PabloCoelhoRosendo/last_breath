#ifndef RECURSOS_H
#define RECURSOS_H

#include "raylib.h"
#include "mapa.h"

// Estrutura para gerenciar todos os recursos do jogo
typedef struct {
    // Texturas de tiles
    Texture2D texturasTiles[MAX_TIPOS_TILE];

    // Texturas de chão por fase
    Texture2D chaoMercado;  // Fase 1
    Texture2D chaoRua;      // Fase 2
    Texture2D chaoLab;      // Fase 3 (opcional)

    // Texturas do jogador (4 direções)
    Texture2D jogadorFrente;
    Texture2D jogadorTras;
    Texture2D jogadorEsquerda;
    Texture2D jogadorDireita;

    // Texturas de zumbis (5 tipos, 4 direções cada)
    Texture2D zumbis[5][4];

    // Texturas de bosses (3 tipos, 4 direções cada)
    Texture2D bosses[3][4];

    // Textura de fundo (se necessária)
    Texture2D fundoMapa;

} Recursos;

// Funções de gerenciamento de recursos
Recursos* criarRecursos(void);
void carregarRecursos(Recursos* recursos);
void descarregarRecursos(Recursos* recursos);
void destruirRecursos(Recursos* recursos);

// Funções auxiliares
Texture2D obterTexturaTile(Recursos* recursos, int tipoTile);
bool texturaValida(Texture2D textura);

#endif // RECURSOS_H
