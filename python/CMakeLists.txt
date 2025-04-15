cmake_minimum_required(VERSION 3.15...3.29)
project(example LANGUAGES CXX)

set(CMAKE_C_COMPILER /usr/local/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/local/bin/g++)
set(CMAKE_CXX_STANDARD 17)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(maze maze.cpp)
install(TARGETS maze DESTINATION .)
