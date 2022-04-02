# Boids executable using Particle System API

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(EXE_NAME Boids)

project(${EXE_NAME})

set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# GLEW - setenv GLEW_HOME C:/Users/davemc/source/repos/Goodies/glew-2.2.0
set(CMAKE_PREFIX_PATH $ENV{GLEW_HOME})
set(CMAKE_LIBRARY_PATH $ENV{GLEW_HOME}/lib/Release/x64)
set(GLEW_USE_STATIC_LIBS TRUE)

find_package(GLEW REQUIRED)

add_definitions(-DGLEW_STATIC)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_STATIC_LIBRARIES})

# FreeGLUT - setenv GLUT_HOME C:/Users/davemc/source/repos/Goodies/freeglut-3.2.2
set(GLUT_INCLUDE_DIR $ENV{GLUT_HOME}/include)
set(GLUT_LIBRARIES "optimized;$ENV{GLUT_HOME}/lib/Release/freeglut_static.lib;debug;$ENV{GLUT_HOME}/lib/Debug/freeglut_staticd.lib")

add_definitions(-DFREEGLUT_STATIC)
include_directories(${GLUT_INCLUDE_DIR})
link_libraries(${GLUT_LIBRARIES})

set(SOURCES
    RenderModel.cpp
    ../PSpray/DrawGroups.cpp
    ../PSpray/DrawGroups.h
    Boids.cpp
)

source_group("src"  FILES ${SOURCES})

add_executable(${EXE_NAME} ${SOURCES})

set_target_properties(${EXE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_ROOT_DIR} )
set_target_properties(${EXE_NAME} PROPERTIES CMAKE_CXX_STANDARD 17 )
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})

target_link_libraries(${EXE_NAME} PRIVATE DMcTools)
target_link_libraries(${EXE_NAME} PRIVATE Particle)
