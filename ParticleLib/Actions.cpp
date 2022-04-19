/// Actions.cpp
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// The legacy implementation of dynamics of particle actions are here.
/// Each action has its own loop over all the particles.
/// This is inefficient, both in terms of minimal opportunities for code optimization, and in terms of memory access.

#include "Actions.h"

#include "PInternalState.h"
#include "Particle/pActionImpls.h"

#include <algorithm>
#include <execution>
#include <sstream>
#include <string>
#include <typeinfo>

// Remove these if not C++17.
// #define P_EXPOL std::execution::par
// #define P_EXPOL std::execution::par_unseq
#define P_EXPOL std::execution::seq

// For some reason most actions are slower with par_unseq. Use the following on the ones that get a speedup.
// #define P_EXPOLP std::execution::seq
#define P_EXPOLP std::execution::par_unseq

namespace PAPI {

std::string PActionBase::name = "PActionBase";
std::string PActionBase::abrv = "XXX";

std::string PAAvoid::abrv = "Av";
std::string PAAvoid::name = "PAAvoid";
std::string PABounce::abrv = "Bo";
std::string PABounce::name = "PABounce";
std::string PACallActionList::abrv = "CAL";
std::string PACallActionList::name = "PACallActionList";
std::string PACallback::abrv = "CB";
std::string PACallback::name = "PACallback";
std::string PACommitKills::abrv = "CK";
std::string PACommitKills::name = "PACommitKills";
std::string PACopyVertexB::abrv = "CVB";
std::string PACopyVertexB::name = "PACopyVertexB";
std::string PADamping::abrv = "Da";
std::string PADamping::name = "PADamping";
std::string PAExplosion::abrv = "Ex";
std::string PAExplosion::name = "PAExplosion";
std::string PAFollow::abrv = "Fo";
std::string PAFollow::name = "PAFollow";
std::string PAGravitate::abrv = "Gre";
std::string PAGravitate::name = "PAGravitate";
std::string PAGravity::abrv = "Gr";
std::string PAGravity::name = "PAGravity";
std::string PAJet::abrv = "Jet";
std::string PAJet::name = "PAJet";
std::string PAKillOld::abrv = "KO";
std::string PAKillOld::name = "PAKillOld";
std::string PAMatchRotVelocity::abrv = "MRV";
std::string PAMatchRotVelocity::name = "PAMatchRotVelocity";
std::string PAMatchVelocity::abrv = "MV";
std::string PAMatchVelocity::name = "PAMatchVelocity";
std::string PAMove::abrv = "Mo";
std::string PAMove::name = "PAMove";
std::string PAOrbitLine::abrv = "OL";
std::string PAOrbitLine::name = "PAOrbitLine";
std::string PAOrbitPoint::abrv = "OP";
std::string PAOrbitPoint::name = "PAOrbitPoint";
std::string PARandomAccel::abrv = "RA";
std::string PARandomAccel::name = "PARandomAccel";
std::string PARandomDisplace::abrv = "RD";
std::string PARandomDisplace::name = "PARandomDisplace";
std::string PARandomRotVelocity::abrv = "RRV";
std::string PARandomRotVelocity::name = "PARandomRotVelocity";
std::string PARandomVelocity::abrv = "RV";
std::string PARandomVelocity::name = "PARandomVelocity";
std::string PARestore::abrv = "Re";
std::string PARestore::name = "PARestore";
std::string PARotDamping::abrv = "RT";
std::string PARotDamping::name = "PARotDamping";
std::string PASink::abrv = "Si";
std::string PASink::name = "PASink";
std::string PASinkVelocity::abrv = "SV";
std::string PASinkVelocity::name = "PASinkVelocity";
std::string PASort::abrv = "Srt";
std::string PASort::name = "PASort";
std::string PASource::abrv = "Src";
std::string PASource::name = "PASource";
std::string PASpeedClamp::abrv = "SL";
std::string PASpeedClamp::name = "PASpeedClamp";
std::string PATargetColor::abrv = "TC";
std::string PATargetColor::name = "PATargetColor";
std::string PATargetRotVelocity::abrv = "TRV";
std::string PATargetRotVelocity::name = "PATargetRotVelocity";
std::string PATargetSize::abrv = "TS";
std::string PATargetSize::name = "PATargetSize";
std::string PATargetVelocity::abrv = "TV";
std::string PATargetVelocity::name = "PATargetVelocity";
std::string PAVortex::abrv = "Vo";
std::string PAVortex::name = "PAVortex";

void PAAvoid::Exec(const PDTriangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOLP, ibegin, iend, [&](Particle_t& m) { PAAvoidTriangle_Impl(m, dt, dom, look_ahead, magnitude, epsilon); });
}

