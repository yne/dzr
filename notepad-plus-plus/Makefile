# Definições do compilador e flags
CC = gcc
CXX = g++
CFLAGS = -Wall -shared
CXXFLAGS = -std=c++14 -O3 -static-libgcc -static-libstdc++ -Wall -Wno-unknown-pragmas -shared
RC = windres

# Nome do arquivo de saída (DLL para Windows)
TARGET = dzr.dll

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build
RES_DIR = src

# Coleta todos os arquivos fonte no diretório SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

RES = NppPluginDemo.res
RCFILE = $(RES_DIR)/NppPluginDemo.rc

# Regra para compilar tudo
default: $(BUILD_DIR)/$(TARGET)

# Cria o arquivo DLL a partir dos objetos
$(BUILD_DIR)/$(TARGET): $(OBJS) $(BUILD_DIR)/$(RES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) $(BUILD_DIR)/$(RES) -o $@ $^

# Regra para compilar os arquivos .c em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra para compilar o arquivo .rc em .res
$(BUILD_DIR)/$(RES): $(RCFILE)
	$(RC) $< -O coff -o $@

# Limpa os arquivos gerados
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
