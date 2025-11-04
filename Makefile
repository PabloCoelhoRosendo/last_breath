# Makefile para Last Breath (Windows e Linux)

# Detectar o sistema operacional
ifeq ($(OS),Windows_NT)
    # Windows
    TARGET = last_breath.exe
    RM = del /Q
    LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
    # Usar o w64devkit se disponível
    CC = gcc
else
    # Linux/Unix
    TARGET = last_breath
    RM = rm -f
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    CC = gcc
endif

# Diretórios
SRC_DIR = src
INCLUDE_DIR = include

# Arquivos fonte
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/jogo.c $(SRC_DIR)/arquivo.c

# Flags do compilador
CFLAGS = -Wall -std=c99 -I$(INCLUDE_DIR)

# Regra padrão
all: $(TARGET)

# Compilar o jogo
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)
	@echo ""
	@echo "=========================================="
	@echo "  ✅ Compilação concluída!"
	@echo "=========================================="
	@echo ""
ifeq ($(OS),Windows_NT)
	@echo "Execute: $(TARGET)"
else
	@echo "Execute: ./$(TARGET)"
endif

# Compilar e executar
run: $(TARGET)
ifeq ($(OS),Windows_NT)
	.\$(TARGET)
else
	./$(TARGET)
endif

# Limpar arquivos compilados
clean:
ifeq ($(OS),Windows_NT)
	@if exist $(TARGET) del /Q $(TARGET)
	@if exist *.o del /Q *.o
else
	$(RM) $(TARGET) last_breath last_breath.exe last_breath_linux *.o
endif
	@echo "Arquivos limpos!"

# Ajuda
help:
	@echo "Comandos disponíveis:"
	@echo "  make          - Compila o jogo"
	@echo "  make run      - Compila e executa"
	@echo "  make clean    - Remove arquivos compilados"
	@echo "  make help     - Mostra esta mensagem"

.PHONY: all run clean help
