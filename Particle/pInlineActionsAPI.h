/// PInlineActionsAPI.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// Only application code should include this.
/// This file contains the host interface definitions to the inline particle actions.
/// It is a header file so that the action implementations are inlined.
/// The intended usage is as follows:
///
/// P.ParticleLoop(std::execution::par_unseq, [&](Particle_t& m) {
///     P.I.Gravity(Efx.GravityVec);
///     P.I.Bounce(0.f, 0.5f, 0.f, PDDisc(pVec(0, 0, 1.f), pVec(0, 0, 1.f), 5));
///     P.I.Move(true, false);
/// });

#include "Particle/pAPI.h"
#include "Particle/pActionImpls.h"
#include "ParticleLib/PInternalState.h"

namespace PAPI {

PINLINE void PContextInlineActions_t::Avoid(Particle_t& m, const float magnitude, const float epsilon, const float look_ahead, const pDomain& dom)
{
    switch (dom.Which) {
    case PDDisc_e: PAAvoidDisc_Impl(m, PS->dt, *dynamic_cast<const PDDisc*>(&dom), look_ahead, magnitude, epsilon); return;
    case PDPlane_e: PAAvoidPlane_Impl(m, PS->dt, *dynamic_cast<const PDPlane*>(&dom), look_ahead, magnitude, epsilon); return;
    case PDRectangle_e: PAAvoidRectangle_Impl(m, PS->dt, *dynamic_cast<const PDRectangle*>(&dom), look_ahead, magnitude, epsilon); return;
    case PDSphere_e: PAAvoidSphere_Impl(m, PS->dt, *dynamic_cast<const PDSphere*>(&dom), look_ahead, magnitude, epsilon); return;
    case PDTriangle_e: PAAvoidTriangle_Impl(m, PS->dt, *dynamic_cast<const PDTriangle*>(&dom), look_ahead, magnitude, epsilon); return;
    default: return;
    }
}

PINLINE void PContextInlineActions_t::Bounce(Particle_t& m, const float friction, const float resilience, const float fric_min_vel, const pDomain& dom)
{
    switch (dom.Which) {
    case PDBox_e: PABounceBox_Impl(m, PS->dt, *static_cast<const PDBox*>(&dom), friction, resilience, fric_min_vel); return;
    case PDDisc_e: PABounceDisc_Impl(m, PS->dt, *static_cast<const PDDisc*>(&dom), friction, resilience, fric_min_vel); return;
    case PDPlane_e: PABouncePlane_Impl(m, PS->dt, *static_cast<const PDPlane*>(&dom), friction, resilience, fric_min_vel); return;
    case PDRectangle_e: PABounceRectangle_Impl(m, PS->dt, *static_cast<const PDRectangle*>(&dom), friction, resilience, fric_min_vel); return;
    case PDSphere_e: PABounceSphere_Impl(m, PS->dt, *static_cast<const PDSphere*>(&dom), friction, resilience, fric_min_vel); return;
    case PDTriangle_e: PABounceTriangle_Impl(m, PS->dt, *static_cast<const PDTriangle*>(&dom), friction, resilience, fric_min_vel); return;
    default: return;
    }
}

PINLINE void PContextInlineActions_t::CopyVertexB(Particle_t& m, const bool copy_pos, const bool copy_vel)
{
    PACopyVertexB_Impl(m, PS->dt, copy_pos, copy_vel);
}

PINLINE void PContextInlineActions_t::Damping(Particle_t& m, const pVec& damping, const float min_vel, const float max_vel)
{
    PADamping_Impl(m, PS->dt, damping, min_vel, max_vel);
}

PINLINE void PContextInlineActions_t::RotDamping(Particle_t& m, const pVec& damping, const float min_vel, const float max_vel)
{
    PARotDamping_Impl(m, PS->dt, damping, min_vel, max_vel);
}

PINLINE void PContextInlineActions_t::Explosion(Particle_t& m, const pVec& center, const float radius, const float magnitude, const float stdev,
                                                const float epsilon)
{
    PAExplosion_Impl(m, PS->dt, center, radius, magnitude, stdev, epsilon);
}

PINLINE void PContextInlineActions_t::Gravity(Particle_t& m, const pVec& dir) { PAGravity_Impl(m, PS->dt, dir); }

PINLINE void PContextInlineActions_t::Jet(Particle_t& m, const pDomain& dom, const pDomain& accel) { PAJet_Impl(m, PS->dt, dom, accel); }

PINLINE void PContextInlineActions_t::Move(Particle_t& m, const bool move_velocity, const bool move_rotational_velocity)
{
    PAMove_Impl(m, PS->dt, move_velocity, move_rotational_velocity);
}

PINLINE void PContextInlineActions_t::OrbitLine(Particle_t& m, const pVec& p, const pVec& axis, const float magnitude, const float epsilon, const float max_radius)
{
    PAOrbitLine_Impl(m, PS->dt, p, axis, magnitude, epsilon, max_radius);
}

PINLINE void PContextInlineActions_t::OrbitPoint(Particle_t& m, const pVec& center, const float magnitude, const float epsilon, const float max_radius)
{
    PAOrbitPoint_Impl(m, PS->dt, center, magnitude, epsilon, max_radius);
}

PINLINE void PContextInlineActions_t::RandomAccel(Particle_t& m, const pDomain& gen_acc) { PARandomAccel_Impl(m, PS->dt, gen_acc); }

PINLINE void PContextInlineActions_t::RandomDisplace(Particle_t& m, const pDomain& gen_disp) { PARandomDisplace_Impl(m, PS->dt, gen_disp); }

PINLINE void PContextInlineActions_t::RandomVelocity(Particle_t& m, const pDomain& gen_vel) { PARandomVelocity_Impl(m, PS->dt, gen_vel); }

PINLINE void PContextInlineActions_t::RandomRotVelocity(Particle_t& m, const pDomain& gen_vel) { PARandomRotVelocity_Impl(m, PS->dt, gen_vel); }

PINLINE void PContextInlineActions_t::Restore(Particle_t& m, const float time_left, const bool vel, const bool rvel)
{
    PARestore_Impl(m, PS->dt, time_left, vel, rvel);
}

PINLINE void PContextInlineActions_t::SpeedClamp(Particle_t& m, const float min_speed, const float max_speed)
{
    PASpeedClamp_Impl(m, PS->dt, min_speed, max_speed);
}

PINLINE void PContextInlineActions_t::TargetColor(Particle_t& m, const pVec& color, const float alpha, const float scale)
{
    PATargetColor_Impl(m, PS->dt, color, alpha, scale);
}

PINLINE void PContextInlineActions_t::TargetSize(Particle_t& m, const pVec& size, const pVec& scale) { PATargetSize_Impl(m, PS->dt, size, scale); }

PINLINE void PContextInlineActions_t::TargetVelocity(Particle_t& m, const pVec& vel, const float scale) { PATargetVelocity_Impl(m, PS->dt, vel, scale); }

PINLINE void PContextInlineActions_t::TargetRotVelocity(Particle_t& m, const pVec& rot_velocity, const float scale)
{
    PATargetRotVelocity_Impl(m, PS->dt, rot_velocity, scale);
}

PINLINE void PContextInlineActions_t::Vortex(Particle_t& m, const pVec& tip, const pVec& axis, const float tightnessExponent, const float max_radius,
                                             const float inSpeed, const float upSpeed, const float aroundSpeed)
{
    PAVortex_Impl(m, PS->dt, tip, axis, tightnessExponent, max_radius, inSpeed, upSpeed, aroundSpeed);
}

//////////////////////////////////////////////////////////////////
// Inter-particle actions

PINLINE void PContextInlineActions_t::Follow(Particle_t& m, const float magnitude, const float epsilon, const float max_radius)
{
    ParticleGroup& pg = PS->PGroups[PS->pgroup_id];
    const Particle_t* endp = &*pg.begin() + (pg.end() - pg.begin());
    PAFollow_Impl(m, PS->dt, magnitude, epsilon, max_radius, &*pg.begin(), endp);
}

PINLINE void PContextInlineActions_t::Gravitate(Particle_t& m, const float magnitude, const float epsilon, const float max_radius)
{
    ParticleGroup& pg = PS->PGroups[PS->pgroup_id];
    const Particle_t* endp = &*pg.begin() + (pg.end() - pg.begin());
    PAGravitate_Impl(m, PS->dt, magnitude, epsilon, max_radius, &*pg.begin(), endp);
}

PINLINE void PContextInlineActions_t::MatchVelocity(Particle_t& m, const float magnitude, const float epsilon, const float max_radius)
{
    ParticleGroup& pg = PS->PGroups[PS->pgroup_id];
    const Particle_t* endp = &*pg.begin() + (pg.end() - pg.begin());
    PAMatchVelocity_Impl(m, PS->dt, magnitude, epsilon, max_radius, &*pg.begin(), endp);
}

PINLINE void PContextInlineActions_t::MatchRotVelocity(Particle_t& m, const float magnitude, const float epsilon, const float max_radius)
{
    ParticleGroup& pg = PS->PGroups[PS->pgroup_id];
    const Particle_t* endp = &*pg.begin() + (pg.end() - pg.begin());
    PAMatchRotVelocity_Impl(m, PS->dt, magnitude, epsilon, max_radius, &*pg.begin(), endp);
}

//////////////////////////////////////////////////////////////////
// Other exceptional actions

PINLINE void PContextInlineActions_t::Callback(Particle_t& m, P_PARTICLE_CALLBACK_ACTION callbackFunc, const pdata_t call_data)
{
    (*callbackFunc)(m, call_data, PS->dt);
}

PINLINE void PContextInlineActions_t::KillOld(Particle_t& m, const float age_limit, const bool kill_less_than)
{
    PAKillOld_Impl(m, PS->dt, age_limit, kill_less_than);
}

PINLINE void PContextInlineActions_t::Sink(Particle_t& m, const bool kill_inside, const pDomain& kill_pos_dom)
{
    PASink_Impl(m, PS->dt, kill_inside, kill_pos_dom);
}

PINLINE void PContextInlineActions_t::SinkVelocity(Particle_t& m, const bool kill_inside, const pDomain& kill_vel_dom)
{
    PASinkVelocity_Impl(m, PS->dt, kill_inside, kill_vel_dom);
}
}; // namespace PAPI
