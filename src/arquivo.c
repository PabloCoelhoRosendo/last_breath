#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SCORES 5

void loadTimes(float times[], int count) {
    FILE *file = fopen("scores.txt", "r");

    for (int i = 0; i < count; i++) {
        times[i] = 999999.999f;
    }

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
            fprintf(file, "%.3f\n", times[i]);
        }
        fclose(file);
    }
}

void checkAndSaveTime(float newTime) {
    float times[MAX_SCORES];

    loadTimes(times, MAX_SCORES);

    if (newTime < times[MAX_SCORES - 1]) {
        int i;
        for (i = MAX_SCORES - 1; i >= 0; i--) {
            if (newTime < times[i]) {
                if (i < MAX_SCORES - 1) {
                    times[i + 1] = times[i];
                }
            } else {
                break;
            }
        }

        times[i + 1] = newTime;

        int minutos = (int)newTime / 60;
        float segundos = fmod(newTime, 60.0f);
        printf("NOVO RECORDE! Tempo de %d:%06.3f adicionado ao ranking.\n",
               minutos, segundos);

        saveTimes(times, MAX_SCORES);
    } else {
        int minutos = (int)newTime / 60;
        float segundos = fmod(newTime, 60.0f);
        printf("Tempo de %d:%06.3f nao entrou no Top %d.\n",
               minutos, segundos, MAX_SCORES);
    }
}