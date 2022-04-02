# Particle System API Main Solution

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(EXE_NAME ParticleSolution)

project(${EXE_NAME})

set(CMAKE_CXX_STANDARD 17)

set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# build in parallel
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/MP>)

add_subdirectory(${PROJECT_ROOT_DIR}/../DMcTools ${CMAKE_CURRENT_BINARY_DIR}/DMcTools)

add_subdirectory(${PROJECT_ROOT_DIR}/ParticleLib ${CMAKE_CURRENT_BINARY_DIR}/ParticleLib)

add_subdirectory(${PROJECT_ROOT_DIR}/Example ${CMAKE_CURRENT_BINARY_DIR}/Example)
add_subdirectory(${PROJECT_ROOT_DIR}/PSpray ${CMAKE_CURRENT_BINARY_DIR}/PSpray)
add_subdirectory(${PROJECT_ROOT_DIR}/ParBench ${CMAKE_CURRENT_BINARY_DIR}/ParBench)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PSpray)
