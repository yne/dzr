# Variáveis
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Nome do executável
TARGET = $(BIN_DIR)/player-c

# Lista de arquivos fonte
SRCS = $(wildcard $(SRC_DIR)/**/*.cpp $(SRC_DIR)/*.cpp)

# Lista de arquivos objeto gerados
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Regra principal
all: $(TARGET)

# Regra para compilar o executável final
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regra para compilar arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -rf $(BUILD_DIR)

# Phony targets (para evitar conflitos com arquivos de mesmo nome)
.PHONY: all clean
