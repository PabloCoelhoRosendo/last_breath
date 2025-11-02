// src/arquivo.c

// Definição do tamanho do ranking
#define MAX_SCORES 5

// Protótipos das funções que você já criou:
void loadScores(int scores[], int count);
void saveScores(int scores[], int count);

// Função para verificar, ordenar e salvar o High Score
void checkAndSaveHighScore(int newScore) {
    int scores[MAX_SCORES]; // Array para armazenar os 5 melhores scores lidos

    // 1. Carregar os scores existentes
    loadScores(scores, MAX_SCORES); // Chama a função que lê os scores do arquivo scores.txt

    // 2. Verificar se o novo score entra no ranking
    // O ranking é ordenado do maior para o menor. O menor score é o último elemento do array.
    if (newScore > scores[MAX_SCORES - 1]) {
        
        // --- O score é um High Score, precisamos inseri-lo ---

        // 3. Encontrar a posição correta e deslocar os scores menores
        int i;
        // Loop que começa do último elemento (MAX_SCORES - 1, o menor score) e vai até o primeiro (0)
        for (i = MAX_SCORES - 1; i >= 0; i--) {
            
            // Condição de comparação: O novo score é maior que o score atual?
            if (newScore > scores[i]) {
                
                // Se o score atual (scores[i]) não for o último do ranking (para evitar erro de índice)
                if (i < MAX_SCORES - 1) {
                    // Deslocamento: O score atual desce uma posição no ranking
                    scores[i + 1] = scores[i]; 
                }
            } else {
                // Se o newScore não for maior que scores[i], significa que encontramos
                // a posição correta de inserção (o score anterior é maior que o newScore).
                break; 
            }
        }
        
        // 4. Inserir o novo score na posição correta
        // O loop parou no índice 'i', então a posição de inserção é 'i + 1'.
        scores[i + 1] = newScore;

        printf("HIGH SCORE! Novo score de %d adicionado ao ranking.\n", newScore);

        // 5. Salvar o novo ranking ordenado no arquivo
        saveScores(scores, MAX_SCORES); // Chama a função que sobrescreve o arquivo scores.txt
    } else {
        // Se a pontuação não é maior que o menor score do ranking, ela é ignorada.
        printf("Score de %d nao entrou no Top %d. Ranking inalterado.\n", newScore, MAX_SCORES);
    }
}