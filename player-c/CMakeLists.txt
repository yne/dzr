cmake_minimum_required(VERSION 3.30)
project(PlayerC C)

# Define o padrão C23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define as flags de compilação para mostrar avisos e gerar símbolos de depuração
add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-unused-function)

# Define o diretório de saída para os binários
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Adiciona o diretório src/ para buscar os arquivos de código e cabeçalhos
include_directories(src)

# Busca as bibliotecas ncursesw e curl
find_package(CURL REQUIRED)

if(MSYS)
    find_library(NCURSES_LIBRARY NAMES ncursesw PATHS /usr/lib /usr/local/lib)
    find_library(MENU_LIBRARY NAMES menuw)
    find_library(PANEL_LIBRARY NAMES panelw)
else()
    find_package(Curses REQUIRED)
    
    find_library(NCURSES_LIBRARY NAMES ncursesw PATHS /usr/lib /usr/local/lib)
    find_library(MENU_LIBRARY NAMES menuw)
    find_library(PANEL_LIBRARY NAMES panelw)

    include_directories(${CURSES_INCLUDE_DIR})
endif()
# Busca explicitamente a biblioteca ncursesw e menu no Linux

# Busca manualmente os arquivos de cabeçalho e a biblioteca do cJSON
find_path(CJSON_INCLUDE_DIR cjson/cJSON.h PATHS /usr/include /usr/local/include)
find_library(CJSON_LIBRARY NAMES cjson PATHS /usr/lib /usr/local/lib)

# Adiciona os diretórios de inclusão do cJSON
include_directories(${CJSON_INCLUDE_DIR})


# Lista os arquivos de código-fonte dentro de src/
file(GLOB SOURCES src/*.c)

message(STATUS "Arquivos encontrados: ${SOURCES}")

# Cria o executável com os arquivos fonte
add_executable(dzr ${SOURCES})


# Liga as bibliotecas ncursesw, curl, cJSON e menu ao executável
target_compile_options(dzr PUBLIC -g3)

# Adiciona as bibliotecas ncursesw, curl, cJSON e menu ao executável
set(LIBRARIES ${NCURSES_LIBRARY} ${CURL_LIBRARIES} ${CJSON_LIBRARY} ${MENU_LIBRARY} ${PANEL_LIBRARY})
MESSAGE(STATUS "Bibliotecas encontradas: ${LIBRARIES}")
target_link_libraries(dzr ${LIBRARIES})
add_custom_target(run ALL ./build/dzr 2>"./out.log" )
