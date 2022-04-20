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
    ../Particle/pInternalsIface.h
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

# Push good flags out to library users
# Warning level, all warnings as errors, optimization
target_compile_options(Particle PUBLIC /W3 /WX /fp:fast /Ot /Oi /Oy /GL /Gy /GF /Qpar /arch:AVX512)
target_link_options(Particle INTERFACE /LTCG)

target_include_directories(Particle
    PRIVATE "."
    PRIVATE ".."
)

# Tell users of the library what they need to include to use it
target_include_directories(Particle
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
