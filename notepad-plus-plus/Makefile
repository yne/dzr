# Definições do compilador e flags
CC = gcc
CXX = g++
CFLAGS = -Wall -shared -fPIC -std=c++17 -Iinclude
CXXFLAGS = -Wall -shared -fPIC -std=c++17 -Iinclude

# Nome do arquivo de saída (DLL para Windows)
TARGET = dzr.dll

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build

# Coleta todos os arquivos fonte no diretório SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Regra para compilar tudo
default: $(BUILD_DIR)/$(TARGET)

# Cria o arquivo DLL a partir dos objetos
$(BUILD_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regra para compilar os arquivos .c em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpa os arquivos gerados
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
