# WSL-Plus Desktop (N0-GUI) —— Qt Widgets 骨架
# 独立 Qt 工程(不依赖 common 大库): v0.1 通过 wsl.exe 命令面对话后端(零耦合),
# 后续可平滑换为直接链接模块(DLL 化)。
cmake_minimum_required(VERSION 3.21)
project(wslplus-desktop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(wslplus-desktop WIN32
    main.cpp
)

target_link_libraries(wslplus-desktop PRIVATE Qt6::Widgets)
