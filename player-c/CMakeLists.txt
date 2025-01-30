cmake_minimum_required(VERSION 3.31.5)
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

# Busca as bibliotecas ncurses e curl
find_package(Curses REQUIRED)
find_package(CURL REQUIRED)

# Verifica se está rodando no Windows (MSYS2/MinGW)
if (MINGW)
    # Define prefixos para localizar bibliotecas no MSYS2 MinGW
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".dll.a" ".lib")

    # Busca manualmente as bibliotecas ncursesw e menu para MinGW
    find_library(NCURSES_LIBRARY NAMES ncursesw PATHS /mingw64/lib /mingw32/lib)
    find_library(MENU_LIBRARY NAMES menuw PATHS /mingw64/lib /mingw32/lib)
else()
    # Busca explicitamente a biblioteca menu no Linux
    find_library(MENU_LIBRARY NAMES menu)
endif()

# Busca manualmente os arquivos de cabeçalho e a biblioteca do cJSON
find_path(CJSON_INCLUDE_DIR cjson/cJSON.h PATHS /usr/include /usr/local/include /mingw64/include /mingw32/include)
find_library(CJSON_LIBRARY NAMES cjson PATHS /usr/lib /usr/local/lib /mingw64/lib /mingw32/lib)

# Se não encontrar, mostra erro
if (NOT CJSON_INCLUDE_DIR OR NOT CJSON_LIBRARY)
    message(FATAL_ERROR "cJSON não encontrado! Instale a biblioteca (ex: sudo apt install libcjson-dev ou pacman -S mingw-w64-ucrt-x86_64-cjson).")
endif()

# Adiciona os diretórios de inclusão do cJSON
include_directories(${CJSON_INCLUDE_DIR})

# Lista os arquivos de código-fonte dentro de src/
set(SOURCES
    src/dzr.c
)

# Cria o executável com os arquivos fonte
add_executable(dzr ${SOURCES})

# Liga as bibliotecas ncurses, curl e cJSON ao executável
target_compile_options(dzr PUBLIC -O1)
target_link_libraries(dzr ${CURSES_LIBRARIES} ${CURL_LIBRARIES} ${CJSON_LIBRARY} ${MENU_LIBRARY})
add_custom_target(run exec ./build/dzr 2>"./out.log" ALL)
