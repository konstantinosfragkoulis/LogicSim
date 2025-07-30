cmake_minimum_required(VERSION 3.31)
project(LogicSim)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

add_executable(LogicSim main.cpp
        Simulator.hpp
        Simulator.cpp
        DragAndDrop.cpp
        DragAndDrop.hpp
        ShortcutManager.cpp
        ShortcutManager.hpp
)

target_link_libraries(LogicSim PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)