cmake_minimum_required(VERSION 3.1)
project(7DRL_BOOTSTRAP)
set(CMAKE_CXX_STANDARD 14)

# libTCOD
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libtcod-1.5.1/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libtcod-1.5.1)

# 7drl
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(7drl ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(7drl LINK_PUBLIC tcod_debug tcodxx_debug)
