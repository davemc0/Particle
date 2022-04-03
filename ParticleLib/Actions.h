/// Actions.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// These structures store the details of actions for action lists

#ifndef _Actions_h
#define _Actions_h

#include "ParticleGroup.h"
#include "pSourceState.h"

#include <string>

namespace PAPI {

#define ACTION_DECLS                                    \
    static std::string name, abrv;                      \
    inline std::string GetName() const { return name; } \
    inline std::string GetAbrv() const { return abrv; } \
    void Execute(ParticleGroup& pg, ParticleList::iterator ibegin, ParticleList::iterator iend);

class PInternalState_t;

struct PActionBase {
    static std::string name, abrv;

    float dt; // This is copied to here from PInternalState_t.

    bool GetKillsParticles() { return bKillsParticles; }
    bool GetDoNotSegment() { return bDoNotSegment; }

    void SetKillsParticles(const bool v) { bKillsParticles = v; }
    void SetDoNotSegment(const bool v) { bDoNotSegment = v; }

    void SetPInternalState(PInternalState_t* P) { PS = P; }

    virtual void Execute(ParticleGroup& pg, ParticleList::iterator ibegin, ParticleList::iterator iend) = 0;

    virtual std::string GetName() const { return name; }
    virtual std::string GetAbrv() const { return abrv; }

private:
    // For doing optimizations where we perform all actions to a working set of particles,
    // then to the next working set, etc. to improve cache coherency.
    // This doesn't work if the application of an action to a particle is a function of other particles in the group.
    bool bDoNotSegment; // True if this action cannot be done in segments

    bool bKillsParticles; // True if this action cannot be part of a normal combined kernel

protected:
    PInternalState_t* PS;
};

///////////////////////////////////////////////////////////////////////////
// Data types derived from PActionBase.

struct PAAvoid : public PActionBase {
    pDomain* position; // Avoid region
    float look_ahead;  // How many time units ahead to look
    float magnitude;   // What percent of the way to go each time
    float epsilon;     // Add to r^2 for softening

    ACTION_DECLS;

    void Exec(const PDTriangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDRectangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDPlane& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDSphere& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDDisc& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
};

struct PABounce : public PActionBase {
    pDomain* position;      // Bounce region
    float oneMinusFriction; // Friction tangent to surface
    float resilience;       // Resilence perpendicular to surface
    float cutoffSqr;        // cutoff velocity; friction applies iff v > cutoff

    ACTION_DECLS;

    void Exec(const PDTriangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDRectangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDBox& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDPlane& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDSphere& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDDisc& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
};

struct PACallback : public PActionBase {
    P_PARTICLE_CALLBACK_ACTION callbackFunc;
    std::string callbackStr;
    pdata_t Data; // The action list number to call

    ACTION_DECLS;
};

struct PACallActionList : public PActionBase {
    int action_list_num; // The action list number to call

    ACTION_DECLS;
};

struct PACopyVertexB : public PActionBase {
    bool copy_pos; // True to copy pos to posB.
    bool copy_vel; // True to copy vel to velB.

    ACTION_DECLS;
};

struct PADamping : public PActionBase {
    pVec damping;  // Damping constant applied to velocity
    float vlowSqr; // Low and high cutoff velocities
    float vhighSqr;

    ACTION_DECLS;
};

struct PARotDamping : public PActionBase {
    pVec damping;  // Damping constant applied to velocity
    float vlowSqr; // Low and high cutoff velocities
    float vhighSqr;

    ACTION_DECLS;
};

struct PAExplosion : public PActionBase {
    pVec center;     // The center of the explosion
    float radius;    // Of shock wave peak
    float magnitude; // At unit radius
    float stdev;     // Sharpness or width of shock wave
    float epsilon;   // Softening parameter

    ACTION_DECLS;
};

struct PAFollow : public PActionBase {
    float magnitude;  // The grav of each particle
    float epsilon;    // Softening parameter
    float max_radius; // Only influence particles within max_radius

    ACTION_DECLS;
};

struct PAGravitate : public PActionBase {
    float magnitude;  // The grav of each particle
    float epsilon;    // Softening parameter
    float max_radius; // Only influence particles within max_radius

    ACTION_DECLS;
};

struct PAGravity : public PActionBase {
    pVec direction; // Amount to increment velocity

    ACTION_DECLS;
};

struct PAJet : public PActionBase {
    pDomain* dom; // Accelerate particles that are within this domain
    pDomain* acc; // Acceleration vector domain

