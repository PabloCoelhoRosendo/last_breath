# 🧟 LAST BREATH - Zombie Survival Game

Last Breath é um jogo de sobrevivência desenvolvido por alunos de Ciência da Computação do segundo período, programado em C com o auxílio da biblioteca Raylib. O jogo se passa em um apocalipse zumbi, onde você precisa sobreviver a hordas de mortos-vivos, enfrentar chefões temíveis e descobrir a cura que pode salvar a humanidade. Junte-se a nós nessa missão que envolve muita ação, estratégia e sobrevivência! 🧟‍♂️💀🔫

**Projeto da disciplina: Programação Imperativa e Funcional**  
**Semestre:** 2025.2

---

## 📖 Sobre o Jogo

Last Breath é um jogo de sobrevivência contra zumbis onde o jogador precisa atravessar três fases distintas, enfrentando hordas progressivas de inimigos e chefões poderosos. Com sistema de armas, pathfinding inteligente e mecânicas de bullet-hell, cada fase oferece um desafio único até a descoberta da cura.

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

- ✅ **Structs** - Player, Zombie, Bala, Boss, Item
- ✅ **Ponteiros** - Manipulação de listas encadeadas
- ✅ **Alocação Dinâmica** - malloc() e free() para gerenciar inimigos
- ✅ **Listas Encadeadas** - Sistema de zumbis, balas e bosses
- ✅ **Arquivos** - Sistema de ranking (leitura/escrita)
- ✅ **Matrizes** - Mapas 32x24 tiles
- ✅ **Funções** - Modularização completa do código

---

## 👑 Requisitos

- [Raylib](https://www.raylib.com/) 5.0+
- Compilador C (gcc)
- Make
- Git

---

## 👑 Instalação das Dependências

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential git libraylib-dev libgl1-mesa-dev libx11-dev
```

#### macOS

Instale o [Homebrew](https://brew.sh/) e depois:
```bash
brew install raylib git
```

#### Windows

Recomenda-se usar o [w64devkit](https://github.com/skeeto/w64devkit/releases):

1. Baixe o w64devkit mais recente
2. Extraia em uma pasta (ex: `C:\w64devkit`)
3. Adicione ao PATH do sistema: `C:\w64devkit\bin`
4. A Raylib já está incluída no w64devkit

---

## 🚀 Instalação e Execução

### Passo 1: Clonar o Repositório
```bash
git clone https://github.com/seu-usuario/Last_Breath.git
cd Last_Breath
```

### Passo 2: Compilação

Para compilar o jogo, execute:
```bash
make
```

### Passo 3: Executando o Jogo

Após compilar, rode:
```bash
make run
```

ou execute diretamente:
```bash
./last_breath          # Linux/macOS
.\last_breath.exe      # Windows
```

### Limpeza dos Arquivos de Build
```bash
make clean
```

---

## 🔧 Comandos do Makefile
```bash
make          # Compila o jogo
make run      # Compila e executa
make clean    # Remove arquivos compilados
make help     # Mostra ajuda com todos os comandos
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