# Particle (the actual library)

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(Particle)

set(LIB_SOURCES
    Actions.cpp    
    Actions.h    
    ActionsAPI.cpp    
    CUDAExecute.cpp
    CUDAExecute.h
    CUDAGen.cpp
    CUDAGen.h
    OtherAPI.cpp    
    PInternalSourceState.h    
    PInternalState.cpp
    PInternalState.h    
    Particle.h    
    ParticleGroup.h
)

set(API_SOURCES
    ../Particle/CUDAActions.cuh
    ../Particle/CUDADomains.cuh
    ../Particle/pAPI.h
    ../Particle/pDomain.h
    ../Particle/pError.h
    ../Particle/pSourceState.h
    ../Particle/pVec.h
)

set(SOURCES
    ${LIB_SOURCES}
    ${API_SOURCES}
)

source_group("Lib" FILES ${LIB_SOURCES})
source_group("API" FILES ${API_SOURCES})

add_library(Particle STATIC ${SOURCES})

# Push good flags out to library users
# Warning level, all warnings as errors, optimization
target_compile_options(Particle PUBLIC /W3 /WX /fp:fast /Ot /Oi /Oy /GL /Gy /GF /Qpar /arch:AVX512)
target_link_options(Particle INTERFACE /LTCG)

target_include_directories(Particle
    PRIVATE "."
    PRIVATE "../Particle"
)

# Tell users of the library what they need to include to use it
target_include_directories(Particle
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
