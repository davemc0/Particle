# Particle System API Main Solution

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(EXE_NAME ParticleSolution)

project(${EXE_NAME})

set(CMAKE_CXX_STANDARD 17)
# set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# build in parallel
add_compile_options(/MP)

add_subdirectory(${PROJECT_ROOT_DIR}/ParticleLib ${CMAKE_CURRENT_BINARY_DIR}/ParticleLib)

add_subdirectory(${PROJECT_ROOT_DIR}/Example ${CMAKE_CURRENT_BINARY_DIR}/Example)
add_subdirectory(${PROJECT_ROOT_DIR}/PSpray ${CMAKE_CURRENT_BINARY_DIR}/PSpray)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PSpray)
