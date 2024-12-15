cmake_minimum_required(VERSION 3.10)
project(
    DZR_PLAYER_C
)

add_executable(dzr dzr.c)

target_link_libraries(dzr curl)