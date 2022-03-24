// This file contains inline implementations of the actions as applied to a single particle.
// Include this file like a .h file into generated CUDA effect kernels.

#ifndef cudaactions_cuh
#define cudaactions_cuh

#include "pSourceState.h"
#include "Particle.h"

using namespace PAPI;

PINLINE void PAAvoidTriangle_Impl(
                                  Particle_t &m, float dt,
                                  PDTriangle &dom,
                                  float look_ahead,
                                  float magnitude,
                                  float epsilon)
{
    float magdt = magnitude * dt;

    const pVec &u = dom.u;
    const pVec &v = dom.v;

    // f is the third (non-basis) triangle edge.
    pVec f = v - u;
    pVec fn = f;
    fn.normalize();

    // See if particle's current and look_ahead positions cross plane.
    // If not, couldn't hit, so keep going.
    pVec pnext = m.pos + m.vel * look_ahead;

    // nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if(pSameSign(distold, distnew))
        return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv; // Time steps before hit

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p; // Offset from origin in plane

    // Dot product with basis vectors of old frame
    // in terms of new frame gives position in uv frame.
    float upos = dot(offset, dom.s1);
    float vpos = dot(offset, dom.s2);

    // Did it cross plane outside triangle?
    if(upos < 0 || vpos < 0 || (upos + vpos) > 1)
        return;

    // A hit! A most palpable hit!
    // Compute distance to the three edges.
    pVec uofs = (dom.uNrm * dot(dom.uNrm, offset)) - offset;
    float udistSqr = uofs.length2();
    pVec vofs = (dom.vNrm * dot(dom.vNrm, offset)) - offset;
    float vdistSqr = vofs.length2();

    pVec foffset = offset - u;
    pVec fofs = (fn * dot(fn, foffset)) - foffset;
    float fdistSqr = fofs.length2();

    // S is the safety vector toward the closest point on boundary.
    pVec S;
    if(udistSqr <= vdistSqr && udistSqr <= fdistSqr) S = uofs;
    else if(vdistSqr <= fdistSqr) S = vofs;
    else S = fofs;

    // Blend S with m.vel.
    S.normalize();

    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec dir = (S * (magdt / (fsqr(t)+epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

PINLINE void PAAvoidRectangle_Impl(
                                   Particle_t &m, float dt,
                                   PDRectangle &dom,
                                   float look_ahead,
                                   float magnitude,
                                   float epsilon)
{
    float magdt = magnitude * dt;

    // See if particle's current and look_ahead positions cross plane.
    // If not, couldn't hit, so keep going.
    pVec pnext = m.pos + m.vel * look_ahead;

    // nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if(pSameSign(distold, distnew))
        return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv;

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p; // Offset from origin in plane

    // Dot product with basis vectors of old frame
    // in terms of new frame gives position in uv frame.
    float upos = dot(offset, dom.s1);
    float vpos = dot(offset, dom.s2);

    // Did it cross plane outside rectangle?
    if(upos < 0 || vpos < 0 || upos > 1 || vpos > 1)
        return;

    // A hit! A most palpable hit!
    // Compute distance to the four edges.
    pVec uofs = (dom.uNrm * dot(dom.uNrm, offset)) - offset;
    float udistSqr = uofs.length2();
    pVec vofs = (dom.vNrm * dot(dom.vNrm, offset)) - offset;
    float vdistSqr = vofs.length2();

    pVec foffset = (dom.u + dom.v) - offset;
    pVec fofs = foffset - (dom.uNrm * dot(dom.uNrm, foffset));
    float fdistSqr = fofs.length2();
    pVec gofs = foffset - (dom.vNrm * dot(dom.vNrm, foffset));
    float gdistSqr = gofs.length2();

    pVec S; // Vector from point of impact to safety
    if(udistSqr <= vdistSqr && udistSqr <= fdistSqr
        && udistSqr <= gdistSqr) S = uofs;
    else if(vdistSqr <= fdistSqr && vdistSqr <= gdistSqr) S = vofs;
    else if(fdistSqr <= gdistSqr) S = fofs;
    else S = gofs;

    // Blend S with m.vel.
    S.normalize();

    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec dir = (S * (magdt / (fsqr(t)+epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

PINLINE void PAAvoidPlane_Impl(
                               Particle_t &m, float dt,
                               PDPlane &dom,
                               float look_ahead,
                               float magnitude,
                               float epsilon)
{
    float magdt = magnitude * dt;

    // See if particle's current and look_ahead positions cross plane.
    // If not, couldn't hit, so keep going.
    pVec pnext = m.pos + m.vel * look_ahead;

    // nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if(pSameSign(distold, distnew))
        return;

    float t = -distold / dot(dom.nrm, m.vel); // Time to collision
    pVec S = m.vel * t + dom.nrm * distold; // Vector from projection point to point of impact

    float slen = S.length2();
    if(slen == 0.0f)
        S = dom.nrm;
    else
        S.normalize();

    // Blend S with m.vel.
    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec dir = (S * (magdt / (fsqr(t)+epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

// Only works for points on the OUTSIDE of the sphere. Ignores inner radius.
PINLINE void PAAvoidSphere_Impl(
                                Particle_t &m, float dt,
                                PDSphere &dom,
                                float look_ahead,
                                float magnitude,
                                float epsilon)
{
    float magdt = magnitude * dt;

    // First do a ray-sphere intersection test and see if it's soon enough.
    // Can I do this faster without t?
    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec L = dom.ctr - m.pos;
    float v = dot(L, Vn);

    float disc = dom.radOutSqr - dot(L, L) + fsqr(v);
    if(disc < 0)
        return; // I'm not heading toward it.

    // Compute length for second rejection test.
    float t = v - sqrtf(disc);
    if(t < 0 || t > (vlen * look_ahead))
        return;

    // Get a vector to safety.
    pVec C = Cross(Vn, L);
    C.normalize();
    pVec S = Cross(Vn, C);

    pVec dir = (S * (magdt / (fsqr(t)+epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

PINLINE void PAAvoidDisc_Impl(
                              Particle_t &m, float dt,
                              PDDisc &dom,
                              float look_ahead,
                              float magnitude,
                              float epsilon)
{
    float magdt = magnitude * dt;

    // See if particle's current and look_ahead positions cross plane.
    // If not, couldn't hit, so keep going.
    pVec pnext = m.pos + m.vel * look_ahead;

    // nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if(pSameSign(distold, distnew))
        return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv;

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p; // Offset from origin in plane

    float radSqr = offset.length2();

    // Are we going to hit the disc ring? If so, always turn to the OUTSIDE of the ring.
    // Could do inside of ring, too, if we took sqrts, found the closer direction, and flipped offset if needed.
    if(radSqr < dom.radInSqr || radSqr > dom.radOutSqr)
        return;

    // Blend S with m.vel.
    pVec S = offset;
    S /= sqrtf(radSqr);

    float vlen = m.vel.length();
    pVec Vn = m.vel / vlen;

    pVec dir = (S * (magdt / (fsqr(t)+epsilon))) + Vn;
    m.vel = dir * (vlen / dir.length()); // Speed of m.vel, but in direction dir.
}

PINLINE void PAAvoid_Impl(
                          Particle_t &m, float dt,
                          pDomain position,
                          float look_ahead,
                          float magnitude,
                          float epsilon)
{
    switch(position.Which) {
    //case PDUnion_e: PAAvoidUnion_Impl(m, dt, position.PDUnion_V, look_ahead, magnitude, epsilon); break;
    //case PDPoint_e: PAAvoidPoint_Impl(m, dt, position.PDPoint_V, look_ahead, magnitude, epsilon); break;
    //case PDLine_e: PAAvoidLine_Impl(m, dt, position.PDLine_V, look_ahead, magnitude, epsilon); break;
    case PDTriangle_e: PAAvoidTriangle_Impl(m, dt, position.PDTriangle_V, look_ahead, magnitude, epsilon); break;
    case PDRectangle_e: PAAvoidRectangle_Impl(m, dt, position.PDRectangle_V, look_ahead, magnitude, epsilon); break;
    case PDDisc_e: PAAvoidDisc_Impl(m, dt, position.PDDisc_V, look_ahead, magnitude, epsilon); break;
    case PDPlane_e: PAAvoidPlane_Impl(m, dt, position.PDPlane_V, look_ahead, magnitude, epsilon); break;
    //case PDBox_e: PAAvoidBox_Impl(m, dt, position.PDBox_V, look_ahead, magnitude, epsilon); break;
    //case PDCylinder_e: PAAvoidCylinder_Impl(m, dt, position.PDCylinder_V, look_ahead, magnitude, epsilon); break;
    //case PDCone_e: PAAvoidCone_Impl(m, dt, position.PDCone_V, look_ahead, magnitude, epsilon); break;
    case PDSphere_e: PAAvoidSphere_Impl(m, dt, position.PDSphere_V, look_ahead, magnitude, epsilon); break;
    //case PDBlob_e: PAAvoidBlob_Impl(m, dt, position.PDBlob_V, look_ahead, magnitude, epsilon); break;
    default:
#ifndef __CUDACC__
        throw PErrNotImplemented(std::string("Avoid not implemented for domain ") + std::string(typeid(position).name()));
#endif
        break;
    }
}

PINLINE void PABounceTriangle_Impl(
                                   Particle_t &m, float dt,
                                   PDTriangle &dom,
                                   float oneMinusFriction,
                                   float resilience,
                                   float cutoffSqr)
{
    // See if particle's current and look_ahead positions cross plane.
    // If not, couldn't hit, so keep going.
    pVec pnext = m.pos + m.vel * dt;

    // nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if(pSameSign(distold, distnew))
        return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv; // Time steps before hit

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p; // Offset from origin in plane

    // Dot product with basis vectors of old frame
    // in terms of new frame gives position in uv frame.
    float upos = dot(offset, dom.s1);
    float vpos = dot(offset, dom.s2);

    // Did it cross plane outside triangle?
    if(upos < 0 || vpos < 0 || (upos + vpos) > 1)
        return;

    // A hit! A most palpable hit!
    // Compute tangential and normal components of velocity
    pVec vn = dom.nrm * nv; // Normal Vn = (V.N)N
    pVec vt = m.vel - vn;   // Tangent Vt = V - Vn

    // Compute new velocity heading out:
    // Don't apply friction if tangential velocity < cutoff
    if(vt.length2() <= cutoffSqr)
        m.vel = vt - vn * resilience;
    else
        m.vel = vt * oneMinusFriction - vn * resilience;
}

PINLINE void PABounceRectangle_Impl(
                                    Particle_t &m, float dt,
                                    PDRectangle &dom,
                                    float oneMinusFriction,
                                    float resilience,
                                    float cutoffSqr)
{
    // See if particle's current and pnext positions cross plane.
    // If not, couldn't hit, so keep going.
    pVec pnext = m.pos + m.vel * dt;

    // nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if(pSameSign(distold, distnew))
        return;

    float nv = dot(dom.nrm, m.vel);
    float t = -distold / nv; // Time steps before hit

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p; // Offset from origin in plane

    // Dot product with basis vectors of old frame
    // in terms of new frame gives position in uv frame.
    float upos = dot(offset, dom.s1);
    float vpos = dot(offset, dom.s2);

    // Did it cross plane outside rectangle?
    if(upos < 0 || upos > 1 || vpos < 0 || vpos > 1)
        return;

    // A hit! A most palpable hit!
    // Compute tangential and normal components of velocity
    pVec vn = dom.nrm * nv; // Normal Vn = (V.N)N
    pVec vt = m.vel - vn;   // Tangent Vt = V - Vn

    // Compute new velocity heading out:
    // Don't apply friction if tangential velocity < cutoff
    if(vt.length2() <= cutoffSqr)
        m.vel = vt - vn * resilience;
    else
        m.vel = vt * oneMinusFriction - vn * resilience;
}

PINLINE void PABouncePlane_Impl(
                                Particle_t &m, float dt,
                                PDPlane &dom,
                                float oneMinusFriction,
                                float resilience,
                                float cutoffSqr)
{
    // See if particle's current and look_ahead positions cross plane.
    // If not, couldn't hit, so keep going.
    pVec pnext = m.pos + m.vel * dt;

    // nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D;
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if(pSameSign(distold, distnew))
        return;

    float nv = dot(dom.nrm, m.vel);
    // float t = -distold / nv; // Time steps before hit

    // A hit! A most palpable hit!
    // Compute tangential and normal components of velocity
    pVec vn = dom.nrm * nv; // Normal Vn = (V.N)N
    pVec vt = m.vel - vn;   // Tangent Vt = V - Vn

    // Compute new velocity heading out:
    // Don't apply friction if tangential velocity < cutoff
    if(vt.length2() <= cutoffSqr)
        m.vel = vt - vn * resilience;
    else
        m.vel = vt * oneMinusFriction - vn * resilience;
}

PINLINE void PABounceSphere_Impl(
                                 Particle_t &m, float dt,
                                 PDSphere &dom,
                                 float oneMinusFriction,
                                 float resilience,
                                 float cutoffSqr)
{
    float dtinv = 1.0f / dt;

    // Bounce particles off the inside or outside of the sphere

    // See if particle's next position is on the opposite side of the domain. If so, bounce it.
    pVec pnext = m.pos + m.vel * dt;

    if(dom.Within(m.pos)) {
        // We are bouncing off the inside of the sphere.
        if(dom.Within(pnext))
            // Still inside. Do nothing.
            return;

        // Trying to go outside. Bounce back in.

        // Inward-pointing normal to surface. This isn't computed quite right;
        // should extrapolate particle position to surface.
        pVec n = dom.ctr - m.pos;
        n.normalize();

        // Compute tangential and normal components of velocity
        float nmag = dot(m.vel, n);

        pVec vn = n * nmag;   // Velocity in Normal dir  Vn = (V.N)N
        pVec vt = m.vel - vn; // Velocity in Tangent dir Vt = V - Vn

        // Reverse normal component of velocity
        if(nmag < 0) vn = -vn; // Don't reverse if it's already heading inward

        // Compute new velocity heading out:
        // Don't apply friction if tangential velocity < cutoff
        float tanscale = (vt.length2() <= cutoffSqr) ? 1.0f : oneMinusFriction;
        m.vel = vt * tanscale + vn * resilience;

        // Now see where the point will end up. Make sure we fixed it to stay inside.
        pVec pthree = m.pos + m.vel * dt;
        if(dom.Within(pthree)) {
            // Still inside. We're good.
            return;
        } else {
            // Since the tangent plane is outside the sphere, reflecting the velocity vector about it won't necessarily bring it inside the sphere.
            pVec toctr = dom.ctr - pthree;
            float dist = toctr.length();
            pVec pwish = dom.ctr - toctr * (0.999f * dom.radOut / dist); // pwish is a point just inside the sphere
            m.vel = (pwish - m.pos) * dtinv; // Compute a velocity to get us to pwish.
        }
    } else {
        // We are bouncing off the outside of the sphere.
        if(!dom.Within(pnext))
            return;

        // Trying to go inside. Bounce back out.

        // Outward-pointing normal to surface. This isn't computed quite right;
        // should extrapolate particle position to surface.
        pVec n = m.pos - dom.ctr;
        n.normalize();

        // Compute tangential and normal components of velocity
        float nmag = dot(m.vel, n);

        pVec vn = n * nmag;   // Velocity in Normal dir  Vn = (V.N)N
        pVec vt = m.vel - vn; // Velocity in Tangent dir Vt = V - Vn

        // Reverse normal component of velocity if it points in
        if(nmag < 0)
            vn = -vn;

        // Compute new velocity heading out:
        // Don't apply friction if tangential velocity < cutoff
        float tanscale = (vt.length2() <= cutoffSqr) ? 1.0f : oneMinusFriction;
        m.vel = vt * tanscale + vn * resilience;
    }
}

PINLINE void PABounceDisc_Impl(
                               Particle_t &m, float dt,
                               PDDisc &dom,
                               float oneMinusFriction,
                               float resilience,
                               float cutoffSqr)
{
    // See if particle's current and look_ahead positions cross plane.
    // If not, couldn't hit, so keep going.
    pVec pnext = m.pos + m.vel * dt;

    // nrm stores the plane normal (the a,b,c of the plane eqn).
    // Old and new distances: dist(p,plane) = n * p + d
    float distold = dot(m.pos, dom.nrm) + dom.D; // XXX May be able to speed this up by removing the add and simplifying the dot product.
    float distnew = dot(pnext, dom.nrm) + dom.D;

    if(pSameSign(distold, distnew))
        return;

    float NdotV = dot(dom.nrm, m.vel);
    float t = -distold / NdotV; // Time until hit

    pVec phit = m.pos + m.vel * t; // Actual intersection point
    pVec offset = phit - dom.p; // Offset from origin in plane

    float radSqr = offset.length2();

    // Are we going to hit the disc ring?
    if(radSqr < dom.radInSqr || radSqr > dom.radOutSqr)
        return;

    // A hit! A most palpable hit!
    // Compute tangential and normal components of velocity
    pVec vn = dom.nrm * NdotV; // Normal Vn = (V.N)N
    pVec vt = m.vel - vn;   // Tangent Vt = V - Vn

    // Compute new velocity heading out:
    // Don't apply friction if tangential velocity < cutoff
    // Bounce() doesn't work correctly with small time step sizes for particles sliding along a surface.
    // The friction and resilience parameters should not be scaled by dt, since a bounce happens instantaneously.
    // On the other hand, they should be scaled by dt because particles sliding along a surface will hit more
    // often if dt is smaller. If you have any suggestions, let me know.
    if(vt.length2() <= cutoffSqr)
        m.vel = vt - vn * resilience;
    else
        m.vel = vt * oneMinusFriction - vn * resilience;
}

PINLINE void PABounce_Impl(
                           Particle_t &m, float dt,
                           pDomain position,
                           float oneMinusFriction,
                           float resilience,
                           float cutoffSqr)
{
    switch(position.Which) {
    //case PDUnion_e: PABounceUnion_Impl(m, dt, position.PDUnion_V, oneMinusFriction, resilience, cutoffSqr); break;
    //case PDPoint_e: PABouncePoint_Impl(m, dt, position.PDPoint_V, oneMinusFriction, resilience, cutoffSqr); break;
    //case PDLine_e: PABounceLine_Impl(m, dt, position.PDLine_V, oneMinusFriction, resilience, cutoffSqr); break;
    case PDTriangle_e: PABounceTriangle_Impl(m, dt, position.PDTriangle_V, oneMinusFriction, resilience, cutoffSqr); break;
    case PDRectangle_e: PABounceRectangle_Impl(m, dt, position.PDRectangle_V, oneMinusFriction, resilience, cutoffSqr); break;
    case PDDisc_e: PABounceDisc_Impl(m, dt, position.PDDisc_V, oneMinusFriction, resilience, cutoffSqr); break;
    case PDPlane_e: PABouncePlane_Impl(m, dt, position.PDPlane_V, oneMinusFriction, resilience, cutoffSqr); break;
    //case PDBox_e: PABounceBox_Impl(m, dt, position.PDBox_V, oneMinusFriction, resilience, cutoffSqr); break;
    //case PDCylinder_e: PABounceCylinder_Impl(m, dt, position.PDCylinder_V, oneMinusFriction, resilience, cutoffSqr); break;
    //case PDCone_e: PABounceCone_Impl(m, dt, position.PDCone_V, oneMinusFriction, resilience, cutoffSqr); break;
    case PDSphere_e: PABounceSphere_Impl(m, dt, position.PDSphere_V, oneMinusFriction, resilience, cutoffSqr); break;
    //case PDBlob_e: PABounceBlob_Impl(m, dt, position.PDBlob_V, oneMinusFriction, resilience, cutoffSqr); break;
    default:
#ifndef __CUDACC__
        throw PErrNotImplemented(std::string("Bounce not implemented for domain ") + std::string(typeid(position).name()));
#endif
        break;
    }
}

// An action list within an action list
PINLINE void PACallActionList_Impl(
                                   Particle_t &m, float dt //,
                                   //int action_list_num
                                   )
{
    // XXX Type something here.
}

PINLINE void PACallback_Impl(
                             Particle_t &m, float dt //,
                             // P_PARTICLE_CALLBACK callback,
                             // pdata_t Data
                             )
{
    // XXX Type something here.
}

// Set the secondary position and velocity from current.
PINLINE void PACopyVertexB_Impl(
                                Particle_t &m, float dt,
                                bool copy_pos,
                                bool copy_vel)
{
    if(copy_pos && copy_vel) {
        m.posB = m.pos;
        m.upB = m.up;
        m.velB = m.vel;
    } else if(copy_pos) {
        m.posB = m.pos;
        m.upB = m.up;
    } else if(copy_vel) {
        m.velB = m.vel;
    }
}

// Dampen velocities
PINLINE void PADamping_Impl(
                            Particle_t &m, float dt,
                            pVec damping,
                            float vlowSqr,
                            float vhighSqr)
{
    // This is important if dt is != 1.
    pVec one=pVec_(1,1,1);
    pVec scale(one - ((one - damping) * dt));

    float vSqr = m.vel.length2();

    if(vSqr >= vlowSqr && vSqr <= vhighSqr) {
        m.vel = CompMult(m.vel, scale);
    }
}

// Dampen rotational velocities
PINLINE void PARotDamping_Impl(
                               Particle_t &m, float dt,
                               pVec damping,
                               float vlowSqr,
                               float vhighSqr)
{
    // This is important if dt is != 1.
    pVec one=pVec_(1,1,1);
    pVec scale(one - ((one - damping) * dt));

    float vSqr = m.rvel.length2();

    if(vSqr >= vlowSqr && vSqr <= vhighSqr) {
        m.rvel = CompMult(m.rvel, scale);
    }
}

// Exert force on each particle away from explosion center
PINLINE void PAExplosion_Impl(
                              Particle_t &m, float dt,
                              pVec center,
                              float radius,
                              float magnitude,
                              float stdev,
                              float epsilon)
{
    float magdt = magnitude * dt;
    float oneOverSigma = 1.0f / stdev;
    float inexp = -0.5f*fsqr(oneOverSigma);
    float outexp = P_ONEOVERSQRT2PI * oneOverSigma;

    // Figure direction to particle.
    pVec dir(m.pos - center);
    float distSqr = dir.length2();
    float dist = sqrtf(distSqr);
    float DistFromWaveSqr = fsqr(radius - dist);

    float Gd = exp(DistFromWaveSqr * inexp) * outexp;
    pVec amount = dir * (Gd * magdt / (dist * (distSqr + epsilon)));

    m.vel += amount;
}

// Follow the next particle in the list
PINLINE void PAFollow_Impl(
                           Particle_t &m, float dt,
                           float magnitude,
                           float epsilon,
                           float max_radius)
{
#if 0
    // XXX Need to do something special in the caller to get the other particle.

    // Accelerate toward the particle after me in the list.
    pVec tohim((*next).pos - m.pos); // tohim = p1 - p0
    float tohimlenSqr = tohim.length2();

    if(tohimlenSqr < max_radiusSqr) {
        // Compute force exerted between the two bodies
        m.vel += tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon)));
    }
#endif
}

// Inter-particle gravitation
PINLINE void PAGravitate_Impl(
                              Particle_t &m, float dt,
                              float magnitude,
                              float epsilon,
                              float max_radius)
{
#if 0
    // XXX Need access to all the particles.
    float magdt = magnitude * dt;
    float max_radiusSqr = max_radius * max_radius;

    ParticleList::iterator j = it;
    j++;

    // Add interactions with other particles
    for(; j != iend; ++j) {
        Particle_t &mj = (*j);

        pVec tohim(mj.pos - m.pos); // tohim = p1 - p0
        float tohimlenSqr = tohim.length2();

        if(tohimlenSqr < max_radiusSqr) {
            // Compute force exerted between the two bodies
            pVec acc(tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon))));

            m.vel += acc;
            mj.vel -= acc;
        }
    }
#endif
}

// Acceleration in a constant direction
PINLINE void PAGravity_Impl(
                            Particle_t &m, float dt,
                            pVec direction)
{
    // Step velocity with acceleration
    m.vel += (direction * dt);
}

// For particles in the domain of influence, accelerate them with a domain.
PINLINE void PAJet_Impl(
                        Particle_t &m, float dt,
                        pDomain dom,
                        pDomain acc)
{
    if(dom.Within(m.pos)) {
        pVec accel = acc.Generate();

        // Step velocity with acceleration
        m.vel += accel * dt;
    }
}

// Get rid of older particles
PINLINE void PAKillOld_Impl(
                            Particle_t &m, float dt,
                            float age_limit,
                            bool kill_less_than)
{
    if(!((m.age < age_limit) ^ kill_less_than)) {
        m.tmp0 = 1.0f;
    }
}

// Match velocity to near neighbors
PINLINE void PAMatchVelocity_Impl(
                                  Particle_t &m, float dt,
                                  float magnitude,
                                  float epsilon,
                                  float max_radius)
{
#if 0
    // XXX Need to handle N squared here.
    float magdt = magnitude * dt;
    float max_radiusSqr = max_radius * max_radius;

    // Add interactions with other particles
    ParticleList::iterator j = it;
    j++;

    // Add interactions with other particles
    for(; j != iend; ++j) {
        Particle_t &mj = (*j);

        pVec tohim(mj.pos - m.pos); // tohim = p1 - p0
        float tohimlenSqr = tohim.length2();

        if(tohimlenSqr < max_radiusSqr) {
            // Compute force exerted between the two bodies
            pVec acc(mj.vel * (magdt / (tohimlenSqr + epsilon)));

            m.vel += acc;
            mj.vel -= acc;
        }
    }
#endif
}

// Match Rotational velocity to near neighbors
PINLINE void PAMatchRotVelocity_Impl(
                                     Particle_t &m, float dt,
                                     float magnitude,
                                     float epsilon,
                                     float max_radius)
{
#if 0
    // XXX Need to handle N squared here.
    float magdt = magnitude * dt;
    float max_radiusSqr = max_radius * max_radius;

    // Add interactions with other particles
    ParticleList::iterator j = it;
    j++;

    // Add interactions with other particles
    for(; j != iend; ++j) {
        Particle_t &mj = (*j);

        pVec tohim(mj.pos - m.pos); // tohim = p1 - p0
        float tohimlenSqr = tohim.length2();

        if(tohimlenSqr < max_radiusSqr) {
            // Compute force exerted between the two bodies
            pVec acc(mj.rvel * (magdt / (tohimlenSqr + epsilon)));

            m.rvel += acc;
            mj.rvel -= acc;
        }
    }
#endif
}

// Apply the particles' velocities to their positions, and age the particles
PINLINE void PAMove_Impl(
                         Particle_t &m, float dt,
                         bool move_velocity,
                         bool move_rotational_velocity)
{
    m.age += dt;
    if(move_rotational_velocity) {
        m.up += m.rvel * dt;
    }
    if(move_velocity) {
        m.pos += m.vel * dt;
    }
}

// Accelerate particles towards a line
PINLINE void PAOrbitLine_Impl(
                              Particle_t &m, float dt,
                              pVec p,
                              pVec axis,
                              float magnitude,
                              float epsilon,
                              float max_radius)
{
    float magdt = magnitude * dt;
    float max_radiusSqr = fsqr(max_radius);

    // Figure direction to particle from base of line.
    pVec f = m.pos - p;

    // Projection of particle onto line
    pVec w = axis * dot(f, axis);

    // Direction from particle to nearest point on line.
    pVec into = w - f;

    // Distance to line (force drops as 1/r^2, normalize by 1/r)
    // Soften by epsilon to avoid tight encounters to infinity
    float rSqr = into.length2();

    if(rSqr < max_radiusSqr)
        // Step velocity with acceleration
        m.vel += into * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
}

// Accelerate particles towards a point
PINLINE void PAOrbitPoint_Impl(
                               Particle_t &m, float dt,
                               pVec center,
                               float magnitude,
                               float epsilon,
                               float max_radius)
{
    float magdt = magnitude * dt;
    float max_radiusSqr = max_radius * max_radius;

    // Figure direction to particle.
    pVec dir(center - m.pos);

    // Distance to gravity well (force drops as 1/r^2, normalize by 1/r)
    // Soften by epsilon to avoid tight encounters to infinity
    float rSqr = dir.length2();

    // Step velocity with acceleration
    if(rSqr < max_radiusSqr)
        m.vel += dir * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
}

// Accelerate in random direction each time step
PINLINE void PARandomAccel_Impl(
                                Particle_t &m, float dt,
                                pDomain gen_acc)
{
    pVec accel = gen_acc.Generate();

    // dt will affect this by making a higher probability of
    // being near the original velocity after unit time. Smaller
    // dt approach a normal distribution instead of a square wave.
    m.vel += accel * dt;
}

// Immediately displace position randomly
PINLINE void PARandomDisplace_Impl(
                                   Particle_t &m, float dt,
                                   pDomain gen_disp)
{
    pVec disp = gen_disp.Generate();

    // dt will affect this by making a higher probability of
    // being near the original position after unit time. Smaller
    // dt approach a normal distribution instead of a square wave.
    m.pos += disp * dt;
}

// Immediately assign a random velocity
PINLINE void PARandomVelocity_Impl(
                                   Particle_t &m, float dt,
                                   pDomain gen_vel)
{
    pVec velocity = gen_vel.Generate();

    // Shouldn't multiply by dt because velocities are invariant of dt.
    m.vel = velocity;
}

// Immediately assign a random rotational velocity
PINLINE void PARandomRotVelocity_Impl(
                                      Particle_t &m, float dt,
                                      pDomain gen_vel)

{
    pVec velocity = gen_vel.Generate();

    // Shouldn't multiply by dt because velocities are invariant of dt.
    m.rvel = velocity;
}

// Figure new velocity at next timestep
PINLINE void Restore(pVec &vel, const pVec &posB, const pVec &pos, const float t,
                     const float dtSqr, const float ttInv6dt, const float tttInv3dtSqr)
{
    pVec b = (vel*-0.6667f*t + posB - pos) * ttInv6dt;
    pVec a = (vel*t - posB - posB + pos + pos) * tttInv3dtSqr;
    vel += a + b;
}

// Over time, restore particles to initial positions
PINLINE void PARestore_Impl(
                            Particle_t &m, float dt,
                            float time_left,
                            bool restore_velocity,
                            bool restore_rvelocity)
{
    if(time_left <= 0) {
        // Already constrained; keep it there.
        if (restore_velocity) {
            m.pos = m.posB;
            m.vel = pVec_(0.0f,0.0f,0.0f);
        }
        if (restore_rvelocity) {
            m.up = m.upB;
            m.rvel = pVec_(0.0f,0.0f,0.0f);
        }
    } else {
        float t = time_left;
        float dtSqr = fsqr(dt);
        float ttInv6dt = dt * 6.0f / fsqr(t);
        float tttInv3dtSqr = dtSqr * 3.0f / (t * t * t);

        if (restore_velocity)
            Restore(m.vel, m.posB, m.pos, t, dtSqr, ttInv6dt, tttInv3dtSqr);
        if (restore_rvelocity)
            Restore(m.rvel, m.upB, m.up, t, dtSqr, ttInv6dt, tttInv3dtSqr);
    }
}

// Kill particles with positions on wrong side of the specified domain
PINLINE void PASink_Impl(
                         Particle_t &m, float dt,
                         bool kill_inside,
                         pDomain position)
{
    // Remove if inside/outside flag matches object's flag
    if(!(position.Within(m.pos) ^ kill_inside)) {
        m.tmp0 = 1.0f;
    }
}

// Kill particles with velocities on wrong side of the specified domain
PINLINE void PASinkVelocity_Impl(
                                 Particle_t &m, float dt,
                                 bool kill_inside,
                                 pDomain velocity)
{
    // Remove if inside/outside flag matches object's flag
    if(!(velocity.Within(m.vel) ^ kill_inside)) {
        m.tmp0 = 1.0f;
    }
}

// Sort the particles by their projection onto the Look vector
PINLINE void PASort_Impl(
                         Particle_t &m, float dt,
                         pVec Eye,
                         pVec Look,
                         bool front_to_back,
                         bool clamp_negative)
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
    if(pRandf() < particle_rate * dt - float(rate))
        rate++;

    // Don't emit more than it can hold.
    if(group_size + rate > group_cap)
        rate = group_cap - group_size;

    return rate;
}

// Create a single particle
PINLINE void PASource_Impl(
                           Particle_t &m, const float dt,
                           const pDomain &position,
                           const pSourceState &SrcSt
                           )
{
    m.pos = position.Generate();
    m.posB = SrcSt.vertexB_tracks_ ? m.pos : SrcSt.VertexB_.Generate();
    m.up = SrcSt.Up_.Generate();
    m.vel = SrcSt.Vel_.Generate();
    m.rvel = SrcSt.RotVel_.Generate();
    m.size = SrcSt.Size_.Generate();
    m.color = SrcSt.Color_.Generate();
    m.alpha = SrcSt.Alpha_.Generate().x();
    m.age = SrcSt.Age_ + pNRandf(SrcSt.AgeSigma_);
    m.mass = SrcSt.Mass_;
    m.tmp0 = 0;
    m.data = SrcSt.Data_;
}

// Clamp particle velocities to the given range
PINLINE void PASpeedLimit_Impl(
                               Particle_t &m, float dt,
                               float min_speed,
                               float max_speed)
{
    float min_sqr = fsqr(min_speed);
    float max_sqr = fsqr(max_speed);

    float sSqr = m.vel.length2();
    if(sSqr<min_sqr && sSqr) {
        float s = sqrtf(sSqr);
        m.vel *= (min_speed/s);
    } else if(sSqr>max_sqr) {
        float s = sqrtf(sSqr);
        m.vel *= (max_speed/s);
    }
}

// Change color of all particles toward the specified color

PINLINE void PATargetColor_Impl(
                                Particle_t &m, float dt,
                                pVec color,
                                float alpha,
                                float scale)
{
    float scaleFac = scale * dt;

    m.color += (color - m.color) * scaleFac;
    m.alpha += (alpha - m.alpha) * scaleFac;
}

// Change sizes of all particles toward the specified size
PINLINE void PATargetSize_Impl(
                               Particle_t &m, float dt,
                               pVec size,
                               pVec scale)
{
    pVec scaleFac = scale * dt;

    pVec dif = size - m.size;
    m.size += CompMult(dif, scaleFac);
}

// Change velocity of all particles toward the specified velocity
PINLINE void PATargetVelocity_Impl(
                                   Particle_t &m, float dt,
                                   pVec velocity,
                                   float scale)
{
    float scaleFac = scale * dt;

    m.vel += (velocity - m.vel) * scaleFac;
}

// Change velocity of all particles toward the specified velocity
PINLINE void PATargetRotVelocity_Impl(
                                      Particle_t &m, float dt,
                                      pVec velocity,
                                      float scale)
{
    float scaleFac = scale * dt;

    m.rvel += (velocity - m.rvel) * scaleFac;
}

PINLINE void PAVortex_Impl(
                           Particle_t &m, float dt,
                           pVec tip,
                           pVec axis,
                           float tightnessExponent,
                           float max_radius,
                           float inSpeed,
                           float upSpeed,
                           float aroundSpeed)
{
    float max_radiusSqr = fsqr(max_radius);
    float axisLength = axis.length();
    float axisLengthInv = 1.0f / axisLength;
    pVec axisN = axis;
    axisN.normalize();

    // This one just rotates a particle around the axis. Amount is based on radius, magnitude, and mass.

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
    float rSqr = parToAxis.length2();

    if(rSqr >= max_radiusSqr || axisScale < 0.0f || alongAxis > 1.0f) {
        //m.color = pVec_(0,0,1);
        return;
    }

    float r = sqrtf(rSqr);
    parToAxis /= r;
    float dtOverMass = dt / m.mass;

    if(rSqr >= silhouetteSqr) {
        // Accelerate toward axis. Force is NOT affected by 1/r^2.
        pVec AccelIn = parToAxis * (inSpeed * dtOverMass);
        m.vel += AccelIn;
        //m.color = pVec_(0,1,0);
        return;
    }

    //m.color = pVec_(1,0,0);
    // Accelerate up or down to simulate gravity or something
    pVec AccelUp = axisN * (upSpeed * dtOverMass);

    // Accelerate around axis by constructing orthogonal vector frame of axis, parToAxis, and RotDir.
    pVec RotDir = Cross(axisN, parToAxis);
    pVec AccelAround = RotDir * (aroundSpeed * dtOverMass);
    m.vel = AccelUp + AccelAround; // NOT += because we want to stop its inward travel.
}

#endif
