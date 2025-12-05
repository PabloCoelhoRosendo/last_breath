#ifndef HORDA_H
#define HORDA_H

#include "boss.h"  // Para ter acesso ao tipo Boss

// EstadoHorda, Player, Zumbi definidos em jogo.h
// Este header DEVE ser incluído após jogo.h

// Funções de Horda
void iniciarHorda(Player *jogador, int numeroHorda);
void atualizarHorda(Player *jogador, Zumbi **zumbis, Boss **bosses, float deltaTime);

#endif
