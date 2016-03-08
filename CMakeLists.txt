cmake_minimum_required(VERSION 3.1)
project(7DRL_BOOTSTRAP)
set(CMAKE_CXX_STANDARD 14)

# libTCOD
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libtcod-1.5.1/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libtcod-1.5.1)
if(WIN32)
       link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libtcod-1.5.1/lib)
endif()

# entityx
option(ENTITYX_BUILD_TESTING OFF)
option(ENTITYX_RUN_BENCHMARKS OFF)
option(ENTITYX_BUILD_SHARED OFF)
add_subdirectory(entityx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/entityx)

# 7drl
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB SOURCES "src/*.h" "src/*.hpp" "src/*.cpp")
add_executable(7drl ${SOURCES})

if(WIN32)
       target_link_libraries(7drl LINK_PUBLIC libtcod-VS entityx)
       file(COPY libtcod-1.5.1/terminal.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR} NO_SOURCE_PERMISSIONS)
       file(COPY 7drl.toml DESTINATION ${CMAKE_CURRENT_BINARY_DIR} NO_SOURCE_PERMISSIONS)
       file(COPY libtcod-1.5.1/zlib1.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR} NO_SOURCE_PERMISSIONS)
       file(COPY libtcod-1.5.1/libtcod-VS.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR} NO_SOURCE_PERMISSIONS)
       file(COPY libtcod-1.5.1/SDL.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR} NO_SOURCE_PERMISSIONS)
else()
       target_link_libraries(7drl LINK_PUBLIC tcod_debug tcodxx_debug entityx)
endif()
