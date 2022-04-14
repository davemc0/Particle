/// Actions.cpp
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// This file implements the dynamics of particle actions.

#include "Actions.h"

#include "ImplActions.h"
#include "PInternalState.h"

#include <algorithm>
#include <typeinfo>
// For dumping errors
#include <execution>
#include <sstream>
#include <string>

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
    // Can build generic bounce function that works on any domain by using the Within function and a normal.

    switch (position->Which) {
    // case PDUnion_e: Exec(*dynamic_cast<const PDUnion_e*>(position.get()), group, ibegin, iend); return;
    // case PDPoint_e: Exec(*dynamic_cast<const PDPoint_e*>(position.get()), group, ibegin, iend); return;
    // case PDLine_e: Exec(*dynamic_cast<const PDLine_e*>(position.get()), group, ibegin, iend); return;
    case PDTriangle_e: Exec(*dynamic_cast<const PDTriangle*>(position.get()), group, ibegin, iend); return;
    case PDRectangle_e: Exec(*dynamic_cast<const PDRectangle*>(position.get()), group, ibegin, iend); return;
    case PDDisc_e: Exec(*dynamic_cast<const PDDisc*>(position.get()), group, ibegin, iend); return;
    case PDPlane_e: Exec(*dynamic_cast<const PDPlane*>(position.get()), group, ibegin, iend); return;
    // case PDBox_e: Exec(*dynamic_cast<const PDBox_e*>(position.get()), group, ibegin, iend); return;
    // case PDCylinder_e: Exec(*dynamic_cast<const PDCylinder_e*>(position.get()), group, ibegin, iend); return;
    // case PDCone_e: Exec(*dynamic_cast<const PDCone_e*>(position.get()), group, ibegin, iend); return;
    case PDSphere_e: Exec(*dynamic_cast<const PDSphere*>(position.get()), group, ibegin, iend); return;
    // case PDBlob_e: Exec(*dynamic_cast<const PDBlob_e*>(position.get()), group, ibegin, iend); return;
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
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceTriangle_Impl(m, dt, dom, oneMinusFriction, resilience, cutoffSqr); });
}

void PABounce::Exec(const PDRectangle& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceRectangle_Impl(m, dt, dom, oneMinusFriction, resilience, cutoffSqr); });
}

void PABounce::Exec(const PDBox& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceBox_Impl(m, dt, dom, oneMinusFriction, resilience, cutoffSqr); });
}

void PABounce::Exec(const PDPlane& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABouncePlane_Impl(m, dt, dom, oneMinusFriction, resilience, cutoffSqr); });
}

void PABounce::Exec(const PDSphere& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(dom.radIn == 0.0f, "Bouncing doesn't work on thick shells. radIn must be 0.");

    float dtinv = 1.0f / dt;
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceSphere_Impl(m, dt, dom, oneMinusFriction, resilience, cutoffSqr); });
}

void PABounce::Exec(const PDDisc& dom, ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { PABounceDisc_Impl(m, dt, dom, oneMinusFriction, resilience, cutoffSqr); });
}

void PABounce::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    // Can build generic bounce function that works on any domain by using the Within function and a normal.
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

// An action list within an action list
void PACallActionList::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    // Execute the specified action list.
    PS->ExecuteActionList(PS->ALists[action_list_num]);
}

void PACallback::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    if (callbackFunc == NULL) return;

    // TODO: Can we parallelize this? Should we let the app specify whether we can?
    for (ParticleList::iterator it = ibegin; it != iend; it++) {
        Particle_t& m = (*it);
        (*callbackFunc)(m, Data, dt);
    }
}

// Set the secondary position and velocity from current.
void PACopyVertexB::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    if (copy_pos && copy_vel) {
        std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
            m.posB = m.pos;
            m.upB = m.up;
            m.velB = m.vel;
        });
    } else if (copy_pos) {
        std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
            m.posB = m.pos;
            m.upB = m.up;
        });
    } else if (copy_vel) {
        std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { m.velB = m.vel; });
    }
}

