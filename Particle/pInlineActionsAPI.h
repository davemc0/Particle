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

PINLINE void PContextInlineActions_t::Bounce(Particle_t& m, float friction, ///< tangential component of the outgoing velocity vector is scaled by (1 - friction)
                                             const float resilience,        ///< normal component of the outgoing velocity vector is scaled by resilience
                                             const float fric_min_vel,      ///< only apply friction if tangential velocity is greater than fric_min_vel
                                                                            ///< so particles can glide smoothly
                                             const pDomain& dom             ///< bounce off the surface of this domain
)
{
    PABounceDisc_Impl(m, PS->dt, *dynamic_cast<const PDDisc*>(&dom), friction, resilience, fric_min_vel);
}

PINLINE void PContextInlineActions_t::Gravity(Particle_t& m, const pVec& dir)
{
    // ParticleGroup& pg = PS->PGroups[PS->pgroup_id];
    PAGravity_Impl(m, PS->dt, dir);
}

PINLINE void PContextInlineActions_t::Move(Particle_t& m, const bool move_velocity, const bool move_rotational_velocity)
{
    PAMove_Impl(m, PS->dt, move_velocity, move_rotational_velocity);
}