void PAAvoid::Exec(const PDRectangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOLP, ibegin, iend, [&](Particle_t& m) { PAAvoidRectangle_Impl(m, dt, dom, look_ahead, magnitude, epsilon); });
}

void PAAvoid::Exec(const PDPlane& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOLP, ibegin, iend, [&](Particle_t& m) { PAAvoidPlane_Impl(m, dt, dom, look_ahead, magnitude, epsilon); });
}

// Only works for points on the OUTSIDE of the sphere. Ignores inner radius.
void PAAvoid::Exec(const PDSphere& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOLP, ibegin, iend, [&](Particle_t& m) { PAAvoidSphere_Impl(m, dt, dom, look_ahead, magnitude, epsilon); });
}

void PAAvoid::Exec(const PDDisc& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOLP, ibegin, iend, [&](Particle_t& m) { PAAvoidDisc_Impl(m, dt, dom, look_ahead, magnitude, epsilon); });
}

void PAAvoid::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    switch (position->Which) {
    // case PDBlob_e: Exec(*dynamic_cast<const PDBlob_e*>(position.get()), group, ibegin, iend); return;
    // case PDBox_e: Exec(*dynamic_cast<const PDBox_e*>(position.get()), group, ibegin, iend); return;
    // case PDCone_e: Exec(*dynamic_cast<const PDCone_e*>(position.get()), group, ibegin, iend); return;
    // case PDCylinder_e: Exec(*dynamic_cast<const PDCylinder_e*>(position.get()), group, ibegin, iend); return;
    // case PDLine_e: Exec(*dynamic_cast<const PDLine_e*>(position.get()), group, ibegin, iend); return;
    // case PDPoint_e: Exec(*dynamic_cast<const PDPoint_e*>(position.get()), group, ibegin, iend); return;
    // case PDUnion_e: Exec(*dynamic_cast<const PDUnion_e*>(position.get()), group, ibegin, iend); return;
    case PDDisc_e: Exec(*dynamic_cast<const PDDisc*>(position.get()), group, ibegin, iend); return;
    case PDPlane_e: Exec(*dynamic_cast<const PDPlane*>(position.get()), group, ibegin, iend); return;
    case PDRectangle_e: Exec(*dynamic_cast<const PDRectangle*>(position.get()), group, ibegin, iend); return;
    case PDSphere_e: Exec(*dynamic_cast<const PDSphere*>(position.get()), group, ibegin, iend); return;
    case PDTriangle_e: Exec(*dynamic_cast<const PDTriangle*>(position.get()), group, ibegin, iend); return;
    default: throw PErrNotImplemented(std::string("Avoid not implemented for domain ") + std::string(typeid(position.get()).name()));
    }
}

// Bounce() doesn't work correctly with small time step sizes for particles sliding along a surface.
// The friction and resilience parameters should not be scaled by dt, since a bounce happens instantaneously.
// On the other hand, they should be scaled by dt because particles sliding along a surface will hit more
// often if dt is smaller. If you have any suggestions, let me know.
//
// This approach uses the actual hit location and hit time to determine whether we actually hit.
// But it doesn't bounce from the actual location or time. It reverses the velocity immediately, applying
// the whole velocity in the outward direction for the whole time step.
void PABounce::Exec(const PDTriangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceTriangle_Impl(m, dt, dom, friction, resilience, fric_min_vel); });
}

void PABounce::Exec(const PDRectangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceRectangle_Impl(m, dt, dom, friction, resilience, fric_min_vel); });
}

void PABounce::Exec(const PDBox& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceBox_Impl(m, dt, dom, friction, resilience, fric_min_vel); });
}

void PABounce::Exec(const PDPlane& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABouncePlane_Impl(m, dt, dom, friction, resilience, fric_min_vel); });
}

void PABounce::Exec(const PDSphere& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(dom.radIn == 0.0f, "Bouncing doesn't work on thick shells. radIn must be 0.");

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceSphere_Impl(m, dt, dom, friction, resilience, fric_min_vel); });
}

void PABounce::Exec(const PDDisc& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceDisc_Impl(m, dt, dom, friction, resilience, fric_min_vel); });
}