// Dampen velocities
void PADamping::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    // This is important if dt is != 1.
    pVec one = pVec(1, 1, 1);
    pVec scale(one - ((one - damping) * dt));

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        float vSqr = m.vel.lenSqr();

        if (vSqr >= vlowSqr && vSqr <= vhighSqr) { m.vel = CompMult(m.vel, scale); }
    });
}

// Dampen rotational velocities
void PARotDamping::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    // This is important if dt is != 1.
    pVec one = pVec(1, 1, 1);
    pVec scale(one - ((one - damping) * dt));

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        float vSqr = m.rvel.lenSqr();

        if (vSqr >= vlowSqr && vSqr <= vhighSqr) { m.rvel = CompMult(m.rvel, scale); }
    });
}

// Exert force on each particle away from explosion center
void PAExplosion::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    float magdt = magnitude * dt;
    float oneOverSigma = 1.0f / stdev;
    float inexp = -0.5f * fsqr(oneOverSigma);
    float outexp = P_ONEOVERSQRT2PI * oneOverSigma;

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        // Figure direction to particle.
        pVec dir(m.pos - center);
        float distSqr = dir.lenSqr();
        float dist = sqrtf(distSqr);
        float DistFromWaveSqr = fsqr(radius - dist);

        float Gd = exp(DistFromWaveSqr * inexp) * outexp;
        pVec acc(dir * (Gd * magdt / (dist * (distSqr + epsilon))));

        m.vel += acc;
    });
}

// Follow the next particle in the list
void PAFollow::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);

    if (group.size() < 2) return;

    ParticleList::iterator end = iend;
    end--;

    // TODO: Parallelize this
    for (ParticleList::iterator it = ibegin; it != end; it++) {
        Particle_t& m = (*it);
        ParticleList::iterator next = it;
        next++;

        // Accelerate toward the particle after me in the list.
        pVec toHim((*next).pos - m.pos); // toHim = p1 - p0
        float toHimlenSqr = toHim.lenSqr();

        if (toHimlenSqr < max_radiusSqr) {
            // Compute force exerted between the two bodies
            m.vel += toHim * (magdt / (sqrtf(toHimlenSqr) * (toHimlenSqr + epsilon)));
        }
    }
}

// Inter-particle gravitation
void PAGravitate::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);

    std::for_each(P_EXPOLP, ibegin, iend, [&](Particle_t& m) {
        // Add interactions with other particles
        for (ParticleList::iterator j = ibegin; j != iend; ++j) {
            Particle_t& mj = (*j);

            pVec toHim(mj.pos - m.pos); // toHim = p1 - p0
            float toHimlenSqr = toHim.lenSqr();
            if (toHimlenSqr > 0.f && toHimlenSqr < max_radiusSqr) {
                // Compute force exerted between the two bodies
                pVec acc(toHim * (magdt / (sqrtf(toHimlenSqr) * (toHimlenSqr + epsilon))));

                m.vel += acc;
            }
        }
    });
}

// Acceleration in a constant direction
void PAGravity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    pVec ddir(direction * dt);

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { m.vel += ddir; });
}

// For particles in the domain of influence, accelerate them with a domain.
void PAJet::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        if (dom->Within(m.pos)) {
            pVec accel = acc->Generate();

            m.vel += accel * dt;
        }
    });
}

// Get rid of older particles
void PAKillOld::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    // Must traverse list carefully so Remove will work
    // TODO: Parallelize this. Perhaps use a partition primitive.
    for (ParticleList::iterator it = ibegin; it != iend;) {
        Particle_t& m = (*it);

        if (!((m.age < age_limit) ^ kill_less_than)) {
            it = group.Remove(it);
            iend = group.end();
        } else
            it++;
    }
}

