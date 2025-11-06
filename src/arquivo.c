// src/arquivo.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>    // Para fmod()

// Definição do tamanho do ranking
#define MAX_SCORES 5

// Implementação das funções de arquivo
void loadTimes(float times[], int count) {
    FILE *file = fopen("scores.txt", "r");
    
    // Inicializar com valores altos (pior tempo possível)
    for (int i = 0; i < count; i++) {
        times[i] = 999999.999f;  // ~16 minutos, tempo bem alto
    }
    
    // Se o arquivo existe, carregar tempos
    if (file != NULL) {
        for (int i = 0; i < count; i++) {
            if (fscanf(file, "%f", &times[i]) != 1) {
                break;
            }
        }
        fclose(file);
    }
}

void saveTimes(float times[], int count) {
    FILE *file = fopen("scores.txt", "w");
    
    if (file != NULL) {
        for (int i = 0; i < count; i++) {
            fprintf(file, "%.3f\n", times[i]);  // Salva com 3 casas decimais
        }
        fclose(file);
    }
}

// Função para verificar, ordenar e salvar o tempo (menor = melhor)
void checkAndSaveTime(float newTime) {
    float times[MAX_SCORES]; // Array para armazenar os 5 melhores tempos

    // 1. Carregar os tempos existentes
    loadTimes(times, MAX_SCORES);

    // 2. Verificar se o novo tempo entra no ranking
    // O ranking é ordenado do menor para o maior. O maior tempo é o último elemento.
    if (newTime < times[MAX_SCORES - 1]) {
        // --- O tempo é bom o suficiente para entrar no ranking ---

        // 3. Encontrar a posição correta e deslocar os tempos maiores
        int i;
        for (i = MAX_SCORES - 1; i >= 0; i--) {
            // Condição de comparação: O novo tempo é menor que o tempo atual?
            if (newTime < times[i]) {
                if (i < MAX_SCORES - 1) {
                    // Deslocamento: O tempo atual sobe uma posição no ranking
                    times[i + 1] = times[i];
                }
            } else {
                break; 
            }
        }
        
        // 4. Inserir o novo tempo na posição correta
        times[i + 1] = newTime;

        // Formatar o tempo em minutos:segundos.milissegundos
        int minutos = (int)newTime / 60;
        float segundos = fmod(newTime, 60.0f);
        printf("NOVO RECORDE! Tempo de %d:%06.3f adicionado ao ranking.\n", 
               minutos, segundos);

        // 5. Salvar o novo ranking ordenado
        saveTimes(times, MAX_SCORES);
    } else {
        // Se o tempo não é menor que o pior tempo do ranking
        int minutos = (int)newTime / 60;
        float segundos = fmod(newTime, 60.0f);
        printf("Tempo de %d:%06.3f nao entrou no Top %d.\n", 
               minutos, segundos, MAX_SCORES);
    }
}