/// PDeclarations.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// Include this file in all applications that use the Particle System API.

#ifndef particle_declarations_h
#define particle_declarations_h

#include "Particle/pVec.h"

namespace PAPI {
/// This is the version number of this release of the API.
const int P_VERSION = 300;

typedef unsigned int pdata_t;

/// A very large float value used as a default arg passed into functions
const float P_MAXFLOAT = 1.0e16f; // Actually this must be < sqrt(MAXFLOAT) since we store this value squared.

/// A very small float value added to some physical calculations to dampen them and improve stability
const float P_EPS = 1e-3f;

/// This is the type of the particle birth and death callback functions that you can register.
typedef void (*P_PARTICLE_CALLBACK)(struct Particle_t& particle, const pdata_t data);

/// This is the type of the callback functions that you can register for the Callback() action.
typedef void (*P_PARTICLE_CALLBACK_ACTION)(struct Particle_t& particle, const pdata_t data, const float dt);
}; // namespace PAPI

#endif