// Match velocity to near neighbors
void PAMatchVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);

    std::for_each(P_EXPOLP, ibegin, iend, [&](Particle_t& m) {
        // Add interactions with other particles
        for (ParticleList::iterator j = ibegin; j != iend; ++j) {
            Particle_t& mj = (*j);

            pVec toHim(mj.pos - m.pos); // toHim = p1 - p0
            float toHimlenSqr = toHim.lenSqr();
            if (toHimlenSqr > 0.f && toHimlenSqr < max_radiusSqr) {
                // Compute force exerted between the two bodies
                pVec veltoHim(mj.vel - m.vel);
                pVec acc(veltoHim * (magdt / (toHimlenSqr + epsilon)));

                m.vel += acc;
            }
        }
    });
}

// Match rotational velocity to near neighbors
void PAMatchRotVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);

    std::for_each(P_EXPOLP, ibegin, iend, [&](Particle_t& m) {
        // Add interactions with other particles
        for (ParticleList::iterator j = ibegin; j != iend; ++j) {
            Particle_t& mj = (*j);

            pVec toHim(mj.pos - m.pos); // toHim = p1 - p0
            float toHimlenSqr = toHim.lenSqr();
            if (toHimlenSqr > 0.f && toHimlenSqr < max_radiusSqr) {
                // Compute force exerted between the two bodies
                pVec rveltoHim(mj.rvel - m.rvel);
                pVec acc(rveltoHim * (magdt / (toHimlenSqr + epsilon)));

                m.rvel += acc;
            }
        }
    });
}

// Apply the particles' velocities to their positions, and age the particles
void PAMove::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    if (move_velocity && move_rotational_velocity) {
        std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
            m.age += dt;
            m.pos += m.vel * dt;
            m.up += m.rvel * dt;
        });
    } else if (move_rotational_velocity) {
        std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
            m.age += dt;
            m.up += m.rvel * dt;
        });
    } else {
        std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
            m.age += dt;
            m.pos += m.vel * dt;
        });
    }
}

// Accelerate particles towards a line
void PAOrbitLine::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        // Figure direction to particle from base of line.
        pVec f = m.pos - p;

        // Projection of particle onto line
        pVec w = axis * dot(f, axis);

        // Direction from particle to nearest point on line.
        pVec into = w - f;

        // Distance to line (force drops as 1/r^2, normalize by 1/r)
        // Soften by epsilon to avoid tight encounters to infinity
        float rSqr = into.lenSqr();

        if (rSqr < max_radiusSqr) m.vel += into * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
    });
}

// Accelerate particles towards a point
void PAOrbitPoint::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        // Figure direction from particle to center
        pVec dir(center - m.pos);

        // Distance to gravity well (force drops as 1/r^2, normalize by 1/r)
        // Soften by epsilon to avoid tight encounters to infinity
        float rSqr = dir.lenSqr();

        if (rSqr < max_radiusSqr) m.vel += dir * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
    });
}

// Accelerate in random direction each time step
void PARandomAccel::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        pVec accel = gen_acc->Generate();

        // Dt will affect this by making a higher probability of being near the original velocity after unit time.
        // Smaller dt approach a normal distribution instead of a square wave.
        m.vel += accel * dt;
    });
}

// Immediately displace position randomly
void PARandomDisplace::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        pVec disp = gen_disp->Generate();

        // Dt will affect this by making a higher probability of being near the original position after unit time.
        // Smaller dt approach a normal distribution instead of a square wave.
        m.pos += disp * dt;
    });
}

// Immediately assign a random velocity
void PARandomVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        pVec velocity = gen_vel->Generate();

        // Don't multiply by dt because velocities are invariant of dt.
        m.vel = velocity;
    });
}

// Immediately assign a random rotational velocity
void PARandomRotVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        pVec velocity = gen_vel->Generate();

        // Don't multiply by dt because velocities are invariant of dt.
        m.rvel = velocity;
    });
}

#if 0
    // Produce coefficients of a velocity function v(t)=at^2 + bt + c
    // satisfying initial x(0)=x0,v(0)=v0 and desired x(t)=x1,v(t)=v1,
    // where x = x(0) + integral(v(T),0,t)
    static inline void _pconstrain(float x0, float v0, float x1, float v1,
        float t, float *a, float *b, float *c)
    {
        *c = v0;
        *b = 2 * (-t*v1 - 2*t*v0 + 3*x1 - 3*x0) / (t * t);
        *a = 3 * (t*v1 + t*v0 - 2*x1 + 2*x0) / (t * t * t);
    }

    // Solve for a desired-behavior velocity function in each axis
    // _pconstrain(m.pos.x(), m.vel.x(), m.posB.x(), 0., timeLeft, &a, &b, &c);

    // Figure new velocity at next timestep
    // m.vel.x() = a * dtSqr + b * dt + c;
