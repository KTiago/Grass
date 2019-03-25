cmake_minimum_required(VERSION 3.13)
project(Grass)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(GrassClient
        src/user.cpp)

add_executable(GrassServer
        src/server.cpp src/parser.cpp src/parser.h src/commands.cpp src/commands.h src/user.h src/client.cpp)


