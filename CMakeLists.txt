cmake_minimum_required(VERSION 3.10)
project(TetrisGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS Graphics Window System Audio REQUIRED)

add_executable(tetris main.cpp)
target_link_libraries(tetris PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)
