// include/arquivo.h
// Header para funções de ranking/pontuação do jogo

// Proteção contra inclusão múltipla
#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>  // Para fopen, fclose, fprintf, fscanf

// Top 5 melhores pontuações
#define MAX_SCORES 5

// Carrega pontuações do arquivo
void loadScores(int scores[], int count);

// Salva pontuações no arquivo
void saveScores(int scores[], int count);

// Verifica se a pontuação entra no ranking e atualiza
void checkAndSaveHighScore(int newScore);

#endif // ARQUIVO_H