#endif

// Figure new velocity at next timestep
static inline void Restore(pVec& vel, const pVec& posB, const pVec& pos, const float t, const float dtSqr, const float ttInv6dt, const float tttInv3dtSqr)
{
    pVec b = (vel * -0.6667f * t + posB - pos) * ttInv6dt;
    pVec a = (vel * t - posB - posB + pos + pos) * tttInv3dtSqr;
    vel += a + b;
}

// Over time, restore particles to initial positions
void PARestore::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    if (time_left <= 0) {
        std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
            // Already constrained; keep it there.
            if (restore_velocity) {
                m.pos = m.posB;
                m.vel = pVec(0.0f, 0.0f, 0.0f);
            }
            if (restore_rvelocity) {
                m.up = m.upB;
                m.rvel = pVec(0.0f, 0.0f, 0.0f);
            }
        });
    } else {
        float t = time_left;
        float dtSqr = fsqr(dt);
        float ttInv6dt = dt * 6.0f / fsqr(t);
        float tttInv3dtSqr = dtSqr * 3.0f / (t * t * t);

        std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
            if (restore_velocity) Restore(m.vel, m.posB, m.pos, t, dtSqr, ttInv6dt, tttInv3dtSqr);
            if (restore_rvelocity) Restore(m.rvel, m.upB, m.up, t, dtSqr, ttInv6dt, tttInv3dtSqr);
        });
    }
}

// Kill particles with positions on wrong side of the specified domain
void PASink::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    // Must traverse list carefully so Remove will work
    // TODO: Parallelize this!
    for (ParticleList::iterator it = ibegin; it != iend;) {
        Particle_t& m = (*it);

        // Remove if inside/outside flag matches object's flag
        if (!(position->Within(m.pos) ^ kill_inside)) {
            it = group.Remove(it);
            iend = group.end();
        } else
            it++;
    }
}

// Kill particles with velocities on wrong side of the specified domain
void PASinkVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    // Must traverse list carefully so Remove will work
    // TODO: Parallelize this!
    for (ParticleList::iterator it = ibegin; it != iend;) {
        Particle_t& m = (*it);

        // Remove if inside/outside flag matches object's flag
        if (!(velocity->Within(m.vel) ^ kill_inside)) {
            it = group.Remove(it);
            iend = group.end();
        } else
            it++;
    }
}

// Sort the particles by their projection onto the Look vector
void PASort::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    float Scale = front_to_back ? -1.0f : 1.0f;

    // First compute projection of particle onto view vector
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        pVec ToP = m.pos - Eye;
        m.tmp0 = dot(ToP, Look) * Scale;
        if (clamp_negative && m.tmp0 < 0) m.tmp0 = 0.0f;
    });

    std::sort(P_EXPOL, ibegin, iend);
}

// Randomly add particles to the system
void PASource::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    PASSERT(ibegin == group.begin() && iend == group.end(), "Can only be done on whole list");

    size_t rate = size_t(floor(particle_rate * dt));

    // Dither the fractional particle in time.
    if (pRandf() < particle_rate * dt - float(rate)) rate++;

    // Don't emit more than it can hold.
    if (group.size() + rate > group.GetMaxParticles()) rate = group.GetMaxParticles() - group.size();

    for (size_t i = 0; i < rate; i++) {
        Particle_t P;

        P.pos = position->Generate();
        P.posB = SrcSt.vertexB_tracks_ ? P.pos : SrcSt.VertexB_->Generate();
        P.up = SrcSt.Up_->Generate();
        P.vel = SrcSt.Vel_->Generate();
        P.rvel = SrcSt.RotVel_->Generate();
        P.size = SrcSt.Size_->Generate();
        P.color = SrcSt.Color_->Generate();
        P.alpha = SrcSt.Alpha_->Generate().x();
        P.age = SrcSt.Age_ + pNRandf(SrcSt.AgeSigma_);
        P.mass = SrcSt.Mass_;
        P.data = SrcSt.Data_;

        group.Add(P);
    }
}

