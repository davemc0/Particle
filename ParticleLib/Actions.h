/// Actions.h
///
/// Copyright 1997-2007 by David K. McAllister
/// http://www.ParticleSystems.org
///
/// These structures store the details of actions for action lists

#ifndef _Actions_h
#define _Actions_h

#include "pSourceState.h"
#include "ParticleGroup.h"

#include <string>

namespace PAPI {

#define COMMON_ITEMS \
    static std::string name, abrv; \
    inline std::string GetName() const { return name; } \
    inline std::string GetAbrv() const { return abrv; } \
    void Execute(ParticleGroup &pg, ParticleList::iterator ibegin, ParticleList::iterator iend);

class PInternalState_t;

struct PActionBase
{
    static std::string name, abrv;

    float dt; // This is copied to here from PInternalState_t.

    bool GetKillsParticles() { return bKillsParticles; }
    bool GetDoNotSegment() { return bDoNotSegment; }

    void SetKillsParticles(const bool v) { bKillsParticles = v; }
    void SetDoNotSegment(const bool v) { bDoNotSegment = v; }

    void SetPInternalState(PInternalState_t *P) { PS = P; }

    virtual void Execute(ParticleGroup &pg, ParticleList::iterator ibegin, ParticleList::iterator iend) = 0;

    virtual std::string GetName() const { return name; }
    virtual std::string GetAbrv() const { return abrv; }

private:
    // For doing optimizations where we perform all actions to a working set of particles,
    // then to the next working set, etc. to improve cache coherency.
    // This doesn't work if the application of an action to a particle is a function of other particles in the group.
    bool bDoNotSegment;   // True if this action cannot be done in segments

    bool bKillsParticles; // True if this action cannot be part of a normal combined kernel

protected:
    PInternalState_t *PS;
};

///////////////////////////////////////////////////////////////////////////
// Data types derived from PActionBase.

struct PAAvoid : public PActionBase
{
    pDomain *position;	// Avoid region
    float look_ahead;	// how many time units ahead to look
    float magnitude;	// what percent of the way to go each time
    float epsilon;		// add to r^2 for softening

    COMMON_ITEMS

    void Exec(const PDTriangle &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDRectangle &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDPlane &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDSphere &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDDisc &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
};

struct PABounce : public PActionBase
{
    pDomain *position;	// Bounce region
    float oneMinusFriction;	// Friction tangent to surface
    float resilience;	// Resilence perpendicular to surface
    float cutoffSqr;	// cutoff velocity; friction applies iff v > cutoff

    COMMON_ITEMS

    void Exec(const PDTriangle &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDRectangle &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDPlane &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDSphere &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDDisc &dom, ParticleGroup &group, ParticleList::iterator ibegin, ParticleList::iterator iend);
};

struct PACallback : public PActionBase
{
    P_PARTICLE_CALLBACK_ACTION callbackFunc;
    std::string callbackStr;
    pdata_t Data; // The action list number to call

    COMMON_ITEMS
};

struct PACallActionList : public PActionBase
{
    int action_list_num; // The action list number to call

    COMMON_ITEMS
};

struct PACopyVertexB : public PActionBase
{
    bool copy_pos;		// True to copy pos to posB.
    bool copy_vel;		// True to copy vel to velB.

    COMMON_ITEMS
};

struct PADamping : public PActionBase
{
    pVec damping;	    // Damping constant applied to velocity
    float vlowSqr;		// Low and high cutoff velocities
    float vhighSqr;

    COMMON_ITEMS
};

struct PARotDamping : public PActionBase
{
    pVec damping;	    // Damping constant applied to velocity
    float vlowSqr;		// Low and high cutoff velocities
    float vhighSqr;

    COMMON_ITEMS
};

struct PAExplosion : public PActionBase
{
    pVec center;		// The center of the explosion
    float radius;		// Of shock wave peak
    float magnitude;	// At unit radius
    float stdev;		// Sharpness or width of shock wave
    float epsilon;		// Softening parameter

    COMMON_ITEMS
};

struct PAFollow : public PActionBase
{
    float magnitude;	// The grav of each particle
    float epsilon;		// Softening parameter
    float max_radius;	// Only influence particles within max_radius

    COMMON_ITEMS
};

struct PAGravitate : public PActionBase
{
    float magnitude;	// The grav of each particle
    float epsilon;		// Softening parameter
    float max_radius;	// Only influence particles within max_radius

    COMMON_ITEMS
};

struct PAGravity : public PActionBase
{
    pVec direction;	    // Amount to increment velocity

    COMMON_ITEMS
};

struct PAJet : public PActionBase
{
    pDomain *dom;		// Accelerate particles that are within this domain
    pDomain *acc;		// Acceleration vector domain

