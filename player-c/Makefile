# Definições do compilador e flags
CC = gcc
CXX = g++
GDB = gdb
CFLAGS = -Wall -Wextra -pedantic -Werror -Wpedantic
CXXFLAGS = -Wall -std=c++11

# Flags adicionais para o linker
LDFLAGS = -lncursesw -lmenuw # Adiciona a biblioteca pdcurses ao processo de linkagem

# Nome do arquivo de saída (EXE para Windows)
TARGET = dzr.exe

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build

# Coleta todos os arquivos fonte no diretório SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.c)  # Busca todos os arquivos .cpp no diretório src
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)  # Substitui o diretório src por obj e a extensão .cpp por .o

# Regra para compilar tudo
default: $(BUILD_DIR)/$(TARGET)
	@echo "Iniciando o processo de build..."  # Log para indicar início do build

# Cria o arquivo EXE a partir dos objetos
$(BUILD_DIR)/$(TARGET): $(OBJS)
	@echo "Linkando objetos para criar o executável final..."  # Log para depuração
	@mkdir -p $(BUILD_DIR)  # Garante que o diretório de build existe
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)  # Comando para linkar os objetos com as flags de linkagem
	@echo "Build concluído: $(BUILD_DIR)/$(TARGET)"  # Log de conclusão do build

# Regra para compilar os arquivos .cpp em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compilando $< ..."  # Log para indicar qual arquivo está sendo compilado
	@mkdir -p $(OBJ_DIR)  # Garante que o diretório de objetos existe
	$(CC) $(CFLAGS) -c $< -o $@  # Comando para compilar o arquivo fonte
	@echo "Arquivo objeto gerado: $@"  # Log para confirmar geração do objeto

# Limpa os arquivos gerados
clean:
	@echo "Limpando arquivos gerados..."  # Log para indicar limpeza
	rm -rf $(OBJ_DIR) $(BUILD_DIR)/$(TARGET)
	@echo "Limpeza concluída."  # Log para confirmar conclusão da limpeza

run: $(BUILD_DIR)/$(TARGET)
	@echo "Executando o programa..."
	./$(BUILD_DIR)/$(TARGET)

debug:
	$(GDB) -w $(BUILD_DIR)/$(TARGET)

