# Particle (the actual library)

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(Particle)

set(LIB_SOURCES
    ActionExecute.cpp
    ActionStructs.h
    ActionsAPI.cpp
    LibHelpers.h
    OtherAPI.cpp
    PInternalState.h
    PInternalState.cpp
    ParticleGroup.h
)

set(API_SOURCES
    ../Particle/pAPI.h
    ../Particle/pAPIContext.h
    ../Particle/pActionDecls.h
    ../Particle/pActionImpls.h
    ../Particle/pDeclarations.h
    ../Particle/pDomain.h
    ../Particle/pError.h
    ../Particle/pInlineActionsAPI.h
    ../Particle/pInternalShadow.h
    ../Particle/pParticle.h
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

# Push good flags out to library users: build in parallel, warning level, all warnings as errors, optimization
target_compile_options(DMcTools PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:/MP>
    $<$<COMPILE_LANGUAGE:CXX>:/W3>
    $<$<COMPILE_LANGUAGE:CXX>:/WX>
    $<$<COMPILE_LANGUAGE:CXX>:/GF>
    $<$<COMPILE_LANGUAGE:CXX>:/GL>
    $<$<COMPILE_LANGUAGE:CXX>:/Gy>
    $<$<COMPILE_LANGUAGE:CXX>:/Oi>
    $<$<COMPILE_LANGUAGE:CXX>:/Ot>
    $<$<COMPILE_LANGUAGE:CXX>:/Oy>
    $<$<COMPILE_LANGUAGE:CXX>:/Qpar>
    $<$<COMPILE_LANGUAGE:CXX>:/arch:AVX512>
    $<$<COMPILE_LANGUAGE:CXX>:/fp:fast>
)
target_link_options(DMcTools INTERFACE $<$<LINK_LANGUAGE:CXX>:/LTCG>)

target_include_directories(Particle
    PRIVATE "."
    PRIVATE ".."
)

# Tell users of the library what they need to include to use it
target_include_directories(Particle
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
