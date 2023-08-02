cmake_minimum_required(VERSION 2.8)
project(render)

add_executable(
    render
    src/main.cpp
)

find_package(SFML 2.5 COMPONENTS audio graphics window system REQUIRED)

target_link_libraries(render sfml-audio sfml-graphics sfml-window sfml-system)
