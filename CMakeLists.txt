
# SPDX-License-Identidier: MIT

cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

project(pax-playground)
set(target pax-playground)

find_package(SDL2 REQUIRED)
add_subdirectory(pax-graphics)

add_executable(${target}
    src/main.c
    src/shapes.c
)

include_directories(${target} ${SDL2_INCLUDE_DIRS})
target_link_libraries(${target} ${SDL2_LIBRARIES})
target_link_libraries(${target} pax_graphics)