void PABounce::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    switch (position->Which) {
    // case PDBlob_e: Exec(*dynamic_cast<const PDBlob*>(position.get()), group, ibegin, iend); return;
    // case PDCone_e: Exec(*dynamic_cast<const PDCone*>(position.get()), group, ibegin, iend); return;
    // case PDCylinder_e: Exec(*dynamic_cast<const PDCylinder*>(position.get()), group, ibegin, iend); return;
    // case PDLine_e: Exec(*dynamic_cast<const PDLine*>(position.get()), group, ibegin, iend); return;
    // case PDPoint_e: Exec(*dynamic_cast<const PDPoint*>(position.get()), group, ibegin, iend); return;
    // case PDUnion_e: Exec(*dynamic_cast<const PDUnion*>(position.get()), group, ibegin, iend); return;
    case PDBox_e: Exec(*dynamic_cast<const PDBox*>(position.get()), group, ibegin, iend); return;
    case PDDisc_e: Exec(*dynamic_cast<const PDDisc*>(position.get()), group, ibegin, iend); return;
    case PDPlane_e: Exec(*dynamic_cast<const PDPlane*>(position.get()), group, ibegin, iend); return;
    case PDRectangle_e: Exec(*dynamic_cast<const PDRectangle*>(position.get()), group, ibegin, iend); return;
    case PDSphere_e: Exec(*dynamic_cast<const PDSphere*>(position.get()), group, ibegin, iend); return;
    case PDTriangle_e: Exec(*dynamic_cast<const PDTriangle*>(position.get()), group, ibegin, iend); return;
    default: throw PErrNotImplemented(std::string("Bounce not implemented for domain ") + std::string(typeid(position.get()).name()));
    }
}

// Set the secondary position and velocity from current.
void PACopyVertexB::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PACopyVertexB_Impl(m, dt, copy_pos, copy_vel); });
}

// Dampen velocities
void PADamping::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PADamping_Impl(m, dt, damping, min_vel, max_vel); });
}

// Dampen rotational velocities
void PARotDamping::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PARotDamping_Impl(m, dt, damping, min_vel, max_vel); });
}

// Exert force on each particle away from explosion center
void PAExplosion::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAExplosion_Impl(m, dt, center, radius, magnitude, stdev, epsilon); });
}

// Acceleration in a constant direction
void PAGravity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAGravity_Impl(m, dt, direction); });
}

// For particles in the domain of influence, accelerate them with a domain.
void PAJet::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAJet_Impl(m, dt, *dom, *acc); });
}

// Apply the particles' velocities to their positions, and age the particles
void PAMove::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAMove_Impl(m, dt, move_velocity, move_rotational_velocity); });
}

// Accelerate particles towards a line
void PAOrbitLine::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAOrbitLine_Impl(m, dt, p, axis, magnitude, epsilon, max_radius); });
}

// Accelerate particles towards a point
void PAOrbitPoint::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAOrbitPoint_Impl(m, dt, center, magnitude, epsilon, max_radius); });
}

// Accelerate in random direction each time step
void PARandomAccel::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PARandomAccel_Impl(m, dt, *gen_acc); });
}

// Immediately displace position randomly
void PARandomDisplace::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PARandomDisplace_Impl(m, dt, *gen_disp); });
}

// Immediately assign a random velocity
void PARandomVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PARandomVelocity_Impl(m, dt, *gen_vel); });
}

// Immediately assign a random rotational velocity
void PARandomRotVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PARandomRotVelocity_Impl(m, dt, *gen_vel); });
}

// Over time, restore particles to initial positions
void PARestore::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PARestore_Impl(m, dt, time_left, restore_velocity, restore_rvelocity); });
}

// Clamp particle velocities to the given range
void PASpeedClamp::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PASpeedClamp_Impl(m, dt, min_speed, max_speed); });
}

// Change color of all particles toward the specified color
void PATargetColor::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PATargetColor_Impl(m, dt, color, alpha, scale); });
}

// Change sizes of all particles toward the specified size
void PATargetSize::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PATargetSize_Impl(m, dt, size, scale); });
}

// Change velocity of all particles toward the specified velocity
void PATargetVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PATargetVelocity_Impl(m, dt, velocity, scale); });
}

// Change velocity of all particles toward the specified velocity
void PATargetRotVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PATargetRotVelocity_Impl(m, dt, velocity, scale); });
}

void PAVortex::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAVortex_Impl(m, dt, tip, axis, tightnessExponent, max_radius, inSpeed, upSpeed, aroundSpeed); });
}

//////////////////////////////////////////////////////////////////
// Inter-particle actions

