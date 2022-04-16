/// Particle.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// This file contains the definition of a particle.
/// It should only be included by API implementation files, not by applications.
/// The only exception is callback functions.

#ifndef Particle_h
#define Particle_h

#include "Particle/pVec.h"

namespace PAPI {

// A single particle
struct Particle_t {
    pVec pos;
    pVec posB;
    pVec up;
    pVec upB;
    pVec vel;
    pVec velB; // Used to compute binormal, normal, etc.
    pVec rvel;
    pVec size;
    pVec color;  // Color must be next to alpha so glColor4fv works.
    float alpha; // This is both cunning and scary.
    float age;
    float mass;
    float tmp0;   // These temporaries are used as padding and for sorting.
    pdata_t data; // Arbitrary data for user callbacks

    inline Particle_t() {}

    inline Particle_t(const pVec& pos, const pVec& posB, const pVec& up, const pVec& upB, const pVec& vel, const pVec& velB, const pVec& rvel,
                      const pVec& rvelB, const pVec& size, const pVec& color, float alpha, float age, float mass, pdata_t data, float tmp0) :
        pos(pos),
        posB(posB), up(up), upB(upB), vel(vel), velB(velB), rvel(rvel), size(size), color(color), alpha(alpha), age(age), mass(mass), data(data), tmp0(0)
    {
    }

    inline Particle_t(const Particle_t& rhs) :
        pos(rhs.pos), posB(rhs.posB), up(rhs.up), upB(rhs.upB), vel(rhs.vel), velB(rhs.velB), rvel(rhs.rvel), size(rhs.size), color(rhs.color),
        alpha(rhs.alpha), age(rhs.age), mass(rhs.mass), data(rhs.data), tmp0(rhs.tmp0)
    {
    }

    // For sorting.
    bool operator<(const Particle_t& P) const { return tmp0 < P.tmp0; }
};

static_assert(sizeof(Particle_t) == 32 * 4, "Unexpected change in Particle_t size!");
}; // namespace PAPI

#endif
