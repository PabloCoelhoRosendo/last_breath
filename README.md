# 🧟 LAST BREATH

**Projeto da disciplina: Programação Imperativa e Funcional**  
**Semestre:** 2025.2  


## 📖 Sobre o Jogo

Last Breath é um jogo de sobrevivência contra zumbis desenvolvido em C utilizando a biblioteca Raylib. O jogador precisa sobreviver a hordas de zumbis cada vez mais difíceis, coletar itens, enfrentar chefões e descobrir a cura que pode salvar a humanidade!

---

## ✨ Características

- **3 Fases Únicas:**
  - 🏪 Fase 1: Interior do Mercado
  - 🏙️ Fase 2: Rua com Prédios
  - 🔬 Fase 3: Laboratório Secreto

- **Sistema de Hordas:** Ondas progressivas de inimigos
- **3 Tipos de Bosses:**
  - 🐺 Prowler (Fase 1) - Ataque em área
  - 🏃 Hunter (Fase 2) - Veloz e agressivo
  - 👹 Abomination (Fase 3) - Bullet-hell estático

- **Sistema de Armas:**
  - Pistola (inicial)
  - Shotgun (dropada pelo Prowler)
  - SMG (dropada pelos Hunters)

- **Pathfinding A*:** Zumbis inteligentes que desviam de obstáculos
- **Sistema de Ranking:** Top 5 melhores tempos salvos

---

## 🎯 Conceitos de Programação Aplicados

Este projeto foi desenvolvido para demonstrar diversos conceitos da disciplina:

- ✅ **Structs** - Player, Zombie, Bala, Boss, Item
- ✅ **Ponteiros** - Manipulação de listas encadeadas
- ✅ **Alocação Dinâmica** - malloc() e free() para gerenciar inimigos
- ✅ **Listas Encadeadas** - Sistema de zumbis, balas e bosses
- ✅ **Arquivos** - Sistema de ranking (leitura/escrita)
- ✅ **Matrizes** - Mapas 32x24 tiles
- ✅ **Funções** - Modularização completa do código

---

## 🛠️ Requisitos

### Windows
- **Compilador:** GCC (recomendado: w64devkit ou MinGW)
- **Biblioteca:** Raylib 5.0+

### Linux
- **Compilador:** GCC
- **Biblioteca:** Raylib 5.0+
- **Dependências:** OpenGL, X11

---

## 🚀 Como Compilar e Executar

### Windows (com w64devkit)

1. Baixe e extraia o [w64devkit](https://github.com/skeeto/w64devkit/releases)
2. Adicione ao PATH: `C:\caminho\para\w64devkit\bin`
3. No terminal, na pasta do projeto:
```bash
make
.\last_breath.exe
```

### Linux

1. Instale as dependências:
```bash
sudo apt-get install build-essential libraylib-dev
```

2. Compile e execute:
```bash
make
./last_breath
```

### Compilação Manual (sem Makefile)

**Windows:**
```bash
gcc src/main.c src/jogo.c src/arquivo.c src/mapa.c src/recursos.c src/pathfinding.c -o last_breath.exe -Iinclude -lraylib -lopengl32 -lgdi32 -lwinmm
```

**Linux:**
```bash
gcc src/main.c src/jogo.c src/arquivo.c src/mapa.c src/recursos.c src/pathfinding.c -o last_breath -Iinclude -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

---

## 🎮 Controles

| Tecla | Ação |
|-------|------|
| **W, A, S, D** | Movimentar o jogador |
| **Mouse** | Mirar |
| **Click Esquerdo** | Atirar |
| **R** | Recarregar arma |
| **1, 2, 3** | Trocar de arma |
| **E** | Interagir (coletar itens/usar portas) |
| **ESC** | Sair do jogo |

---

## 📁 Estrutura do Projeto
```
Last_Breath/
├── assets/               # Recursos visuais
│   ├── avatar/          # Sprites do jogador
│   ├── zumbis/          # Sprites dos zumbis (5 tipos)
│   ├── bosses/          # Sprites dos chefões
│   ├── tiles/           # Texturas do mapa
│   ├── maps/            # Arquivos de mapa (.txt)
│   ├── armas/           # Sprites das armas
│   ├── bala/            # Sprite da bala
│   ├── chave/           # Sprite da chave
│   ├── logo/            # Logo do jogo
│   └── background/      # Background do menu
├── include/             # Headers (.h)
├── src/                 # Código fonte (.c)
├── Makefile            # Automação de compilação
└── README.md           # Este arquivo
```

---

## 🎯 Objetivos do Jogo

### Fase 1: Mercado
1. Sobreviva a 3 hordas de zumbis
2. Derrote o boss Prowler
3. Colete a Chave e a Shotgun
4. Use a porta para ir à Fase 2

### Fase 2: Rua
1. Sobreviva a 3 hordas (incluindo Hunters)
2. Derrote os 2 Hunters finais
3. Colete a Chave e a SMG
4. Entre no Laboratório (porta invisível abaixo do prédio verde)

### Fase 3: Laboratório
1. Sobreviva aos zumbis iniciais
2. Derrote o boss Abomination (bullet-hell!)
3. Colete a CURE
4. **🎉 VITÓRIA!**

---

## 📊 Sistema de Ranking

- Apenas **vitórias completas** (coletar a CURE) são salvas no ranking
- Top 5 melhores tempos
- Arquivo salvo em: `scores.txt`
- Formato: Minutos:Segundos.Milissegundos