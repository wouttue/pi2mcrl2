
cmake_minimum_required(VERSION 3.20)
project(PiCalc)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(pi2mcrl2
    src/main.cpp
    src/picalc/ast.cpp
    src/picalc/cli.cpp
    src/picalc/core.cpp
    src/picalc/lexer.cpp
    src/picalc/mcrl2converter.cpp
    src/picalc/parser.cpp
)
