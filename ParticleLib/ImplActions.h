/// ImplActions.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// This file contains inline implementations of the actions as applied to a single particle.
/// It is included into Actions.cpp for the standard for-loop based API.
/// The hope is that it can be used to make a per-particle API that can enable all the code to operate on a given particle to be inlined.
/// Once it's inlined the C++ application code could have its own for loop wrapped around this.
/// This approach should provide good optimization in several ways:
/// 1) optimized computation through inlining
/// 2) much better memory access since each particle would only be visited once, and
/// 3) Multi-core and AVX optimization through std::for_each(std::execution::par_unseq, ...).
///
/// This approach should also enable CUDA kernels to easily express and compute one particle per thread.

// Classes of nonstraightforward cases:
// Actions that kill particles
// Actions that use domains (maybe not difficult)
// Actions that refer to other particles
// Actions that call out

#ifndef implactions_h
#define implactions_h

#include "Particle.h"
#include "Particle/pSourceState.h"

using namespace PAPI;

PINLINE void PAAvoidTriangle_Impl(Particle_t& m, const float dt, const PDTriangle& dom, const float look_ahead, const float magnitude, const float epsilon)
{
    float magdt = magnitude * dt;
    // ^^^ Above values do not vary per particle.

    const pVec& u = dom.u;
    const pVec& v = dom.v;

    // F is the third (non-basis) triangle edge.
    pVec f = v - u;
    pVec fn = f;
    fn.normalize();
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * look_ahead;

    // Nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if (pSameSign(distold, distnew)) return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv; // Time until hit

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p;    // Offset from origin in plane

    // Dot product with basis vectors of old frame
    // in terms of new frame gives position in uv frame.
    float upos = dot(offset, dom.s1);
    float vpos = dot(offset, dom.s2);

    // Did it cross plane outside triangle?
    if (upos <= 0 || vpos <= 0 || (upos + vpos) >= 1) return;

    // A hit, a very palpable hit. Compute distance to the three edges
    pVec uofs = (dom.uNrm * dot(dom.uNrm, offset)) - offset;
    float udistSqr = uofs.lenSqr();
    pVec vofs = (dom.vNrm * dot(dom.vNrm, offset)) - offset;
    float vdistSqr = vofs.lenSqr();

    pVec foffset = offset - u;
    pVec fofs = (fn * dot(fn, foffset)) - foffset;
    float fdistSqr = fofs.lenSqr();

    // S is the safety vector toward the closest point on boundary.
    pVec S;
    if (udistSqr <= vdistSqr && udistSqr <= fdistSqr)
        S = uofs;
    else if (vdistSqr <= fdistSqr)
        S = vofs;
    else
        S = fofs;

    if (S == pVec(0.f)) return; // It's aimed straight at an edge. S will become NaN.
    S.normalize();              // Blend S with m.vel.

    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec dir = (S * (magdt / (fsqr(t) + epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

PINLINE void PAAvoidRectangle_Impl(Particle_t& m, const float dt, const PDRectangle& dom, const float look_ahead, const float magnitude, const float epsilon)
{
    float magdt = magnitude * dt;
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * look_ahead;

    // Nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if (pSameSign(distold, distnew)) return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv;

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p;    // Offset from origin in plane

    // Dot product with basis vectors of old frame
    // in terms of new frame gives position in uv frame.
    float upos = dot(offset, dom.s1);
    float vpos = dot(offset, dom.s2);

    // Did it cross plane outside rectangle?
    if (upos <= 0 || vpos <= 0 || upos >= 1 || vpos >= 1) return;

    // A hit, a very palpable hit. Compute distance to the four edges
    pVec uofs = (dom.uNrm * dot(dom.uNrm, offset)) - offset;
    float udistSqr = uofs.lenSqr();
    pVec vofs = (dom.vNrm * dot(dom.vNrm, offset)) - offset;
    float vdistSqr = vofs.lenSqr();

    pVec foffset = (dom.u + dom.v) - offset;
    pVec fofs = foffset - (dom.uNrm * dot(dom.uNrm, foffset));
    float fdistSqr = fofs.lenSqr();
    pVec gofs = foffset - (dom.vNrm * dot(dom.vNrm, foffset));
    float gdistSqr = gofs.lenSqr();

    pVec S; // Vector from point of impact to safety
    if (udistSqr <= vdistSqr && udistSqr <= fdistSqr && udistSqr <= gdistSqr)
        S = uofs;
    else if (vdistSqr <= fdistSqr && vdistSqr <= gdistSqr)
        S = vofs;
    else if (fdistSqr <= gdistSqr)
        S = fofs;
    else
        S = gofs;

    if (S == pVec(0.f)) return; // It's aimed straight at an edge. S will become NaN.
    S.normalize();              // Blend S with m.vel.

    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec dir = (S * (magdt / (fsqr(t) + epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

PINLINE void PAAvoidPlane_Impl(Particle_t& m, const float dt, const PDPlane& dom, const float look_ahead, const float magnitude, const float epsilon)
{
    float magdt = magnitude * dt;
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * look_ahead;

    // Nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if (pSameSign(distold, distnew)) return;

    // A hit, a very palpable hit.
    float t = -distold / dot(dom.nrm, m.vel); // Time to collision
    pVec S = m.vel * t + dom.nrm * distold;   // Vector from projection point to point of impact

    if (S == pVec(0.f))
        S = dom.nrm;
    else
        S.normalize();

    // Blend S with m.vel.
    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec dir = (S * (magdt / (fsqr(t) + epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

// TODO: Only works for points on the OUTSIDE of the sphere. Ignores inner radius.
PINLINE void PAAvoidSphere_Impl(Particle_t& m, const float dt, const PDSphere& dom, const float look_ahead, const float magnitude, const float epsilon)
{
    float magdt = magnitude * dt;
    // ^^^ Above values do not vary per particle.

    // First do a ray-sphere intersection test and see if it's soon enough.
    // Can I do this faster without t?
    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec L = dom.ctr - m.pos;
    float v = dot(L, Vn);

    float dscr = dom.radOutSqr - dot(L, L) + fsqr(v);
    if (dscr < 0) return; // A hit, a very palpable hit.

    // Compute length for second rejection test.
    float t = v - sqrtf(dscr);
    if (t < 0 || t > (vlen * look_ahead)) return;

    // Get a vector to safety.
    pVec C = Cross(Vn, L);
    C.normalize();
    pVec S = Cross(Vn, C);

    pVec dir = (S * (magdt / (fsqr(t) + epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

PINLINE void PAAvoidDisc_Impl(Particle_t& m, const float dt, const PDDisc& dom, const float look_ahead, const float magnitude, const float epsilon)
{
    float magdt = magnitude * dt;
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * look_ahead;

    // Nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if (pSameSign(distold, distnew)) return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv;

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p;    // Offset from origin in plane

    float radSqr = offset.lenSqr();

    // Are we going to hit the disc ring? If so, always turn to the OUTSIDE of the ring.
    // Could do inside of ring, too, if we took sqrts, found the closer direction, and flipped offset if needed.
    if (radSqr < dom.radInSqr || radSqr > dom.radOutSqr) return;

    // Blend S with m.vel.
    pVec S = offset;
    S /= sqrtf(radSqr);

    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec dir = (S * (magdt / (fsqr(t) + epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

// Bounce() doesn't work correctly with small time step sizes for particles sliding along a surface.
// The friction and resilience parameters should not be scaled by dt, since a bounce happens instantaneously.
// On the other hand, they should be scaled by dt because particles sliding along a surface will hit more
// often if dt is smaller. If you have any suggestions, let me know.
//
// This approach uses the actual hit location and hit time to determine whether we actually hit.
// But it doesn't bounce from the actual location or time. It reverses the velocity immediately, applying
// the whole velocity in the outward direction for the whole time step.
PINLINE void PABounceTriangle_Impl(Particle_t& m, const float dt, const PDTriangle& dom, const float friction, const float resilience, const float fric_min_vel)
{
    float oneMinusFriction = 1.f - friction;
    float FricMinTanVelSqr = fsqr(fric_min_vel);
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * dt;

    // Nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if (pSameSign(distold, distnew)) return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv; // Time until hit

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p;    // Offset from origin in plane

    // Dot product with basis vectors of old frame
    // in terms of new frame gives position in uv frame.
    float upos = dot(offset, dom.s1);
    float vpos = dot(offset, dom.s2);

    // Did it cross plane outside triangle?
    if (upos < 0 || vpos < 0 || (upos + vpos) > 1) return;

    // A hit, a very palpable hit. Compute tangential and normal components of velocity
    pVec vn = dom.nrm * nv; // Normal Vn = (V.N)N
    pVec vt = m.vel - vn;   // Tangent Vt = V - Vn

    // Compute new velocity, applying resilience and, unless tangential velocity < fric_min_vel, friction
    float fric = (vt.lenSqr() <= FricMinTanVelSqr) ? 1.f : oneMinusFriction;
    m.vel = vt * fric - vn * resilience;
}

PINLINE void PABounceRectangle_Impl(Particle_t& m, const float dt, const PDRectangle& dom, const float friction, const float resilience, const float fric_min_vel)
{
    float oneMinusFriction = 1.f - friction;
    float FricMinTanVelSqr = fsqr(fric_min_vel);
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * dt;

    // Nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if (pSameSign(distold, distnew)) return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv; // Time until hit

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p;    // Offset from origin in plane

    // Dot product with basis vectors of old frame
    // in terms of new frame gives position in uv frame.
    float upos = dot(offset, dom.s1);
    float vpos = dot(offset, dom.s2);

    // Did it cross plane outside rectangle?
    if (upos < 0 || upos > 1 || vpos < 0 || vpos > 1) return;

    // A hit, a very palpable hit. Compute tangential and normal components of velocity
    pVec vn = dom.nrm * nv; // Normal Vn = (V.N)N
    pVec vt = m.vel - vn;   // Tangent Vt = V - Vn

    // Compute new velocity, applying resilience and, unless tangential velocity < fric_min_vel, friction
    float fric = (vt.lenSqr() <= FricMinTanVelSqr) ? 1.f : oneMinusFriction;
    m.vel = vt * fric - vn * resilience;
}

PINLINE void PABounceBox_Impl(Particle_t& m, const float dt, const PDBox& dom, const float friction, const float resilience, const float fric_min_vel)
{
    float oneMinusFriction = 1.f - friction;
    float FricMinTanVelSqr = fsqr(fric_min_vel);
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * dt;

    bool oldIn = dom.Within(m.pos);
    bool newIn = dom.Within(pnext);
    if (oldIn == newIn) return;

    pVec vn(0.f), vt(m.vel);
    if (oldIn) { // Bounce off the inside
        // Does it handle bouncing off two walls on the same time step?
        if (pnext.x() < dom.p0.x() || pnext.x() > dom.p1.x()) { std::swap(vn.x(), vt.x()); }
        if (pnext.y() < dom.p0.y() || pnext.y() > dom.p1.y()) { std::swap(vn.y(), vt.y()); }
        if (pnext.z() < dom.p0.z() || pnext.z() > dom.p1.z()) { std::swap(vn.z(), vt.z()); }
    } else { // Bounce off the outside
        if (pnext.x() > dom.p0.x() || pnext.x() < dom.p1.x()) { std::swap(vn.x(), vt.x()); }
        if (pnext.y() > dom.p0.y() || pnext.y() < dom.p1.y()) { std::swap(vn.y(), vt.y()); }
        if (pnext.z() > dom.p0.z() || pnext.z() < dom.p1.z()) { std::swap(vn.z(), vt.z()); }
    }

    // Compute new velocity, applying resilience and, unless tangential velocity < fric_min_vel, friction
    float fric = (vt.lenSqr() <= FricMinTanVelSqr) ? 1.f : oneMinusFriction;
    m.vel = vt * fric - vn * resilience;
}

PINLINE void PABouncePlane_Impl(Particle_t& m, const float dt, const PDPlane& dom, const float friction, const float resilience, const float fric_min_vel)
{
    float oneMinusFriction = 1.f - friction;
    float FricMinTanVelSqr = fsqr(fric_min_vel);
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * dt;

    // Nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if (pSameSign(distold, distnew)) return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv; // Time until hit

    // A hit, a very palpable hit. Compute tangential and normal components of velocity
    pVec vn = dom.nrm * nv; // Normal Vn = (V.N)N
    pVec vt = m.vel - vn;   // Tangent Vt = V - Vn

    // Compute new velocity, applying resilience and, unless tangential velocity < fric_min_vel, friction
    float fric = (vt.lenSqr() <= FricMinTanVelSqr) ? 1.f : oneMinusFriction;
    m.vel = vt * fric - vn * resilience;
}

PINLINE void PABounceSphere_Impl(Particle_t& m, const float dt, const PDSphere& dom, const float friction, const float resilience, const float fric_min_vel)
{
    float oneMinusFriction = 1.f - friction;
    float FricMinTanVelSqr = fsqr(fric_min_vel);
    float dtinv = 1.0f / dt;
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * dt;

    if (dom.Within(m.pos)) {           // We are bouncing off the inside of the sphere.
        if (dom.Within(pnext)) return; // Still inside. Do nothing.

        // Trying to go outside. Bounce back in.

        // Inward-pointing normal to surface. This isn't computed quite right;
        // should extrapolate particle position to surface.
        pVec n = dom.ctr - m.pos;
        n.normalize();

        // Compute tangential and normal components of velocity
        float nmag = dot(m.vel, n);

        pVec vn = n * nmag;   // Velocity in Normal dir Vn = (V.N)N
        pVec vt = m.vel - vn; // Velocity in Tangent dir Vt = V - Vn

        // Reverse normal component of velocity
        if (nmag < 0) vn = -vn; // Don't reverse if it's already heading inward

        // Compute new velocity, applying resilience and, unless tangential velocity < fric_min_vel, friction
        float fric = (vt.lenSqr() <= FricMinTanVelSqr) ? 1.f : oneMinusFriction;
        m.vel = vt * fric + vn * resilience;

        // Now see where the point will end up. Make sure we fixed it to stay inside.
        pVec pthree = m.pos + m.vel * dt;
        if (dom.Within(pthree)) {
            return; // Still inside. We're good.
        } else {
            // Since the tangent plane is outside the sphere, reflecting the velocity vector about it won't necessarily bring it inside the sphere.
            pVec toctr = dom.ctr - pthree;
            float dist = toctr.length();
            pVec pwish = dom.ctr - toctr * (0.999f * dom.radOut / dist); // Pwish is a point just inside the sphere.
            m.vel = (pwish - m.pos) * dtinv;                             // Compute a velocity to get us to pwish on this timestep
        }
    } else { // We are bouncing off the outside of the sphere.
        if (!dom.Within(pnext)) return;

        // Trying to go inside. Bounce back out.

        // Outward-pointing normal to surface. This isn't computed quite right;
        // should extrapolate particle position to surface using ray-sphere intersection
        pVec n = m.pos - dom.ctr;
        n.normalize();

        float NdotV = dot(n, m.vel);

        // A hit, a very palpable hit. Compute tangential and normal components of velocity
        pVec vn = n * NdotV;  // Normal Vn = (V.N)N
        pVec vt = m.vel - vn; // Tangent Vt = V - Vn

        // Compute new velocity, applying resilience and, unless tangential velocity < fric_min_vel, friction
        float fric = (vt.lenSqr() <= FricMinTanVelSqr) ? 1.f : oneMinusFriction;
        m.vel = vt * fric - vn * resilience;
    }
}

PINLINE void PABounceDisc_Impl(Particle_t& m, const float dt, const PDDisc& dom, const float friction, const float resilience, const float fric_min_vel)
{
    float oneMinusFriction = 1.f - friction;
    float FricMinTanVelSqr = fsqr(fric_min_vel);
    // ^^^ Above values do not vary per particle.

    // See if particle's current and pnext positions cross boundary. If not, skip it.
    pVec pnext = m.pos + m.vel * dt;

    // Nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if (pSameSign(distold, distnew)) return;

    float NdotV = dot(dom.nrm, m.vel);
    float t = -distold / NdotV; // Time until hit

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p;    // Offset from origin in plane

    float radSqr = offset.lenSqr();

    // Are we going to hit the disc ring?
    if (radSqr < dom.radInSqr || radSqr > dom.radOutSqr) return;

    // A hit, a very palpable hit. Compute tangential and normal components of velocity
    pVec vn = dom.nrm * NdotV; // Normal Vn = (V.N)N
    pVec vt = m.vel - vn;      // Tangent Vt = V - Vn

    // Compute new velocity, applying resilience and, unless tangential velocity < fric_min_vel, friction
    float fric = (vt.lenSqr() <= FricMinTanVelSqr) ? 1.f : oneMinusFriction;
    m.vel = vt * fric - vn * resilience;
}

// Set the secondary position and velocity from current
PINLINE void PACopyVertexB_Impl(Particle_t& m, const float dt, const bool copy_pos, const bool copy_vel)
{
    if (copy_pos && copy_vel) {
        m.posB = m.pos;
        m.upB = m.up;
        m.velB = m.vel;
    } else if (copy_pos) {
        m.posB = m.pos;
        m.upB = m.up;
    } else if (copy_vel) {
        m.velB = m.vel;
    }
}

// Dampen velocities
PINLINE void PADamping_Impl(Particle_t& m, const float dt, const pVec damping, const float vlow, const float vhigh)
{
    pVec scale(pVec(1.f) - ((pVec(1.f) - damping) * dt)); // This is important if dt is != 1.
    float vlowSqr = fsqr(vlow);
    float vhighSqr = fsqr(vhigh);
    // ^^^ Above values do not vary per particle.

    float vSqr = m.vel.lenSqr();

    if (vSqr >= vlowSqr && vSqr <= vhighSqr) { m.vel = CompMult(m.vel, scale); }
}

// Dampen rotational velocities
PINLINE void PARotDamping_Impl(Particle_t& m, const float dt, const pVec damping, const float vlow, const float vhigh)
{
    pVec scale(pVec(1.f) - ((pVec(1.f) - damping) * dt)); // This is important if dt is != 1.
    float vlowSqr = fsqr(vlow);
    float vhighSqr = fsqr(vhigh);
    // ^^^ Above values do not vary per particle.

    float vSqr = m.rvel.lenSqr();

    if (vSqr >= vlowSqr && vSqr <= vhighSqr) { m.rvel = CompMult(m.rvel, scale); }
}

// Exert force on each particle away from explosion center
PINLINE void PAExplosion_Impl(Particle_t& m, const float dt, const pVec center, const float radius, const float magnitude, const float stdev, const float epsilon)
{
    float magdt = magnitude * dt;
    float oneOverSigma = 1.0f / stdev;
    float inexp = -0.5f * fsqr(oneOverSigma);
    float outexp = P_ONEOVERSQRT2PI * oneOverSigma;
    // ^^^ Above values do not vary per particle.

    // Figure direction to particle.
    pVec dir(m.pos - center);
    float distSqr = dir.lenSqr();
    float dist = sqrtf(distSqr);
    float DistFromWaveSqr = fsqr(radius - dist);

    float Gd = exp(DistFromWaveSqr * inexp) * outexp;
    pVec acc(dir * (Gd * magdt / (dist * (distSqr + epsilon))));

    m.vel += acc;
}

// Follow the next particle in the list
PINLINE void PAFollow_Impl(Particle_t& m, const float dt, const float magnitude, const float epsilon, const float max_radius)
{
#if 0
 // XXX Need to do something special in the caller to get the other particle.

 // Accelerate toward the particle after me in the list.
 pVec toHim((*next).pos - m.pos); // toHim = p1 - p0
 float toHimlenSqr = toHim.lenSqr();

 if (toHimlenSqr < max_radiusSqr) {
 // Compute force exerted between the two bodies
 m.vel += toHim * (magdt / (sqrtf(toHimlenSqr) * (toHimlenSqr + epsilon)));
 }
#endif
}

// Inter-particle gravitation
PINLINE void PAGravitate_Impl(Particle_t& m, const float dt, const float magnitude, const float epsilon, const float max_radius)
{
#if 0
 // XXX Need access to all the particles.
 float magdt = magnitude * dt;
 float max_radiusSqr = fsqr(max_radius);

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
#endif
}

// Acceleration in a constant direction
PINLINE void PAGravity_Impl(Particle_t& m, const float dt, const pVec direction)
{
    pVec ddir(direction * dt);

    m.vel += ddir;
}

// For particles in the domain of influence, accelerate them with a domain.
PINLINE void PAJet_Impl(Particle_t& m, const float dt, const std::shared_ptr<pDomain> dom, const std::shared_ptr<pDomain> acc)
{
    if (dom->Within(m.pos)) {
        pVec accel = acc->Generate();

        m.vel += accel * dt;
    }
}

// Get rid of older particles
PINLINE void PAKillOld_Impl(Particle_t& m, const float dt, const float age_limit, const bool kill_less_than)
{
    if (!((m.age < age_limit) ^ kill_less_than)) { m.tmp0 = 1.0f; }
}

// Match velocity to near neighbors
PINLINE void PAMatchVelocity_Impl(Particle_t& m, const float dt, const float magnitude, const float epsilon, const float max_radius)
{
#if 0
 // XXX Need to handle N squared here.
 float magdt = magnitude * dt;
 float max_radiusSqr = fsqr(max_radius);

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
#endif
}

// Match Rotational velocity to near neighbors
PINLINE void PAMatchRotVelocity_Impl(Particle_t& m, const float dt, const float magnitude, const float epsilon, const float max_radius)
{
#if 0
 // XXX Need to handle N squared here.
 float magdt = magnitude * dt;
 float max_radiusSqr = fsqr(max_radius);

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
#endif
}

// Apply the particles' velocities to their positions, and age the particles
PINLINE void PAMove_Impl(Particle_t& m, const float dt, const bool move_velocity, const bool move_rotational_velocity)
{
    m.age += dt;
    if (move_rotational_velocity) { m.up += m.rvel * dt; }
    if (move_velocity) { m.pos += m.vel * dt; }
}

// Accelerate particles towards a line
PINLINE void PAOrbitLine_Impl(Particle_t& m, const float dt, const pVec p, const pVec axis, const float magnitude, const float epsilon, const float max_radius)
{
    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);
    pVec axisNrm(axis);
    axisNrm.normalize(); // Do we need this? Should we make it user responsibilty?
    // ^^^ Above values do not vary per particle.

    // Figure direction to particle from base of line.
    pVec f = m.pos - p;

    // Projection of particle onto line
    pVec w = axisNrm * dot(f, axisNrm);

    // Direction from particle to nearest point on line.
    pVec into = w - f;

    // Distance to line (force drops as 1/r^2, normalize by 1/r)
    // Soften by epsilon to avoid tight encounters to infinity
    float rSqr = into.lenSqr();

    if (rSqr < max_radiusSqr) m.vel += into * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
}

// Accelerate particles towards a point
PINLINE void PAOrbitPoint_Impl(Particle_t& m, const float dt, const pVec center, const float magnitude, const float epsilon, const float max_radius)
{
    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);
    // ^^^ Above values do not vary per particle.

    // Figure direction from particle to center
    pVec dir(center - m.pos);

    // Distance to gravity well (force drops as 1/r^2, normalize by 1/r)
    // Soften by epsilon to avoid tight encounters to infinity
    float rSqr = dir.lenSqr();

    if (rSqr < max_radiusSqr) m.vel += dir * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
}

// Accelerate in random direction each time step
PINLINE void PARandomAccel_Impl(Particle_t& m, const float dt, const std::shared_ptr<pDomain> gen_acc)
{
    pVec accel = gen_acc->Generate();

    // Dt will affect this by making a higher probability of being near the original velocity after unit time.
    // Smaller dt approach a normal distribution instead of a square wave.
    m.vel += accel * dt;
}

// Immediately displace position randomly
PINLINE void PARandomDisplace_Impl(Particle_t& m, const float dt, const std::shared_ptr<pDomain> gen_disp)
{
    pVec disp = gen_disp->Generate();

    // Dt will affect this by making a higher probability of being near the original position after unit time.
    // Smaller dt approach a normal distribution instead of a square wave.
    m.pos += disp * dt;
}

// Immediately assign a random velocity
PINLINE void PARandomVelocity_Impl(Particle_t& m, const float dt, const std::shared_ptr<pDomain> gen_vel)
{
    pVec velocity = gen_vel->Generate();

    // Don't multiply by dt because velocities are invariant of dt.
    m.vel = velocity;
}

// Immediately assign a random rotational velocity
PINLINE void PARandomRotVelocity_Impl(Particle_t& m, const float dt, const std::shared_ptr<pDomain> gen_vel)

{
    pVec velocity = gen_vel->Generate();

    // Don't multiply by dt because velocities are invariant of dt.
    m.rvel = velocity;
}

// Figure new velocity at next timestep
PINLINE void doRestore(pVec& vel, const pVec& posB, const pVec& pos, const float t, const float dtSqr, const float ttInv6dt, const float tttInv3dtSqr)
{
    pVec b = (vel * -0.6667f * t + posB - pos) * ttInv6dt;
    pVec a = (vel * t - posB - posB + pos + pos) * tttInv3dtSqr;
    vel += a + b;
}

// Over time, restore particles to initial positions
PINLINE void PARestore_Impl(Particle_t& m, const float dt, const float time_left, const bool restore_velocity, const bool restore_rvelocity)
{
    if (time_left <= 0) {
        // Already constrained; keep it there.
        if (restore_velocity) {
            m.pos = m.posB;
            m.vel = pVec(0.0f, 0.0f, 0.0f);
        }
        if (restore_rvelocity) {
            m.up = m.upB;
            m.rvel = pVec(0.0f, 0.0f, 0.0f);
        }
    } else {
        float t = time_left;
        float dtSqr = fsqr(dt);
        float ttInv6dt = dt * 6.0f / fsqr(t);
        float tttInv3dtSqr = dtSqr * 3.0f / (t * t * t);

        if (restore_velocity) doRestore(m.vel, m.posB, m.pos, t, dtSqr, ttInv6dt, tttInv3dtSqr);
        if (restore_rvelocity) doRestore(m.rvel, m.upB, m.up, t, dtSqr, ttInv6dt, tttInv3dtSqr);
    }
}

// Kill particles with positions on wrong side of the specified domain
PINLINE void PASink_Impl(Particle_t& m, const float dt, const bool kill_inside, const std::shared_ptr<pDomain> position)
{
    // Remove if inside/outside flag matches object's flag
    if (!(position->Within(m.pos) ^ kill_inside)) { m.tmp0 = 1.0f; }
}

// Kill particles with velocities on wrong side of the specified domain
PINLINE void PASinkVelocity_Impl(Particle_t& m, const float dt, const bool kill_inside, const std::shared_ptr<pDomain> velocity)
{
    // Remove if inside/outside flag matches object's flag
    if (!(velocity->Within(m.vel) ^ kill_inside)) { m.tmp0 = 1.0f; }
}

// Sort the particles by their projection onto the Look vector
PINLINE void PASort_Impl(Particle_t& m, const float dt, const pVec Eye, const pVec Look, const bool front_to_back, const bool clamp_negative)
{
#if 0
 // XXX Need access to all particles.

 float Scale = front_to_back ? -1.0f : 1.0f;

 // First compute projection of particle onto view vector
 for (ParticleList::iterator it = ibegin; it != iend; it++) {
 Particle_t &m = (*it);
 pVec ToP = m.pos - Eye;
 m.tmp0 = dot(ToP, Look) * Scale;
 if(clamp_negative && m.tmp0 < 0) m.tmp0 = 0.0f;
 }

 std::sort<ParticleList::iterator>(ibegin, iend);
#endif
}

// Return how many particles to add to the group right now
PINLINE size_t SourceQuantity(const float particle_rate, const float dt, const size_t group_size, const size_t group_cap)
{
    size_t rate = size_t(floor(particle_rate * dt));

    // Dither the fractional particle in time.
    if (pRandf() < particle_rate * dt - float(rate)) rate++;

    // Don't emit more than it can hold.
    if (group_size + rate > group_cap) rate = group_cap - group_size;

    return rate;
}

// Create a single particle
PINLINE void PASource_Impl(Particle_t& m, const float dt, const std::shared_ptr<pDomain> position, const pSourceState& SrcSt)
{
    m.pos = position->Generate();
    m.posB = SrcSt.vertexB_tracks_ ? m.pos : SrcSt.VertexB_->Generate();
    m.up = SrcSt.Up_->Generate();
    m.vel = SrcSt.Vel_->Generate();
    m.rvel = SrcSt.RotVel_->Generate();
    m.size = SrcSt.Size_->Generate();
    m.color = SrcSt.Color_->Generate();
    m.alpha = SrcSt.Alpha_->Generate().x();
    m.age = SrcSt.Age_ + pNRandf(SrcSt.AgeSigma_);
    m.mass = SrcSt.Mass_;
    m.tmp0 = 0;
    m.data = SrcSt.Data_;
}

// Clamp particle velocities to the given range
PINLINE void PASpeedClamp_Impl(Particle_t& m, const float dt, const float min_speed, const float max_speed)
{
    float min_sqr = fsqr(min_speed);
    float max_sqr = fsqr(max_speed);
    // ^^^ Above values do not vary per particle.

    float sSqr = m.vel.lenSqr();
    if (sSqr < min_sqr && sSqr) {
        float s = sqrtf(sSqr);
        m.vel *= (min_speed / s);
    } else if (sSqr > max_sqr) {
        float s = sqrtf(sSqr);
        m.vel *= (max_speed / s);
    }
}

// Change color of all particles toward the specified color
PINLINE void PATargetColor_Impl(Particle_t& m, const float dt, const pVec color, const float alpha, const float scale)
{
    float scaleFac = scale * dt;
    // ^^^ Above values do not vary per particle.

    m.color += (color - m.color) * scaleFac;
    m.alpha += (alpha - m.alpha) * scaleFac;
}

// Change sizes of all particles toward the specified size
PINLINE void PATargetSize_Impl(Particle_t& m, const float dt, const pVec size, const pVec scale)
{
    pVec scaleFac = scale * dt;
    // ^^^ Above values do not vary per particle.

    pVec dif = size - m.size;
    m.size += CompMult(dif, scaleFac);
}

// Change velocity of all particles toward the specified velocity
PINLINE void PATargetVelocity_Impl(Particle_t& m, const float dt, const pVec velocity, const float scale)
{
    float scaleFac = scale * dt;
    // ^^^ Above values do not vary per particle.

    m.vel += (velocity - m.vel) * scaleFac;
}

// Change velocity of all particles toward the specified velocity
PINLINE void PATargetRotVelocity_Impl(Particle_t& m, const float dt, const pVec velocity, const float scale)
{
    float scaleFac = scale * dt;
    // ^^^ Above values do not vary per particle.

    m.rvel += (velocity - m.rvel) * scaleFac;
}

// This one just rotates a particle around the axis. Amount is based on radius, magnitude, and mass.
PINLINE void PAVortex_Impl(Particle_t& m, const float dt, const pVec tip, const pVec axis, const float tightnessExponent, const float max_radius,
                           const float inSpeed, const float upSpeed, float aroundSpeed)
{
    float max_radiusSqr = fsqr(max_radius);
    float axisLength = axis.length();
    float axisLengthInv = 1.0f / axisLength;
    pVec axisN = axis;
    axisN.normalize();
    // ^^^ Above values do not vary per particle.

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
}

#endif