    ACTION_DECLS;
};

struct PAKillOld : public PActionBase {
    float age_limit;     // Exact age at which to kill particles.
    bool kill_less_than; // True to kill particles less than limit.

    ACTION_DECLS;
};

struct PAMatchVelocity : public PActionBase {
    float magnitude;  // The grav of each particle
    float epsilon;    // Softening parameter
    float max_radius; // Only influence particles within max_radius

    ACTION_DECLS;
};

struct PAMatchRotVelocity : public PActionBase {
    float magnitude;  // The grav of each particle
    float epsilon;    // Softening parameter
    float max_radius; // Only influence particles within max_radius

    ACTION_DECLS;
};

struct PAMove : public PActionBase {
    bool move_velocity;
    bool move_rotational_velocity;

    ACTION_DECLS;
};

struct PAOrbitLine : public PActionBase {
    pVec p, axis;     // Endpoints of line to which particles are attracted
    float magnitude;  // Scales acceleration
    float epsilon;    // Softening parameter
    float max_radius; // Only influence particles within max_radius

    ACTION_DECLS;
};

struct PAOrbitPoint : public PActionBase {
    pVec center;      // Point to which particles are attracted
    float magnitude;  // Scales acceleration
    float epsilon;    // Softening parameter
    float max_radius; // Only influence particles within max_radius

    ACTION_DECLS;
};

struct PARandomAccel : public PActionBase {
    pDomain* gen_acc; // The domain of random accelerations.

    ACTION_DECLS;
};

struct PARandomDisplace : public PActionBase {
    pDomain* gen_disp; // The domain of random displacements.

    ACTION_DECLS;
};

struct PARandomVelocity : public PActionBase {
    pDomain* gen_vel; // The domain of random velocities.

    ACTION_DECLS;
};

struct PARandomRotVelocity : public PActionBase {
    pDomain* gen_vel; // The domain of random velocities.

    ACTION_DECLS;
};

struct PARestore : public PActionBase {
    float time_left; // Time remaining until they should be in position.
    bool restore_velocity;
    bool restore_rvelocity;

    ACTION_DECLS;
};

struct PASink : public PActionBase {
    bool kill_inside;  // True to dispose of particles *inside* domain
    pDomain* position; // Disposal region

    ACTION_DECLS;
};

struct PASinkVelocity : public PActionBase {
    bool kill_inside;  // True to dispose of particles with vel *inside* domain
    pDomain* velocity; // Disposal region

    ACTION_DECLS;
};

struct PASort : public PActionBase {
    pVec Eye;            // A point on the line to project onto
    pVec Look;           // The direction for which to sort particles
    bool front_to_back;  // True to sort front_to_back
    bool clamp_negative; // True to clamp negative dot products to zero

    ACTION_DECLS;
};

struct PASource : public PActionBase {
    pDomain* position;   // Choose a position in this domain
    float particle_rate; // Particles to generate per unit time
    pSourceState SrcSt;  // The state needed to create a new particle

    ACTION_DECLS;
};

struct PASpeedLimit : public PActionBase {
    float min_speed; // Clamp speed to this minimum.
    float max_speed; // Clamp speed to this maximum.

    ACTION_DECLS;
};

struct PATargetColor : public PActionBase {
    pVec color;  // Color to shift towards
    float alpha; // Alpha value to shift towards
    float scale; // Amount to shift by (1 == all the way)

    ACTION_DECLS;
};

struct PATargetSize : public PActionBase {
    pVec size;  // Size to shift towards
    pVec scale; // Amount to shift by per frame (1 == all the way)

    ACTION_DECLS;
};

struct PATargetVelocity : public PActionBase {
    pVec velocity; // Velocity to shift towards
    float scale;   // Amount to shift by (1 == all the way)

    ACTION_DECLS;
};

struct PATargetRotVelocity : public PActionBase {
    pVec velocity; // Velocity to shift towards
    float scale;   // Amount to shift by (1 == all the way)

    ACTION_DECLS;
};

struct PAVortex : public PActionBase {
    pVec tip;                // Tip of vortex
    pVec axis;               // Axis around which vortex is applied
    float tightnessExponent; // Raise radius to this power to create vortex-like silhouette
    float max_radius;        // Only influence particles within max_radius of axis
    float inSpeed;           // Inward acceleration of particles outside the vortex
    float upSpeed;           // Vertical acceleration of particles inside the vortex
    float aroundSpeed;       // Acceleration around vortex of particles inside the vortex

    ACTION_DECLS;
};
}; // namespace PAPI

#undef ACTION_DECLS

#endif
