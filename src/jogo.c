// src/jogo.c

#include "raylib.h"
#include "jogo.h"
#include "arquivo.h"

#include <stdlib.h> // Para malloc e free (Requisito: Alocação Dinâmica de Memória)
#include <stdio.h>

//  Matriz do Mapa (- Leo)
    
void mapa(int mapa[TAMANHO_MAPA][TAMANHO_MAPA]){
    for (int i = 0; i < TAMANHO_MAPA; i++){
        for(int j = 0; j <  TAMANHO_MAPA; j++){
            mapa[i][j] = 0;
        }
    }
}

// --- Funções do Módulo de Balas (- Pablo) ---

// Função para Alocação Dinâmica e inserção de uma nova Bala na Lista Encadeada
void adicionarBala(Bala **cabeca, Vector2 posInicial, Vector2 alvo) {
    
    // 1. Alocação Dinâmica de Memória (Requisito: Alocação Dinâmica)
    Bala *novaBala = (Bala *)malloc(sizeof(Bala));

    if (novaBala == NULL) {
        printf("ERRO: Falha na alocacao de memoria para nova Bala!\n");
        return;
    }

    // 2. Inicializar a nova bala
    novaBala->posicao = posInicial;

    // ... (Cálculo de velocidade da bala Raylib)

    // 3. Inserir a bala no início da lista (Requisito: Ponteiros e Lista Encadeada)
    novaBala->proximo = *cabeca;
    *cabeca = novaBala;
}

// ... (Implementação de atualizarBalas, que fará a remoção com 'free')

// ... (Implementação de iniciarJogo, atualizarJogo e desenharJogo)