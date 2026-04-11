cmake_minimum_required(VERSION 4.1)
project(Game2048)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(game2048_core
        Board.cpp
        Game.cpp
        Agent.cpp
        Collector.cpp
        DataGenerator.cpp
        Evaluators.cpp
)

target_include_directories(game2048_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(compare main_compare.cpp)
target_link_libraries(compare PRIVATE game2048_core)

add_executable(collect_data main_collect_data.cpp)
target_link_libraries(collect_data PRIVATE game2048_core)