// Clamp particle velocities to the given range
void PASpeedClamp::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    float min_sqr = fsqr(min_speed);
    float max_sqr = fsqr(max_speed);

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        float sSqr = m.vel.lenSqr();
        if (sSqr < min_sqr && sSqr) {
            float s = sqrtf(sSqr);
            m.vel *= (min_speed / s);
        } else if (sSqr > max_sqr) {
            float s = sqrtf(sSqr);
            m.vel *= (max_speed / s);
        }
    });
}

// Change color of all particles toward the specified color
void PATargetColor::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    float scaleFac = scale * dt;

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        m.color += (color - m.color) * scaleFac;
        m.alpha += (alpha - m.alpha) * scaleFac;
    });
}

// Change sizes of all particles toward the specified size
void PATargetSize::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    pVec scaleFac = scale * dt;

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        pVec dif = size - m.size;
        m.size += CompMult(dif, scaleFac);
    });
}

// Change velocity of all particles toward the specified velocity
void PATargetVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    float scaleFac = scale * dt;

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { m.vel += (velocity - m.vel) * scaleFac; });
}

// Change velocity of all particles toward the specified velocity
void PATargetRotVelocity::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    float scaleFac = scale * dt;

    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) { m.rvel += (velocity - m.rvel) * scaleFac; });
}

void PAVortex::Execute(ParticleGroup& group, ParticleList::iterator ibegin, ParticleList::iterator iend)
{
    float max_radiusSqr = fsqr(max_radius);
    float axisLength = axis.length();
    float axisLengthInv = 1.0f / axisLength;
    pVec axisN = axis;
    axisN.normalize();

    // This one just rotates a particle around the axis. Amount is based on radius, magnitude, and mass.
    std::for_each(P_EXPOL, ibegin, iend, [&](Particle_t& m) {
        // Direction to particle from base of line.
        pVec tipToPar = m.pos - tip;

        // Projection of particle onto line
        float axisScale = dot(tipToPar, axisN);
        pVec parOnAxis = axisN * axisScale;

        // Distance to axis
        float alongAxis = axisScale * axisLengthInv;

        // How much to scale the vortex's force by as a function of how far up the axis the particle is.
        float alongAxisPow = powf(alongAxis, tightnessExponent);
        float silhouetteSqr = fsqr(alongAxisPow * max_radius);

        // Direction from particle to nearest point on line.
        pVec parToAxis = parOnAxis - tipToPar;
        float rSqr = parToAxis.lenSqr();

        if (rSqr >= max_radiusSqr || axisScale < 0.0f || alongAxis > 1.0f) {
            // m.color = pVec(0,0,1);
            return;
        }

        float r = sqrtf(rSqr);
        parToAxis /= r;
        float dtOverMass = dt / m.mass;

        if (rSqr >= silhouetteSqr) {
            // Accelerate toward axis. Force is NOT affected by 1/r^2.
            pVec AccelIn = parToAxis * (inSpeed * dtOverMass);
            m.vel += AccelIn;
            // m.color = pVec(0,1,0);
            return;
        }

        // Particles inside the cone have their velocity totally replaced right now. :(
        // m.color = pVec(1,0,0);
        // Accelerate up or down to simulate gravity or something
        pVec AccelUp = axisN * (upSpeed * dtOverMass);

        // Accelerate around axis by constructing orthogonal vector frame of axis, parToAxis, and RotDir.
        pVec RotDir = Cross(axisN, parToAxis);
        pVec AccelAround = RotDir * (aroundSpeed * dtOverMass);
        m.vel = AccelUp + AccelAround; // NOT += because we want to stop its inward travel.
    });
}
}; // namespace PAPI
