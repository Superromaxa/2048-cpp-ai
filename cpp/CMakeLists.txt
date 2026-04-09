cmake_minimum_required(VERSION 4.1)
project(Game2048)

set(CMAKE_CXX_STANDARD 20)

add_executable(Game2048 main.cpp
Board.cpp
Game.cpp
Agent.cpp
Collector.cpp
DataGenerator.cpp
Evaluators.cpp)
