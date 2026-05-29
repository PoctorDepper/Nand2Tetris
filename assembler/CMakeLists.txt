cmake_minimum_required(VERSION 3.28)
project(assembler C)

set(CMAKE_C_STANDARD 11)

include_directories(include/)

add_executable(assembler
        src/main.c
        src/symbol_table.c
        src/file_lines.c
)
