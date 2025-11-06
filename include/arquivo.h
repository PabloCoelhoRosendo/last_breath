// include/arquivo.h
// Header para funções de ranking/pontuação do jogo

// Proteção contra inclusão múltipla
#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>  // Para fopen, fclose, fprintf, fscanf

// Top 5 melhores pontuações
#define MAX_SCORES 5

// Carrega tempos do arquivo
void loadTimes(float times[], int count);

// Salva tempos no arquivo
void saveTimes(float times[], int count);

// Verifica se o tempo entra no ranking e atualiza (menor tempo = melhor)
void checkAndSaveTime(float newTime);

#endif // ARQUIVO_H
