#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>

#define MAX_SCORES 5

void loadTimes(float times[], int count);

void saveTimes(float times[], int count);

void checkAndSaveTime(float newTime);

#endif
