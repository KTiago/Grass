cmake_minimum_required(VERSION 3.13)
project(Grass)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(GrassClient
        src/User.cpp)

add_executable(GrassServer
        src/server.cpp src/Parser.cpp src/Parser.h src/commands.cpp src/commands.h src/User.h src/client.cpp)