    COMMON_ITEMS
};

struct PAKillOld : public PActionBase
{
    float age_limit;		// Exact age at which to kill particles.
    bool kill_less_than;	// True to kill particles less than limit.

    COMMON_ITEMS
};

struct PAMatchVelocity : public PActionBase
{
    float magnitude;	// The grav of each particle
    float epsilon;		// Softening parameter
    float max_radius;	// Only influence particles within max_radius

    COMMON_ITEMS
};

struct PAMatchRotVelocity : public PActionBase
{
    float magnitude;	// The grav of each particle
    float epsilon;		// Softening parameter
    float max_radius;	// Only influence particles within max_radius

    COMMON_ITEMS
};

struct PAMove : public PActionBase
{
    bool move_velocity;
    bool move_rotational_velocity;

    COMMON_ITEMS
};

struct PAOrbitLine : public PActionBase
{
    pVec p, axis;	    // Endpoints of line to which particles are attracted
    float magnitude;	// Scales acceleration
    float epsilon;		// Softening parameter
    float max_radius;	// Only influence particles within max_radius

    COMMON_ITEMS
};

struct PAOrbitPoint : public PActionBase
{
    pVec center;		// Point to which particles are attracted
    float magnitude;	// Scales acceleration
    float epsilon;		// Softening parameter
    float max_radius;	// Only influence particles within max_radius

    COMMON_ITEMS
};

struct PARandomAccel : public PActionBase
{
    pDomain *gen_acc;	// The domain of random accelerations.

    COMMON_ITEMS
};

struct PARandomDisplace : public PActionBase
{
    pDomain *gen_disp;	// The domain of random displacements.

    COMMON_ITEMS
};

struct PARandomVelocity : public PActionBase
{
    pDomain *gen_vel;	// The domain of random velocities.

    COMMON_ITEMS
};

struct PARandomRotVelocity : public PActionBase
{
    pDomain *gen_vel;	// The domain of random velocities.

    COMMON_ITEMS
};

struct PARestore : public PActionBase
{
    float time_left;	// Time remaining until they should be in position.
    bool restore_velocity;
    bool restore_rvelocity;

    COMMON_ITEMS
};

struct PASink : public PActionBase
{
    bool kill_inside;	// True to dispose of particles *inside* domain
    pDomain *position;	// Disposal region

    COMMON_ITEMS
};

struct PASinkVelocity : public PActionBase
{
    bool kill_inside;	// True to dispose of particles with vel *inside* domain
    pDomain *velocity;	// Disposal region

    COMMON_ITEMS
};

struct PASort : public PActionBase
{
    pVec Eye;		// A point on the line to project onto
    pVec Look;		// The direction for which to sort particles
    bool front_to_back; // True to sort front_to_back
    bool clamp_negative; // True to clamp negative dot products to zero

    COMMON_ITEMS
};

struct PASource : public PActionBase
{
    pDomain *position;     // Choose a position in this domain
    float particle_rate;  // Particles to generate per unit time
    pSourceState SrcSt;   // The state needed to create a new particle

    COMMON_ITEMS
};

struct PASpeedLimit : public PActionBase
{
    float min_speed;		// Clamp speed to this minimum.
    float max_speed;		// Clamp speed to this maximum.

    COMMON_ITEMS
};

struct PATargetColor : public PActionBase
{
    pVec color;		    // Color to shift towards
    float alpha;		// Alpha value to shift towards
    float scale;		// Amount to shift by (1 == all the way)

    COMMON_ITEMS
};

struct PATargetSize : public PActionBase
{
    pVec size;		// Size to shift towards
    pVec scale;		// Amount to shift by per frame (1 == all the way)

    COMMON_ITEMS
};

struct PATargetVelocity : public PActionBase
{
    pVec velocity;	    // Velocity to shift towards
    float scale;		// Amount to shift by (1 == all the way)

    COMMON_ITEMS
};

struct PATargetRotVelocity : public PActionBase
{
    pVec velocity;	    // Velocity to shift towards
    float scale;		// Amount to shift by (1 == all the way)

    COMMON_ITEMS
};

struct PAVortex : public PActionBase
{
    pVec tip;		         // Tip of vortex
    pVec axis;		         // Axis around which vortex is applied
    float tightnessExponent; // Raise radius to this power to create vortex-like silhouette
    float max_radius;	     // Only influence particles within max_radius of axis
    float inSpeed;	         // inward acceleration of particles outside the vortex
    float upSpeed;           // vertical acceleration of particles inside the vortex
    float aroundSpeed;       // acceleration around vortex of particles inside the vortex

    COMMON_ITEMS
};

};

#endif
