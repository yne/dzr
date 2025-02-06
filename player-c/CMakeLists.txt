cmake_minimum_required(VERSION 3.30)
project(PlayerC C)

# Define o padrão C23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define as flags de compilação para mostrar avisos e gerar símbolos de depuração
add_compile_options(-Wall -Wextra -pedantic -Werror -Wpedantic)

# Define o diretório de saída para os binários
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Adiciona o diretório src/ para buscar os arquivos de código e cabeçalhos
include_directories(src)

# Busca as bibliotecas ncursesw e curl
find_package(CURL REQUIRED)

# Busca explicitamente a biblioteca ncursesw e menu no Linux
find_library(NCURSES_LIBRARY NAMES ncursesw PATHS /usr/lib /usr/local/lib)
find_library(MENU_LIBRARY NAMES menuw)

# Busca manualmente os arquivos de cabeçalho e a biblioteca do cJSON
find_path(CJSON_INCLUDE_DIR cjson/cJSON.h PATHS /usr/include /usr/local/include)
find_library(CJSON_LIBRARY NAMES cjson PATHS /usr/lib /usr/local/lib)

# Se não encontrar, mostra erro
if (NOT CJSON_INCLUDE_DIR OR NOT CJSON_LIBRARY)
    message(FATAL_ERROR "cJSON não encontrado! Instale a biblioteca (ex: sudo apt install libcjson-dev).")
endif()

# Adiciona os diretórios de inclusão do cJSON
include_directories(${CJSON_INCLUDE_DIR})


# Se não encontrar a ncursesw, mostra erro
if (NOT NCURSES_LIBRARY)
    message(FATAL_ERROR "ncursesw não encontrado! Instale a biblioteca (ex: sudo apt install libncursesw5-dev).")
endif()

# Lista os arquivos de código-fonte dentro de src/
set(SOURCES
    src/dzr.c
)

# Cria o executável com os arquivos fonte
add_executable(dzr ${SOURCES})

# Liga as bibliotecas ncursesw, curl, cJSON e menu ao executável
target_compile_options(dzr PUBLIC -O1 -g3)
MESSAGE(STATUS "Bibliotecas encontradas: ${NCURSES_LIBRARY} ${CURL_LIBRARIES} ${CJSON_LIBRARY} ${MENU_LIBRARY}")
target_link_libraries(dzr ${NCURSES_LIBRARY} ${CURL_LIBRARIES} ${CJSON_LIBRARY} ${MENU_LIBRARY})
add_custom_target(run exec ./build/dzr 2>"./out.log" ALL)
