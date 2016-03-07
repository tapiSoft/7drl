cmake_minimum_required(VERSION 3.1)
project(7DRL_BOOTSTRAP)
set(CMAKE_CXX_STANDARD 14)

# libTCOD
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libtcod-1.5.1/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libtcod-1.5.1)

# entityx
option(ENTITYX_BUILD_TESTING OFF)
option(ENTITYX_RUN_BENCHMARKS OFF)
option(ENTITYX_BUILD_SHARED OFF)
add_subdirectory(entityx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/entityx)

# 7drl
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(7drl
	${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/gamestate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	)

target_link_libraries(7drl LINK_PUBLIC tcod_debug tcodxx_debug entityx)
