#include "jogo.h"     // Deve vir ANTES para ter as definições completas
#include "horda.h"
#include "boss.h"

#include <stdio.h>

void iniciarHorda(Player *jogador, int numeroHorda) {
    jogador->hordaAtual = numeroHorda;
    jogador->estadoHorda = HORDA_EM_PROGRESSO;
    jogador->zumbisSpawnados = 0;
    jogador->bossesSpawnados = 0;
    jogador->tempoSpawn = 0.0f;  
    jogador->tempoSpawnBoss = 0.0f;  
    
    if (jogador->fase == 1) {
        jogador->bossesTotaisHorda = 0;  
        switch (numeroHorda) {
            case 1:
                jogador->zumbisTotaisHorda = 5;  
                break;
            case 2:
                jogador->zumbisTotaisHorda = 7;  
                break;
            case 3:
                jogador->zumbisTotaisHorda = 2;  
                jogador->bossesTotaisHorda = 1;  
                break;
            default:
                jogador->zumbisTotaisHorda = 5;
                break;
        }
    } else if (jogador->fase == 2) {
        switch (numeroHorda) {
            case 1:
                jogador->zumbisTotaisHorda = 2;   
                jogador->bossesTotaisHorda = 1;   
                break;
            case 2:
                jogador->zumbisTotaisHorda = 2;   
                jogador->bossesTotaisHorda = 1;   
                break;
            case 3:
                jogador->zumbisTotaisHorda = 0;   
                jogador->bossesTotaisHorda = 2;   
                break;
            default:
                jogador->zumbisTotaisHorda = 2;
                jogador->bossesTotaisHorda = 1;
                break;
        }
    } else {
        jogador->zumbisTotaisHorda = 5;
        jogador->bossesTotaisHorda = 0;
    }
    
    jogador->zumbisRestantes = jogador->zumbisTotaisHorda;
    
    printf("=== HORDA %d INICIADA (FASE %d) ===\n", numeroHorda, jogador->fase);
    printf("Zumbis a spawnar: %d\n", jogador->zumbisTotaisHorda);
    printf("Bosses a spawnar: %d\n", jogador->bossesTotaisHorda);
}

void atualizarHorda(Player *jogador, Zumbi **zumbis, Boss **bosses, float deltaTime) {
    if (jogador->vida <= 0 || jogador->jogoVencido) {
        return;
    }
    
    if (jogador == NULL || zumbis == NULL || bosses == NULL) {
        return;
    }
    
    int zumbisVivos = 0;
    Zumbi *z = *zumbis;
    while (z != NULL) {
        zumbisVivos++;
        z = z->proximo;
    }
    jogador->zumbisRestantes = zumbisVivos;
    
    int bossesVivos = contarBossesVivos(*bosses);
    
    switch (jogador->estadoHorda) {
        case HORDA_NAO_INICIADA:
            if ((jogador->fase == 1 || jogador->fase == 2) && jogador->hordaAtual == 0) {
                iniciarHorda(jogador, 1);
            }
            break;
            
        case HORDA_EM_PROGRESSO:
            if (zumbisVivos == 0 && jogador->zumbisSpawnados >= jogador->zumbisTotaisHorda &&
                bossesVivos == 0 && jogador->bossesSpawnados >= jogador->bossesTotaisHorda) {
                jogador->estadoHorda = HORDA_COMPLETA;
                printf("=== HORDA %d COMPLETA! ===\n", jogador->hordaAtual);
                
                int maxHordas = 3; 
                if ((jogador->fase == 1 || jogador->fase == 2) && jogador->hordaAtual < maxHordas) {
                    jogador->estadoHorda = HORDA_INTERVALO;
                    jogador->tempoIntervalo = 10.0f;  
                    printf("Proxima horda em 10 segundos...\n");
                }
            }
            break;
            
        case HORDA_COMPLETA:
            break;
            
        case HORDA_INTERVALO:
            jogador->tempoIntervalo -= deltaTime;
            
            if (jogador->tempoIntervalo <= 0.0f) {
                iniciarHorda(jogador, jogador->hordaAtual + 1);
            }
            break;
    }
}
