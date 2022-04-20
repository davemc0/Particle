/// ActionStructs.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// These structures store the details of actions for action lists

#ifndef _ActionStructs_h
#define _ActionStructs_h

#include "Particle/pSourceState.h"
#include "ParticleGroup.h"

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
    std::shared_ptr<pDomain> position;
    float look_ahead;
    float magnitude;
    float epsilon;

    ACTION_DECLS;

    void Exec(const PDTriangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDRectangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDPlane& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDSphere& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
    void Exec(const PDDisc& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend);
};

struct PABounce : public PActionBase {
    std::shared_ptr<pDomain> position;
    float friction;
    float resilience;
    float fric_min_vel;

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
    pdata_t call_data;

    ACTION_DECLS;
};

struct PACallActionList : public PActionBase {
    int action_list_num;

    ACTION_DECLS;
};

struct PACommitKills : public PActionBase {
    ACTION_DECLS;
};

struct PACopyVertexB : public PActionBase {
    bool copy_pos;
    bool copy_vel;

    ACTION_DECLS;
};

struct PADamping : public PActionBase {
    pVec damping;
    float min_vel;
    float max_vel;

    ACTION_DECLS;
};

struct PARotDamping : public PActionBase {
    pVec damping;
    float min_vel;
    float max_vel;

    ACTION_DECLS;
};

struct PAExplosion : public PActionBase {
    pVec center;
    float radius;
    float magnitude;
    float stdev;
    float epsilon;

    ACTION_DECLS;
};

struct PAFollow : public PActionBase {
    float magnitude;
    float epsilon;
    float max_radius;

    ACTION_DECLS;
};

struct PAGravitate : public PActionBase {
    float magnitude;
    float epsilon;
    float max_radius;

    ACTION_DECLS;
};

struct PAGravity : public PActionBase {
    pVec direction;

    ACTION_DECLS;
};

struct PAJet : public PActionBase {
    std::shared_ptr<pDomain> dom;
    std::shared_ptr<pDomain> acc;

    ACTION_DECLS;
};

struct PAKillOld : public PActionBase {
    float age_limit;
    bool kill_less_than;

    ACTION_DECLS;
};

struct PAMatchVelocity : public PActionBase {
    float magnitude;
    float epsilon;
    float max_radius;

    ACTION_DECLS;
};

struct PAMatchRotVelocity : public PActionBase {
    float magnitude;
    float epsilon;
    float max_radius;

    ACTION_DECLS;
};

struct PAMove : public PActionBase {
    bool move_velocity;
    bool move_rotational_velocity;

    ACTION_DECLS;
};

struct PAOrbitLine : public PActionBase {
    pVec p, axis;
    float magnitude;
    float epsilon;
    float max_radius;

    ACTION_DECLS;
};

struct PAOrbitPoint : public PActionBase {
    pVec center;
    float magnitude;
    float epsilon;
    float max_radius;

    ACTION_DECLS;
};

struct PARandomAccel : public PActionBase {
    std::shared_ptr<pDomain> gen_acc;

    ACTION_DECLS;
};

struct PARandomDisplace : public PActionBase {
    std::shared_ptr<pDomain> gen_disp;

    ACTION_DECLS;
};

struct PARandomVelocity : public PActionBase {
    std::shared_ptr<pDomain> gen_vel;

    ACTION_DECLS;
};

struct PARandomRotVelocity : public PActionBase {
    std::shared_ptr<pDomain> gen_vel;

    ACTION_DECLS;
};

struct PARestore : public PActionBase {
    float time_left;
    bool restore_velocity;
    bool restore_rvelocity;

    ACTION_DECLS;
};

struct PASink : public PActionBase {
    bool kill_inside;
    std::shared_ptr<pDomain> kill_pos_dom;

    ACTION_DECLS;
};

struct PASinkVelocity : public PActionBase {
    bool kill_inside;
    std::shared_ptr<pDomain> kill_vel_dom;

    ACTION_DECLS;
};

struct PASort : public PActionBase {
    pVec Eye;
    pVec Look;
    bool front_to_back;
    bool clamp_negative;

    ACTION_DECLS;
};

struct PASource : public PActionBase {
    std::shared_ptr<pDomain> gen_pos;
    float particle_rate;
    pSourceState SrcSt;

    ACTION_DECLS;
};

struct PASpeedClamp : public PActionBase {
    float min_speed;
    float max_speed;

    ACTION_DECLS;
};

struct PATargetColor : public PActionBase {
    pVec color;
    float alpha;
    float scale;

    ACTION_DECLS;
};

struct PATargetSize : public PActionBase {
    pVec size;
    pVec scale;

    ACTION_DECLS;
};

struct PATargetVelocity : public PActionBase {
    pVec velocity;
    float scale;

    ACTION_DECLS;
};

struct PATargetRotVelocity : public PActionBase {
    pVec velocity;
    float scale;

    ACTION_DECLS;
};

struct PAVortex : public PActionBase {
    pVec tip;
    pVec axis;
    float tightnessExponent;
    float max_radius;
    float inSpeed;
    float upSpeed;
    float aroundSpeed;

    ACTION_DECLS;
};
}; // namespace PAPI

#undef ACTION_DECLS

#endif
