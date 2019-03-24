cmake_minimum_required(VERSION 3.13)
project(Grass)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(Grass
        src/client.cpp
        src/server.cpp src/parser.cpp src/parser.h)
