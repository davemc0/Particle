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

// TODO: Fix PInternalState.
//#include "PInternalState.h"
#include "Particle/pAPI.h"
#include "Particle/pActionImpls.h"

namespace PAPI {

#ifdef _DEBUG
#define P_CHECK_ERR                                                                       \
    PASSERT(!PSh.get_in_new_list(), "Can't call inline action while making action list"); \
    PASSERT(PS->in_particle_loop, "Can only call inline action while inside ParticleLoop")
#else
#define P_CHECK_ERR
#endif

PINLINE void PContextActions_t::Avoid(Particle_t& m, const float magnitude, const float epsilon, const float look_ahead, const pDomain& dom)
{
    P_CHECK_ERR;
    switch (dom.Which) {
    case PDDisc_e: PAAvoidDisc_Impl(m, PSh.get_dt(), *dynamic_cast<const PDDisc*>(&dom), look_ahead, magnitude, epsilon); return;
    case PDPlane_e: PAAvoidPlane_Impl(m, PSh.get_dt(), *dynamic_cast<const PDPlane*>(&dom), look_ahead, magnitude, epsilon); return;
    case PDRectangle_e: PAAvoidRectangle_Impl(m, PSh.get_dt(), *dynamic_cast<const PDRectangle*>(&dom), look_ahead, magnitude, epsilon); return;
    case PDSphere_e: PAAvoidSphere_Impl(m, PSh.get_dt(), *dynamic_cast<const PDSphere*>(&dom), look_ahead, magnitude, epsilon); return;
    case PDTriangle_e: PAAvoidTriangle_Impl(m, PSh.get_dt(), *dynamic_cast<const PDTriangle*>(&dom), look_ahead, magnitude, epsilon); return;
    default: return;
    }
}

PINLINE void PContextActions_t::Bounce(Particle_t& m, const float friction, const float resilience, const float fric_min_vel, const pDomain& dom)
{
    P_CHECK_ERR;
    switch (dom.Which) {
    case PDBox_e: PABounceBox_Impl(m, PSh.get_dt(), *static_cast<const PDBox*>(&dom), friction, resilience, fric_min_vel); return;
    case PDDisc_e: PABounceDisc_Impl(m, PSh.get_dt(), *static_cast<const PDDisc*>(&dom), friction, resilience, fric_min_vel); return;
    case PDPlane_e: PABouncePlane_Impl(m, PSh.get_dt(), *static_cast<const PDPlane*>(&dom), friction, resilience, fric_min_vel); return;
    case PDRectangle_e: PABounceRectangle_Impl(m, PSh.get_dt(), *static_cast<const PDRectangle*>(&dom), friction, resilience, fric_min_vel); return;
    case PDSphere_e: PABounceSphere_Impl(m, PSh.get_dt(), *static_cast<const PDSphere*>(&dom), friction, resilience, fric_min_vel); return;
    case PDTriangle_e: PABounceTriangle_Impl(m, PSh.get_dt(), *static_cast<const PDTriangle*>(&dom), friction, resilience, fric_min_vel); return;
    default: return;
    }
}

PINLINE void PContextActions_t::CopyVertexB(Particle_t& m, const bool copy_pos, const bool copy_vel)
{
    P_CHECK_ERR;
    PACopyVertexB_Impl(m, PSh.get_dt(), copy_pos, copy_vel);
}

PINLINE void PContextActions_t::Damping(Particle_t& m, const pVec& damping, const float min_vel, const float max_vel)
{
    P_CHECK_ERR;
    PADamping_Impl(m, PSh.get_dt(), damping, min_vel, max_vel);
}

PINLINE void PContextActions_t::RotDamping(Particle_t& m, const pVec& damping, const float min_vel, const float max_vel)
{
    P_CHECK_ERR;
    PARotDamping_Impl(m, PSh.get_dt(), damping, min_vel, max_vel);
}

PINLINE void PContextActions_t::Explosion(Particle_t& m, const pVec& center, const float radius, const float magnitude, const float stdev, const float epsilon)
{
    P_CHECK_ERR;
    PAExplosion_Impl(m, PSh.get_dt(), center, radius, magnitude, stdev, epsilon);
}

PINLINE void PContextActions_t::Gravity(Particle_t& m, const pVec& dir)
{
    P_CHECK_ERR;
    PAGravity_Impl(m, PSh.get_dt(), dir);
}

PINLINE void PContextActions_t::Jet(Particle_t& m, const pDomain& dom, const pDomain& accel)
{
    P_CHECK_ERR;
    PAJet_Impl(m, PSh.get_dt(), dom, accel);
}

PINLINE void PContextActions_t::Move(Particle_t& m, const bool move_velocity, const bool move_rotational_velocity)
{
    P_CHECK_ERR;
    PAMove_Impl(m, PSh.get_dt(), move_velocity, move_rotational_velocity);
}

PINLINE void PContextActions_t::OrbitLine(Particle_t& m, const pVec& p, const pVec& axis, const float magnitude, const float epsilon, const float max_radius)
{
    P_CHECK_ERR;
    PAOrbitLine_Impl(m, PSh.get_dt(), p, axis, magnitude, epsilon, max_radius);
}

PINLINE void PContextActions_t::OrbitPoint(Particle_t& m, const pVec& center, const float magnitude, const float epsilon, const float max_radius)
{
    P_CHECK_ERR;
    PAOrbitPoint_Impl(m, PSh.get_dt(), center, magnitude, epsilon, max_radius);
}

PINLINE void PContextActions_t::RandomAccel(Particle_t& m, const pDomain& gen_acc)
{
    P_CHECK_ERR;
    PARandomAccel_Impl(m, PSh.get_dt(), gen_acc);
}

PINLINE void PContextActions_t::RandomDisplace(Particle_t& m, const pDomain& gen_disp)
{
    P_CHECK_ERR;
    PARandomDisplace_Impl(m, PSh.get_dt(), gen_disp);
}

PINLINE void PContextActions_t::RandomVelocity(Particle_t& m, const pDomain& gen_vel)
{
    P_CHECK_ERR;
    PARandomVelocity_Impl(m, PSh.get_dt(), gen_vel);
}

PINLINE void PContextActions_t::RandomRotVelocity(Particle_t& m, const pDomain& gen_vel)
{
    P_CHECK_ERR;
    PARandomRotVelocity_Impl(m, PSh.get_dt(), gen_vel);
}

PINLINE void PContextActions_t::Restore(Particle_t& m, const float time_left, const bool vel, const bool rvel)
{
    P_CHECK_ERR;
    PARestore_Impl(m, PSh.get_dt(), time_left, vel, rvel);
}

PINLINE void PContextActions_t::SpeedClamp(Particle_t& m, const float min_speed, const float max_speed)
{
    P_CHECK_ERR;
    PASpeedClamp_Impl(m, PSh.get_dt(), min_speed, max_speed);
}

PINLINE void PContextActions_t::TargetColor(Particle_t& m, const pVec& color, const float alpha, const float scale)
{
    P_CHECK_ERR;
    PATargetColor_Impl(m, PSh.get_dt(), color, alpha, scale);
}

PINLINE void PContextActions_t::TargetSize(Particle_t& m, const pVec& size, const pVec& scale)
{
    P_CHECK_ERR;
    PATargetSize_Impl(m, PSh.get_dt(), size, scale);
}

PINLINE void PContextActions_t::TargetVelocity(Particle_t& m, const pVec& vel, const float scale)
{
    P_CHECK_ERR;
    PATargetVelocity_Impl(m, PSh.get_dt(), vel, scale);
}

PINLINE void PContextActions_t::TargetRotVelocity(Particle_t& m, const pVec& rot_velocity, const float scale)
{
    P_CHECK_ERR;
    PATargetRotVelocity_Impl(m, PSh.get_dt(), rot_velocity, scale);
}

PINLINE void PContextActions_t::Vortex(Particle_t& m, const pVec& tip, const pVec& axis, const float tightnessExponent, const float max_radius,
                                       const float inSpeed, const float upSpeed, const float aroundSpeed)
{
    P_CHECK_ERR;
    PAVortex_Impl(m, PSh.get_dt(), tip, axis, tightnessExponent, max_radius, inSpeed, upSpeed, aroundSpeed);
}

//////////////////////////////////////////////////////////////////
// Inter-particle actions

PINLINE void PContextActions_t::Follow(Particle_t& m, const float magnitude, const float epsilon, const float max_radius)
{
    P_CHECK_ERR;
    PAFollow_Impl(m, PSh.get_dt(), magnitude, epsilon, max_radius, PSh.get_const_pgroup_begin(), PSh.get_const_pgroup_end());
}

PINLINE void PContextActions_t::Gravitate(Particle_t& m, const float magnitude, const float epsilon, const float max_radius)
{
    P_CHECK_ERR;
    PAGravitate_Impl(m, PSh.get_dt(), magnitude, epsilon, max_radius, PSh.get_const_pgroup_begin(), PSh.get_const_pgroup_end());
}

PINLINE void PContextActions_t::MatchVelocity(Particle_t& m, const float magnitude, const float epsilon, const float max_radius)
{
    P_CHECK_ERR;
    PAMatchVelocity_Impl(m, PSh.get_dt(), magnitude, epsilon, max_radius, PSh.get_const_pgroup_begin(), PSh.get_const_pgroup_end());
}

PINLINE void PContextActions_t::MatchRotVelocity(Particle_t& m, const float magnitude, const float epsilon, const float max_radius)
{
    P_CHECK_ERR;
    PAMatchRotVelocity_Impl(m, PSh.get_dt(), magnitude, epsilon, max_radius, PSh.get_const_pgroup_begin(), PSh.get_const_pgroup_end());
}

//////////////////////////////////////////////////////////////////
// Other exceptional actions

PINLINE void PContextActions_t::Callback(Particle_t& m, P_PARTICLE_CALLBACK_ACTION callbackFunc, const pdata_t call_data)
{
    P_CHECK_ERR;
    (*callbackFunc)(m, call_data, PSh.get_dt());
}

PINLINE void PContextActions_t::KillOld(Particle_t& m, const float age_limit, const bool kill_less_than)
{
    P_CHECK_ERR;
    PAKillOld_Impl(m, PSh.get_dt(), age_limit, kill_less_than);
}

PINLINE void PContextActions_t::Sink(Particle_t& m, const bool kill_inside, const pDomain& kill_pos_dom)
{
    P_CHECK_ERR;
    PASink_Impl(m, PSh.get_dt(), kill_inside, kill_pos_dom);
}

PINLINE void PContextActions_t::SinkVelocity(Particle_t& m, const bool kill_inside, const pDomain& kill_vel_dom)
{
    P_CHECK_ERR;
    PASinkVelocity_Impl(m, PSh.get_dt(), kill_inside, kill_vel_dom);
}

#undef P_CHECK_ERR
}; // namespace PAPI