// Follow the next particle in the list
void PAFollow::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");
    if (group.size() < 2) return;
    const Particle_t* endp = &*ibegin + (iend - ibegin);
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAFollow_Impl(m, dt, magnitude, epsilon, max_radius, &*ibegin, endp); });
}

// Inter-particle gravitation
void PAGravitate::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");
    if (group.size() < 2) return;
    const Particle_t* endp = &*ibegin + (iend - ibegin);
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAGravitate_Impl(m, dt, magnitude, epsilon, max_radius, &*ibegin, endp); });
}

// Match velocity to near neighbors
void PAMatchVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");
    if (group.size() < 2) return;
    const Particle_t* endp = &*ibegin + (iend - ibegin);
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAMatchVelocity_Impl(m, dt, magnitude, epsilon, max_radius, &*ibegin, endp); });
}

// Match rotational velocity to near neighbors
void PAMatchRotVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");
    if (group.size() < 2) return;
    const Particle_t* endp = &*ibegin + (iend - ibegin);
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PAMatchRotVelocity_Impl(m, dt, magnitude, epsilon, max_radius, &*ibegin, endp); });
}

//////////////////////////////////////////////////////////////////
// Other exceptional actions

// An action list within an action list
void PACallActionList::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    // Execute the specified action list.
    PS->ExecuteActionList(PS->ALists[action_list_num]);
}

void PACallback::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    if (callbackFunc == NULL) return;

    // TODO: Can we parallelize this? Should we let the app specify whether to call back in parallel?
    for (ParticleList::iterator it = ibegin; it != iend; it++) {
        Particle_t& m = (*it);
        (*callbackFunc)(m, call_data, dt);
    }
}

// Delete particles tagged to be killed by inline P.I.KillOld(), P.I.Sink(), and P.I.SinkVelocity()
void PACommitKills::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

#if 0
    // Slower on Fountain and Waterfall and maybe all
    ParticleList::iterator first_goner = std::partition(P_EXPOLP, ibegin, iend, [](Particle_t& m) { return m.tmp0 != P_MAXFLOAT; });
    group.RemoveRange(first_goner, iend);
#else
    // Must traverse list carefully so Remove will work
    for (ParticleList::iterator it = ibegin; it != iend;) {
        Particle_t& m = (*it);
        if (m.tmp0 == P_MAXFLOAT) {
            it = group.Remove(it);
            iend = group.end();
        } else
            ++it;
    }

#endif
}

// Get rid of older particles
void PAKillOld::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    // Must traverse list carefully so Remove will work
    for (ParticleList::iterator it = ibegin; it != iend;) {
        Particle_t& m = (*it);
        PAKillOld_Impl(m, dt, age_limit, kill_less_than);
        if (m.tmp0 == P_MAXFLOAT) {
            it = group.Remove(it);
            iend = group.end();
        } else
            ++it;
    }
}

// Kill particles with positions on wrong side of the specified domain
void PASink::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    // Must traverse list carefully so Remove will work
    // TODO: Would it be faster to parallelize the loop over the predicate and then do a partition?
    for (ParticleList::iterator it = ibegin; it != iend;) {
        Particle_t& m = (*it);
        PASink_Impl(m, dt, kill_inside, *kill_pos_dom);
        if (m.tmp0 == P_MAXFLOAT) {
            it = group.Remove(it);
            iend = group.end();
        } else
            ++it;
    }
}

// Kill particles with velocities on wrong side of the specified domain
void PASinkVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    // Must traverse list carefully so Remove will work
    for (ParticleList::iterator it = ibegin; it != iend;) {
        Particle_t& m = (*it);
        PASinkVelocity_Impl(m, dt, kill_inside, *kill_vel_dom);
        if (m.tmp0 == P_MAXFLOAT) {
            it = group.Remove(it);
            iend = group.end();
        } else
            ++it;
    }
}

// Sort the particles by their projection onto the Look vector
void PASort::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PASort_Impl(m, dt, Eye, Look, front_to_back, clamp_negative); });

    std::sort(P_EXPOL, ibegin, iend);
}

// Randomly add particles to the system
void PASource::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    LIB_ASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    size_t rate = SourceQuantity(particle_rate, dt, group.size(), group.GetMaxParticles());

    for (size_t i = 0; i < rate; i++) {
        Particle_t m;
        PASource_Impl(m, dt, *gen_pos, SrcSt);
        group.Add(m);
    }
}
}; // namespace PAPI
