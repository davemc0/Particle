// actions.cpp
//
// Copyright 1997-2005 by David K. McAllister
//
// I used code Copyright 1997 by Jonathan P. Leech
// as an example in implenting this.
//
// This file implements the dynamics of particle actions.

#include "general.h"

#include <algorithm>
// For dumping errors
#include <iostream>

void PAAvoid::ExecTriangle(ParticleGroup *group)
{
	float magdt = magnitude * dt;

	// Compute the inverse matrix of the plane basis.
	pVector &u = position.u;
	pVector &v = position.v;

	// The normalized bases are needed inside the loop.
	pVector un = u / position.radius1Sqr;
	pVector vn = v / position.radius2Sqr;

	// f is the third (non-basis) triangle edge.
	pVector f = v - u;
	pVector fn(f);
	fn.normalize();

	// w = u cross v
	float wx = u.y*v.z-u.z*v.y;
	float wy = u.z*v.x-u.x*v.z;
	float wz = u.x*v.y-u.y*v.x;

	float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);

	pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
	s1 *= det;
	pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
	s2 *= -det;

	// See which particles hit.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// See if particle's current and next positions cross plane.
		// If not, couldn't hit, so keep going.
		pVector pnext(m.pos + m.vel * dt * look_ahead);

		// p2 stores the plane normal (the a,b,c of the plane eqn).
		// Old and new distances: dist(p,plane) = n * p + d
		// radius1 stores -n*p, which is d.
		float distold = m.pos * position.p2 + position.radius1;
		float distnew = pnext * position.p2 + position.radius1;

		// Opposite signs if product < 0
		// Is there a faster way to do this?
		if(distold * distnew >= 0)
			continue;

		float nv = position.p2 * m.vel;
		float t = -distold / nv;

		// Actual intersection point p(t) = pos + vel t
		pVector phit(m.pos + m.vel * t);

		// Offset from origin in plane, p - origin
		pVector offset(phit - position.p1);

		// Dot product with basis vectors of old frame
		// in terms of new frame gives position in uv frame.
		float upos = offset * s1;
		float vpos = offset * s2;

		// Did it cross plane outside triangle?
		if(upos < 0 || vpos < 0 || (upos + vpos) > 1)
			continue;

		// A hit! A most palpable hit!
		// Compute distance to the three edges.
		pVector uofs = (un * (un * offset)) - offset;
		float udistSqr = uofs.length2();
		pVector vofs = (vn * (vn * offset)) - offset;
		float vdistSqr = vofs.length2();
		pVector foffset(offset - u);
		pVector fofs = (fn * (fn * foffset)) - foffset;
		float fdistSqr = fofs.length2();
		pVector S;
		if(udistSqr <= vdistSqr && udistSqr <= fdistSqr) S = uofs;
		else if(vdistSqr <= fdistSqr) S = vofs;
		else S = fofs;

		S.normalize();

		// We now have a vector to safety.
		float vm = m.vel.length();
		pVector Vn = m.vel / vm;

		// Blend S into V.
		pVector tmp = (S * (magdt / (t*t+epsilon))) + Vn;
		m.vel = tmp * (vm / tmp.length());
	}
}

void PAAvoid::ExecDisc(ParticleGroup *group)
{
	float magdt = magnitude * dt;

	float r1Sqr = fsqr(position.radius1);
	float r2Sqr = fsqr(position.radius2);

	// See which particles hit.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// See if particle's current and next positions cross plane.
		// If not, couldn't hit, so keep going.
		pVector pnext(m.pos + m.vel * dt * look_ahead);

		// p2 stores the plane normal (the a,b,c of the plane eqn).
		// Old and new distances: dist(p,plane) = n * p + d
		// radius1 stores -n*p, which is d. radius1Sqr stores d.
		float distold = m.pos * position.p2 + position.radius1Sqr;
		float distnew = pnext * position.p2 + position.radius1Sqr;

		// Opposite signs if product < 0
		// Is there a faster way to do this?
		if(distold * distnew >= 0)
			continue;

		// Find position at the crossing point by parameterizing
		// p(t) = pos + vel * t
		// Solve dist(p(t),plane) = 0 e.g.
		// n * p(t) + D = 0 ->
		// n * p + t (n * v) + D = 0 ->
		// t = -(n * p + D) / (n * v)
		// Could factor n*v into distnew = distold + n*v and save a bit.
		// Safe since n*v != 0 assured by quick rejection test.
		// This calc is indep. of dt because we have established that it
		// will hit before dt. We just want to know when.
		float nv = position.p2 * m.vel;
		float t = -distold / nv;

		// Actual intersection point p(t) = pos + vel t
		pVector phit(m.pos + m.vel * t);

		// Offset from origin in plane, phit - origin
		pVector offset(phit - position.p1);

		float rad = offset.length2();

		if(rad > r1Sqr || rad < r2Sqr)
			continue;

		// A hit! A most palpable hit!
		pVector S = offset;
		S.normalize();

		// We now have a vector to safety.
		float vm = m.vel.length();
		pVector Vn = m.vel / vm;

		// Blend S into V.
		pVector tmp = (S * (magdt / (t*t+epsilon))) + Vn;
		m.vel = tmp * (vm / tmp.length());
	}
}

void PAAvoid::ExecPlane(ParticleGroup *group)
{
	float magdt = magnitude * dt;

	if(look_ahead < P_MAXFLOAT) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// p2 stores the plane normal (the a,b,c of the plane eqn).
			// Old and new distances: dist(p,plane) = n * p + d
			// radius1 stores -n*p, which is d.
			float dist = m.pos * position.p2 + position.radius1;

			if(dist < look_ahead) {
				float vm = m.vel.length();
				pVector Vn = m.vel / vm;
				// float dot = Vn * position.p2;

				pVector tmp = (position.p2 * (magdt / (dist*dist+epsilon))) + Vn;
				m.vel = tmp * (vm / tmp.length());
			}
		}
	} else {
		// If not using radius cutoff, avoid the if().
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// p2 stores the plane normal (the a,b,c of the plane eqn).
			// Old and new distances: dist(p,plane) = n * p + d
			// radius1 stores -n*p, which is d.
			float dist = m.pos * position.p2 + position.radius1;

			float vm = m.vel.length();
			pVector Vn = m.vel / vm;
			// float dot = Vn * position.p2;

			pVector tmp = (position.p2 * (magdt / (dist*dist+epsilon))) + Vn;
			m.vel = tmp * (vm / tmp.length());
		}
	}
}

void PAAvoid::ExecRectangle(ParticleGroup *group)
{
	float magdt = magnitude * dt;

	// Compute the inverse matrix of the plane basis.
	pVector &u = position.u;
	pVector &v = position.v;

	// The normalized bases are needed inside the loop.
	pVector un = u / position.radius1Sqr;
	pVector vn = v / position.radius2Sqr;

	// w = u cross v
	float wx = u.y*v.z-u.z*v.y;
	float wy = u.z*v.x-u.x*v.z;
	float wz = u.x*v.y-u.y*v.x;

	float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);

	pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
	s1 *= det;
	pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
	s2 *= -det;

	// See which particles hit.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// See if particle's current and next positions cross plane.
		// If not, couldn't hit, so keep going.
		pVector pnext(m.pos + m.vel * dt * look_ahead);

		// p2 stores the plane normal (the a,b,c of the plane eqn).
		// Old and new distances: dist(p,plane) = n * p + d
		// radius1 stores -n*p, which is d.
		float distold = m.pos * position.p2 + position.radius1;
		float distnew = pnext * position.p2 + position.radius1;

		// Opposite signs if product < 0
		// There is no faster way to do this.
		if(distold * distnew >= 0)
			continue;

		float nv = position.p2 * m.vel;
		float t = -distold / nv;

		// Actual intersection point p(t) = pos + vel t
		pVector phit(m.pos + m.vel * t);

		// Offset from origin in plane, p - origin
		pVector offset(phit - position.p1);

		// Dot product with basis vectors of old frame
		// in terms of new frame gives position in uv frame.
		float upos = offset * s1;
		float vpos = offset * s2;

		// Did it cross plane outside triangle?
		if(upos < 0 || vpos < 0 || upos > 1 || vpos > 1)
			continue;

		// A hit! A most palpable hit!
		// Compute distance to the three edges.
		pVector uofs = (un * (un * offset)) - offset;
		float udistSqr = uofs.length2();
		pVector vofs = (vn * (vn * offset)) - offset;
		float vdistSqr = vofs.length2();

		pVector foffset((u + v) - offset);
		pVector fofs = (un * (un * foffset)) - foffset;
		float fdistSqr = fofs.length2();
		pVector gofs = (un * (un * foffset)) - foffset;
		float gdistSqr = gofs.length2();

		pVector S;
		if(udistSqr <= vdistSqr && udistSqr <= fdistSqr
			&& udistSqr <= gdistSqr) S = uofs;
		else if(vdistSqr <= fdistSqr && vdistSqr <= gdistSqr) S = vofs;
		else if(fdistSqr <= gdistSqr) S = fofs;
		else S = gofs;

		S.normalize();

		// We now have a vector to safety.
		float vm = m.vel.length();
		pVector Vn = m.vel / vm;

		// Blend S into V.
		pVector tmp = (S * (magdt / (t*t+epsilon))) + Vn;
		m.vel = tmp * (vm / tmp.length());
	}
}

void PAAvoid::ExecSphere(ParticleGroup *group)
{
	float magdt = magnitude * dt;

	float rSqr = position.radius1 * position.radius1;

	// See which particles are aimed toward the sphere.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// First do a ray-sphere intersection test and
		// see if it's soon enough.
		// Can I do this faster without t?
		float vm = m.vel.length();
		pVector Vn = m.vel / vm;

		pVector L = position.p1 - m.pos;
		float v = L * Vn;

		float disc = rSqr - (L * L) + v * v;
		if(disc < 0)
			continue; // I'm not heading toward it.

		// Compute length for second rejection test.
		float t = v - sqrtf(disc);
		if(t < 0 || t > (vm * look_ahead))
			continue;

		// Get a vector to safety.
		pVector C = Cross(Vn, L);
		C.normalize();
		pVector S = Cross(Vn, C);

		// Blend S into V.
		pVector tmp = (S * (magdt / (t*t+epsilon))) + Vn;
		m.vel = tmp * (vm / tmp.length());
	}
}

void PAAvoid::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;

	switch(position.type) {
	case PDTriangle:
		ExecTriangle(group);
		break;
	case PDDisc:
		ExecDisc(group);
		break;
	case PDPlane:
		ExecPlane(group);
		break;
	case PDRectangle:
		ExecRectangle(group);
		break;
	case PDSphere:
		ExecSphere(group);
		break;
	default:
		std::cerr << "pAvoid not implemented for this domain type. Sorry.\n";
		abort();
		break;
	}
}

void PABounce::ExecTriangle(ParticleGroup *group)
{
	// Compute the inverse matrix of the plane basis.
	pVector &u = position.u;
	pVector &v = position.v;

	// w = u cross v
	float wx = u.y*v.z-u.z*v.y;
	float wy = u.z*v.x-u.x*v.z;
	float wz = u.x*v.y-u.y*v.x;

	float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);

	pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
	s1 *= det;
	pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
	s2 *= -det;

	// See which particles bounce.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// See if particle's current and next positions cross plane.
		// If not, couldn't bounce, so keep going.
		pVector pnext(m.pos + m.vel * dt);

		// p2 stores the plane normal (the a,b,c of the plane eqn).
		// Old and new distances: dist(p,plane) = n * p + d
		// radius1 stores -n*p, which is d.
		float distold = m.pos * position.p2 + position.radius1;
		float distnew = pnext * position.p2 + position.radius1;

		// Opposite signs if product < 0
		// Is there a faster way to do this?
		if(distold * distnew >= 0)
			continue;

		// Find position at the crossing point by parameterizing
		// p(t) = pos + vel * t
		// Solve dist(p(t),plane) = 0 e.g.
		// n * p(t) + D = 0 ->
		// n * p + t (n * v) + D = 0 ->
		// t = -(n * p + D) / (n * v)
		// Could factor n*v into distnew = distold + n*v and save a bit.
		// Safe since n*v != 0 assured by quick rejection test.
		// This calc is indep. of dt because we have established that it
		// will hit before dt. We just want to know when.
		float nv = position.p2 * m.vel;
		float t = -distold / nv;

		// Actual intersection point p(t) = pos + vel t
		pVector phit(m.pos + m.vel * t);

		// Offset from origin in plane, p - origin
		pVector offset(phit - position.p1);

		// Dot product with basis vectors of old frame
		// in terms of new frame gives position in uv frame.
		float upos = offset * s1;
		float vpos = offset * s2;

		// Did it cross plane outside triangle?
		if(upos < 0 || vpos < 0 || (upos + vpos) > 1)
			continue;

		// A hit! A most palpable hit!

		// Compute tangential and normal components of velocity
		pVector vn(position.p2 * nv); // Normal Vn = (V.N)N
		pVector vt(m.vel - vn); // Tangent Vt = V - Vn

		// Compute new velocity heading out:
		// Don't apply friction if tangential velocity < cutoff
		if(vt.length2() <= cutoffSqr)
			m.vel = vt - vn * resilience;
		else
			m.vel = vt * oneMinusFriction - vn * resilience;
	}
}

void PABounce::ExecDisc(ParticleGroup *group)
{
	float r1Sqr = fsqr(position.radius1);
	float r2Sqr = fsqr(position.radius2);

	// See which particles bounce.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// See if particle's current and next positions cross plane.
		// If not, couldn't bounce, so keep going.
		pVector pnext(m.pos + m.vel * dt);

		// p2 stores the plane normal (the a,b,c of the plane eqn).
		// Old and new distances: dist(p,plane) = n * p + d
		// radius1 stores -n*p, which is d. radius1Sqr stores d.
		float distold = m.pos * position.p2 + position.radius1Sqr;
		float distnew = pnext * position.p2 + position.radius1Sqr;

		// Opposite signs if product < 0
		// Is there a faster way to do this?
		if(distold * distnew >= 0)
			continue;

		// Find position at the crossing point by parameterizing
		// p(t) = pos + vel * t
		// Solve dist(p(t),plane) = 0 e.g.
		// n * p(t) + D = 0 ->
		// n * p + t (n * v) + D = 0 ->
		// t = -(n * p + D) / (n * v)
		// Could factor n*v into distnew = distold + n*v and save a bit.
		// Safe since n*v != 0 assured by quick rejection test.
		// This calc is indep. of dt because we have established that it
		// will hit before dt. We just want to know when.
		float nv = position.p2 * m.vel;
		float t = -distold / nv;

		// Actual intersection point p(t) = pos + vel t
		pVector phit(m.pos + m.vel * t);

		// Offset from origin in plane, phit - origin
		pVector offset(phit - position.p1);

		float rad = offset.length2();

		if(rad > r1Sqr || rad < r2Sqr)
			continue;

		// A hit! A most palpable hit!

		// Compute tangential and normal components of velocity
		pVector vn(position.p2 * nv); // Normal Vn = (V.N)N
		pVector vt(m.vel - vn); // Tangent Vt = V - Vn

		// Compute new velocity heading out:
		// Don't apply friction if tangential velocity < cutoff
		if(vt.length2() <= cutoffSqr)
			m.vel = vt - vn * resilience;
		else
			m.vel = vt * oneMinusFriction - vn * resilience;
	}
}

void PABounce::ExecPlane(ParticleGroup *group)
{
	// See which particles bounce.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// See if particle's current and next positions cross plane.
		// If not, couldn't bounce, so keep going.
		pVector pnext(m.pos + m.vel * dt);

		// p2 stores the plane normal (the a,b,c of the plane eqn).
		// Old and new distances: dist(p,plane) = n * p + d
		// radius1 stores -n*p, which is d.
		float distold = m.pos * position.p2 + position.radius1;
		float distnew = pnext * position.p2 + position.radius1;

		// Opposite signs if product < 0
		if(distold * distnew >= 0)
			continue;

		// Compute tangential and normal components of velocity
		float nmag = m.vel * position.p2;
		pVector vn(position.p2 * nmag); // Normal Vn = (V.N)N
		pVector vt(m.vel - vn); // Tangent Vt = V - Vn

		// Compute new velocity heading out:
		// Don't apply friction if tangential velocity < cutoff
		if(vt.length2() <= cutoffSqr)
			m.vel = vt - vn * resilience;
		else
			m.vel = vt * oneMinusFriction - vn * resilience;
	}
}

void PABounce::ExecRectangle(ParticleGroup *group)
{
	// Compute the inverse matrix of the plane basis.
	pVector &u = position.u;
	pVector &v = position.v;

	// w = u cross v
	float wx = u.y*v.z-u.z*v.y;
	float wy = u.z*v.x-u.x*v.z;
	float wz = u.x*v.y-u.y*v.x;

	float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);

	pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
	s1 *= det;
	pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
	s2 *= -det;

	// See which particles bounce.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// See if particle's current and next positions cross plane.
		// If not, couldn't bounce, so keep going.
		pVector pnext(m.pos + m.vel * dt);

		// p2 stores the plane normal (the a,b,c of the plane eqn).
		// Old and new distances: dist(p,plane) = n * p + d
		// radius1 stores -n*p, which is d.
		float distold = m.pos * position.p2 + position.radius1;
		float distnew = pnext * position.p2 + position.radius1;

		// Opposite signs if product < 0
		if(distold * distnew >= 0)
			continue;

		// Find position at the crossing point by parameterizing
		// p(t) = pos + vel * t
		// Solve dist(p(t),plane) = 0 e.g.
		// n * p(t) + D = 0 ->
		// n * p + t (n * v) + D = 0 ->
		// t = -(n * p + D) / (n * v)
		float t = -distold / (position.p2 * m.vel);

		// Actual intersection point p(t) = pos + vel t
		pVector phit(m.pos + m.vel * t);

		// Offset from origin in plane, p - origin
		pVector offset(phit - position.p1);

		// Dot product with basis vectors of old frame
		// in terms of new frame gives position in uv frame.
		float upos = offset * s1;
		float vpos = offset * s2;

		// Crossed plane outside bounce region if !(0<=[uv]pos<=1)
		if(upos < 0 || upos > 1 || vpos < 0 || vpos > 1)
			continue;

		// A hit! A most palpable hit!

		// Compute tangential and normal components of velocity
		float nmag = m.vel * position.p2;
		pVector vn(position.p2 * nmag); // Normal Vn = (V.N)N
		pVector vt(m.vel - vn); // Tangent Vt = V - Vn

		// Compute new velocity heading out:
		// Don't apply friction if tangential velocity < cutoff
		if(vt.length2() <= cutoffSqr)
			m.vel = vt - vn * resilience;
		else
			m.vel = vt * oneMinusFriction - vn * resilience;
	}
}

void PABounce::ExecSphere(ParticleGroup *group)
{
	float dtinv = 1.0f / dt;

	// Bounce particles off the inside or outside of the sphere
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// See if particle's next position is on the opposite side of the domain. If so, bounce it.
		pVector pnext = m.pos + m.vel * dt;

		if(position.Within(m.pos)) {
			// We are bouncing off the inside of the sphere.
			if(position.Within(pnext))
				// Still inside. Do nothing.
				continue;

			// Trying to go outside. Bounce back in.

			// Inward-pointing normal to surface. This isn't computed quite right;
			// should extrapolate particle position to surface.
			pVector n(position.p1 - m.pos);
			n.normalize();

			// Compute tangential and normal components of velocity
			float nmag = m.vel * n;

			pVector vn = n * nmag;   // Velocity in Normal dir  Vn = (V.N)N
			pVector vt = m.vel - vn; // Velocity in Tangent dir Vt = V - Vn

			// Reverse normal component of velocity
			if(nmag < 0) vn = -vn; // Don't reverse if it's already heading inward

			// Compute new velocity heading out:
			// Don't apply friction if tangential velocity < cutoff
			float tanscale = (vt.length2() <= cutoffSqr) ? 1.0f : oneMinusFriction;
			m.vel = vt * tanscale + vn * resilience;

			// Now see where the point will end up. Make sure we fixed it to stay inside.
			pVector pthree = m.pos + m.vel * dt;
			if(position.Within(pthree)) {
				// Still inside. We're good.
				continue;
			} else {
				pVector toctr = position.p1 - pthree;
				float dist = toctr.length();
				pVector pwish = position.p1 - toctr * (0.999f * position.radius1 / dist);
				m.vel = (pwish - m.pos) * dtinv; // Compute a velocity to get us to pwish.
			}
		} else {
			// We are bouncing off the outside of the sphere.
			if(!position.Within(pnext))
				continue;

			// Trying to go inside. Bounce back out.

			// Outward-pointing normal to surface. This isn't computed quite right;
			// should extrapolate particle position to surface.
			pVector n(m.pos - position.p1);
			n.normalize();

			// Compute tangential and normal components of velocity
			float nmag = m.vel * n;

			pVector vn = n * nmag;   // Velocity in Normal dir  Vn = (V.N)N
			pVector vt = m.vel - vn; // Velocity in Tangent dir Vt = V - Vn

			// Reverse normal component of velocity
			vn = -vn;

			// Compute new velocity heading out:
			// Don't apply friction if tangential velocity < cutoff
			float tanscale = (vt.length2() <= cutoffSqr) ? 1.0f : oneMinusFriction;
			m.vel = vt * tanscale + vn * resilience;

#if 0
			// This shouldn't be necessary because we reflect velocity vector off the tangent plane,
			// which is outside 
			// Now see where the point will end up. Make sure we fixed it to stay outside.
			pVector pthree = m.pos + m.vel * dt;
			if(!position.Within(pthree)) {
				// Still outside. We're good.
				continue;
			} else {
				pVector toctr = position.p1 - pthree;
				float dist = toctr.length();
				pVector pwish = position.p1 - toctr * (1.1001f * position.radius1 / dist);
				m.vel = (pwish - m.pos) * dtinv; // Compute a velocity to get us to pwish.
			pVector pfour = m.pos + m.vel * dt;
				if(position.Within(pfour))
					std::cerr << "GRRR!\n";
			}
#endif
		}
	}
}

void PABounce::Execute(ParticleGroup *group)
{
	switch(position.type) {
	case PDTriangle:
		ExecTriangle(group);
		break;
	case PDDisc:
		ExecDisc(group);
		break;
	case PDPlane:
		ExecPlane(group);
		break;
	case PDRectangle:
		ExecRectangle(group);
		break;
	case PDSphere:
		ExecSphere(group);
		break;
	default:
		std::cerr << "pBounce not implemented for this domain type. Sorry.\n";
		abort();
		break;
	}
}

// An action list within an action list
void PACallActionList::Execute(ParticleGroup *group)
{
	pCallActionList(action_list_num);
}

// Set the secondary position and velocity from current.
void PACopyVertexB::Execute(ParticleGroup *group)
{
	if(copy_pos && copy_vel) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			m.posB = m.pos;
			m.velB = m.vel;
		}
	} else if(copy_pos) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);
			m.posB = m.pos;
		}
	} else if(copy_vel) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);
			m.velB = m.vel;
			m.rvelB = m.rvel;
		}
	}
}

// Dampen velocities
void PADamping::Execute(ParticleGroup *group)
{
	// This is important if dt is != 1.
	pVector one(1,1,1);
	pVector scale(one - ((one - damping) * dt));

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		float vSqr = m.vel.length2();

		if(vSqr >= vlowSqr && vSqr <= vhighSqr) {
			m.vel.x *= scale.x;
			m.vel.y *= scale.y;
			m.vel.z *= scale.z;
		}
	}
}

// Dampen rotational velocities
void PARotDamping::Execute(ParticleGroup *group)
{
	// This is important if dt is != 1.
	pVector one(1,1,1);
	pVector scale(one - ((one - damping) * dt));

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		float vSqr = m.rvel.length2();

		if(vSqr >= vlowSqr && vSqr <= vhighSqr) {
			m.rvel.x *= scale.x;
			m.rvel.y *= scale.y;
			m.rvel.z *= scale.z;
		}
	}
}

// Exert force on each particle away from explosion center
void PAExplosion::Execute(ParticleGroup *group)
{
	float radius = velocity * age;
	float magdt = magnitude * dt;
	float oneOverSigma = 1.0f / stdev;
	float inexp = -0.5f*fsqr(oneOverSigma);
	float outexp = ONEOVERSQRT2PI * oneOverSigma;

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// Figure direction to particle.
		pVector dir(m.pos - center);
		float distSqr = dir.length2();
		float dist = sqrtf(distSqr);
		float DistFromWaveSqr = fsqr(radius - dist);

		float Gd = exp(DistFromWaveSqr * inexp) * outexp;
		pVector amount = dir * (Gd * magdt / (dist * (distSqr + epsilon)));

		m.vel += amount;
	}

	age += dt;
}

// Follow the next particle in the list
void PAFollow::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;

	if (group->size() < 2)
		return;

	ParticleList::iterator end = group->end();
	--end;

	if(max_radiusSqr < P_MAXFLOAT) {
		for (ParticleList::iterator it = group->begin(); it != end; ++it) {
			Particle &m = (*it);
			ParticleList::iterator next = it;
			++next;

			// Accelerate toward the particle after me in the list.
			pVector tohim((*next).pos - m.pos); // tohim = p1 - p0
			float tohimlenSqr = tohim.length2();

			if(tohimlenSqr < max_radiusSqr) {
				// Compute force exerted between the two bodies
				m.vel += tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon)));
			}
		}
	} else {
		// If not using radius cutoff, avoid the if().
		for (ParticleList::iterator it = group->begin(); it != end; ++it) {
			Particle &m = (*it);
			ParticleList::iterator next = it;
			++next;

			// Accelerate toward the particle after me in the list.
			pVector tohim((*next).pos - m.pos); // tohim = p1 - p0
			float tohimlenSqr = tohim.length2();

			// Compute force exerted between the two bodies
			m.vel += tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon)));
		}
	}
}

// Inter-particle gravitation
void PAGravitate::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;

	if(max_radiusSqr < P_MAXFLOAT) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			ParticleList::iterator j = it;
			++j;

			// Add interactions with other particles
			for(; j != group->end(); ++j) {
				Particle &mj = (*j);

				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();

				if(tohimlenSqr < max_radiusSqr) {
					// Compute force exerted between the two bodies
					pVector acc(tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon))));

					m.vel += acc;
					mj.vel -= acc;
				}
			}
		}
	} else {
		// If not using radius cutoff, avoid the if().
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Add interactions with other particles
			ParticleList::iterator j = it;
			++j;

			for(; j != group->end(); ++j) {
				Particle &mj = (*j);

				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();

				// Compute force exerted between the two bodies
				pVector acc(tohim * (magdt / (sqrtf(tohimlenSqr) * (tohimlenSqr + epsilon))));

				m.vel += acc;
				mj.vel -= acc;
			}
		}
	}
}

// Acceleration in a constant direction
void PAGravity::Execute(ParticleGroup *group)
{
	pVector ddir(direction * dt);

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		// Step velocity with acceleration
		m.vel += ddir;
	}
}

// For particles in the sphere of influence, accelerate them with a domain.
// In the future, change the sphere of influence to a domain of influence.
// Only problem is you don't get the falloff.
// Also, need to be able to pass two domains to an action.
void PAJet::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;

	if(max_radiusSqr < P_MAXFLOAT) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Figure direction to particle.
			pVector dir = m.pos - center;

			// Distance to jet (force drops as 1/r^2)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = dir.length2();

			if(rSqr < max_radiusSqr) {
				pVector accel;
				acc.Generate(accel);

				// Step velocity with acceleration
				m.vel += accel * (magdt / (rSqr + epsilon));
			}
		}
	} else {
		// If not using radius cutoff, avoid the if().
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Figure direction to particle.
			pVector dir = m.pos - center;

			// Distance to jet (force drops as 1/r^2)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = dir.length2();

			pVector accel;
			acc.Generate(accel);

			// Step velocity with acceleration
			m.vel += accel * (magdt / (rSqr + epsilon));
		}
	}
}

// Get rid of older particles
void PAKillOld::Execute(ParticleGroup *group)
{
	// Must traverse list carefully so Remove will work
	for (ParticleList::iterator it = group->begin(); it != group->end(); ) {
		Particle &m = (*it);

		if(!((m.age < age_limit) ^ kill_less_than))
			group->Remove(it);
		else
			++it;
	}
}

// Match velocity to near neighbors
void PAMatchVelocity::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;

	if(max_radiusSqr < P_MAXFLOAT) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Add interactions with other particles
			ParticleList::iterator j = it;
			++j;

			// Add interactions with other particles
			for(; j != group->end(); ++j) {
				Particle &mj = (*j);

				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();

				if(tohimlenSqr < max_radiusSqr) {
					// Compute force exerted between the two bodies
					pVector acc(mj.vel * (magdt / (tohimlenSqr + epsilon)));

					m.vel += acc;
					mj.vel -= acc;
				}
			}
		}
	} else {
		// If not using radius cutoff, avoid the if().
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Add interactions with other particles
			ParticleList::iterator j = it;
			++j;

			// Add interactions with other particles
			for(; j != group->end(); ++j) {
				Particle &mj = (*j);

				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();

				// Compute force exerted between the two bodies
				pVector acc(mj.vel * (magdt / (tohimlenSqr + epsilon)));

				m.vel += acc;
				mj.vel -= acc;
			}
		}
	}
}

// Match Rotational velocity to near neighbors
void PAMatchRotVelocity::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;

	if(max_radiusSqr < P_MAXFLOAT) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Add interactions with other particles
			ParticleList::iterator j = it;
			++j;

			// Add interactions with other particles
			for(; j != group->end(); ++j) {
				Particle &mj = (*j);

				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();

				if(tohimlenSqr < max_radiusSqr) {
					// Compute force exerted between the two bodies
					pVector acc(mj.rvel * (magdt / (tohimlenSqr + epsilon)));

					m.rvel += acc;
					mj.rvel -= acc;
				}
			}
		}
	} else {
		// If not using radius cutoff, avoid the if().
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Add interactions with other particles
			ParticleList::iterator j = it;
			++j;

			// Add interactions with other particles
			for(; j != group->end(); ++j) {
				Particle &mj = (*j);

				pVector tohim(mj.pos - m.pos); // tohim = p1 - p0
				float tohimlenSqr = tohim.length2();

				// Compute force exerted between the two bodies
				pVector acc(mj.rvel * (magdt / (tohimlenSqr + epsilon)));

				m.rvel += acc;
				mj.rvel -= acc;
			}
		}
	}
}

void PAMove::Execute(ParticleGroup *group)
{
	// Step particle positions forward by dt, and age the particles.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		m.age += dt;
		m.pos += m.vel * dt;
		m.up += m.rvel * dt;
	}
}

// Accelerate particles towards a line
void PAOrbitLine::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;

	if(max_radiusSqr < P_MAXFLOAT) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Figure direction to particle from base of line.
			pVector f = m.pos - p;

			// Projection of particle onto line
			pVector w = axis * (f * axis);

			// Direction from particle to nearest point on line.
			pVector into = w - f;

			// Distance to line (force drops as 1/r^2, normalize by 1/r)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = into.length2();

			if(rSqr < max_radiusSqr)
				// Step velocity with acceleration
				m.vel += into * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
		}
	} else {
		// If not using radius cutoff, avoid the if().
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Figure direction to particle from base of line.
			pVector f = m.pos - p;

			// Projection of particle onto line
			pVector w = axis * (f * axis);

			// Direction from particle to nearest point on line.
			pVector into = w - f;

			// Distance to line (force drops as 1/r^2, normalize by 1/r)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = into.length2();

			// Step velocity with acceleration
			m.vel += into * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
		}
	}
}

// Accelerate particles towards a point
void PAOrbitPoint::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = max_radius * max_radius;

	if(max_radiusSqr < P_MAXFLOAT) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Figure direction to particle.
			pVector dir(center - m.pos);

			// Distance to gravity well (force drops as 1/r^2, normalize by 1/r)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = dir.length2();

			// Step velocity with acceleration
			if(rSqr < max_radiusSqr)
				m.vel += dir * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
		}
	} else {
		// If not using radius cutoff, avoid the if().
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Figure direction to particle.
			pVector dir(center - m.pos);

			// Distance to gravity well (force drops as 1/r^2, normalize by 1/r)
			// Soften by epsilon to avoid tight encounters to infinity
			float rSqr = dir.length2();

			// Step velocity with acceleration
			m.vel += dir * (magdt / (sqrtf(rSqr) * (rSqr + epsilon)));
		}
	}
}

// Accelerate in random direction each time step
void PARandomAccel::Execute(ParticleGroup *group)
{
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		pVector acceleration;
		gen_acc.Generate(acceleration);

		// dt will affect this by making a higher probability of
		// being near the original velocity after unit time. Smaller
		// dt approach a normal distribution instead of a square wave.
		m.vel += acceleration * dt;
	}
}

// Immediately displace position randomly
void PARandomDisplace::Execute(ParticleGroup *group)
{
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		pVector displacement;
		gen_disp.Generate(displacement);

		// dt will affect this by making a higher probability of
		// being near the original position after unit time. Smaller
		// dt approach a normal distribution instead of a square wave.
		m.pos += displacement * dt;
	}
}

// Immediately assign a random velocity
void PARandomVelocity::Execute(ParticleGroup *group)
{
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		pVector velocity;
		gen_vel.Generate(velocity);

		// Shouldn't multiply by dt because velocities are
		// invariant of dt. How should dt affect this?
		m.vel = velocity;
	}
}

// Immediately assign a random rotational velocity
void PARandomRotVelocity::Execute(ParticleGroup *group)
{
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		pVector velocity;
		gen_vel.Generate(velocity);

		// Shouldn't multiply by dt because velocities are
		// invariant of dt. How should dt affect this?
		m.rvel = velocity;
	}
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
// _pconstrain(m.pos.x, m.vel.x, m.posB.x, 0., timeLeft, &a, &b, &c);

// Figure new velocity at next timestep
// m.vel.x = a * dtSqr + b * dt + c;
#endif

// Figure new velocity at next timestep
static inline void Restore(pVector &vel, const pVector &posB, const pVector &pos, const float t,
						   const float dtSqr, const float ttInv6dt, const float tttInv3dtSqr)
{
	pVector b = (vel*-0.6667f*t + posB - pos) * ttInv6dt;
	pVector a = (vel*t - posB - posB + pos + pos) * tttInv3dtSqr;
	vel += a + b;
}

// Over time, restore particles to initial positions
// Put all particles on the surface of a statue, explode the statue,
// and then suck the particles back to the original position. Cool!
void PARestore::Execute(ParticleGroup *group)
{
	if(time_left <= 0) {
		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			// Already constrained; keep it there.
			m.pos = m.posB;
			m.vel = pVector(0.0f,0.0f,0.0f);
			m.rvel = pVector(0.0f,0.0f,0.0f);
			m.up = m.upB;
		}
	} else {
		float t = time_left;
		float dtSqr = fsqr(dt);
		float ttInv6dt = dt * 6.0f / fsqr(t);
		float tttInv3dtSqr = dtSqr * 3.0f / (t * t * t);

		for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
			Particle &m = (*it);

			if (restore_velocity)
				Restore(m.vel, m.posB, m.pos, t, dtSqr, ttInv6dt, tttInv3dtSqr);
			if (restore_rvelocity)
				Restore(m.rvel, m.upB, m.up, t, dtSqr, ttInv6dt, tttInv3dtSqr);
		}
	}

	time_left -= dt;
}

// Kill particles with positions on wrong side of the specified domain
void PASink::Execute(ParticleGroup *group)
{
	// Must traverse list in carefully so Remove will work
	for (ParticleList::iterator it = group->begin(); it != group->end(); ) {
		Particle &m = (*it);

		// Remove if inside/outside flag matches object's flag
		if(!(position.Within(m.pos) ^ kill_inside))
			group->Remove(it);
		else
			++it;
	}
}

// Kill particles with velocities on wrong side of the specified domain
void PASinkVelocity::Execute(ParticleGroup *group)
{
	// Must traverse list carefully so Remove will work
	for (ParticleList::iterator it = group->begin(); it != group->end(); ) {
		Particle &m = (*it);

		// Remove if inside/outside flag matches object's flag
		if(!(velocity.Within(m.vel) ^ kill_inside))
			group->Remove(it);
		else
			++it;
	}
}

// Sort the particles by their projection onto the Look vector
void PASort::Execute(ParticleGroup *group)
{
	// First compute projection of particle onto view vector
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		pVector ToP = m.pos - Eye;
		m.tmp0 = ToP * Look;
	}

	// sort<ParticleList::iterator>(group->begin(), group->end());
	Particle *beg = &*group->begin();
	Particle *end = &*group->end();
	std::sort<Particle *>(beg, end);
	// sort<Particle *>(&*group->begin(), &*group->end());
}

// Randomly add particles to the system
void PASource::Execute(ParticleGroup *group)
{
	size_t rate = size_t(floor(particle_rate * dt));

	// Dither the fractional particle in time.
	if(pRandf() < particle_rate * dt - float(rate))
		rate++;

	// Don't emit more than it can hold.
	if(group->size() + rate > group->GetMaxParticles())
		rate = group->GetMaxParticles() - group->size();

	if(vertexB_tracks) {
		for(size_t i = 0; i < rate; i++) {
			pVector pos, up, vel, rvel, siz, col, al;

			position.Generate(pos);
			size.Generate(siz);
			upVector.Generate(up);
			velocity.Generate(vel);
			rvelocity.Generate(rvel);
			color.Generate(col);
			alpha.Generate(al);
			float ag = age + NRand(age_sigma);

			group->Add(pos, pos, up, vel, rvel, siz, col, al.x, ag);
		}
	} else {
		for(size_t i = 0; i < rate; i++) {
			pVector pos, posB, up, vel, rvel, siz, col, al;

			position.Generate(pos);
			positionB.Generate(posB);
			size.Generate(siz);
			upVector.Generate(up);
			velocity.Generate(vel);
			rvelocity.Generate(rvel);
			color.Generate(col);
			alpha.Generate(al);
			float ag = age + NRand(age_sigma);

			group->Add(pos, posB, up, vel, rvel, siz, col, al.x, ag);
		}
	}
}

void PASpeedLimit::Execute(ParticleGroup *group)
{
	float min_sqr = min_speed*min_speed;
	float max_sqr = max_speed*max_speed;

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		float sSqr = m.vel.length2();
		if(sSqr<min_sqr && sSqr) {
			float s = sqrtf(sSqr);
			m.vel *= (min_speed/s);
		} else if(sSqr>max_sqr) {
			float s = sqrtf(sSqr);
			m.vel *= (max_speed/s);
		}
	}
}

// Change color of all particles toward the specified color
void PATargetColor::Execute(ParticleGroup *group)
{
	float scaleFac = scale * dt;

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		m.color += (color - m.color) * scaleFac;
		m.alpha += (alpha - m.alpha) * scaleFac;
	}
}

// Change sizes of all particles toward the specified size
void PATargetSize::Execute(ParticleGroup *group)
{
	float scaleFac_x = scale.x * dt;
	float scaleFac_y = scale.y * dt;
	float scaleFac_z = scale.z * dt;

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		pVector dif(size - m.size);
		dif.x *= scaleFac_x;
		dif.y *= scaleFac_y;
		dif.z *= scaleFac_z;
		m.size += dif;
	}
}

// Change velocity of all particles toward the specified velocity
void PATargetVelocity::Execute(ParticleGroup *group)
{
	float scaleFac = scale * dt;

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		m.vel += (velocity - m.vel) * scaleFac;
	}
}

// Change velocity of all particles toward the specified velocity
void PATargetRotVelocity::Execute(ParticleGroup *group)
{
	float scaleFac = scale * dt;

	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);
		m.rvel += (velocity - m.rvel) * scaleFac;
	}
}

// Immediately displace position using vortex
// Vortex tip at center, around axis, with magnitude
// and tightness exponent

// magnitude is how much acceleration to apply at the top of the vortex (non-tip end)
// tightnessExponent is like a phong exponent that gives a curve to the vortex silhouette
// axis is the vector along the center of the vortex starting at p
// p is the tip of the vortex
void PAVortex::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;
	float max_radiusSqr = fsqr(max_radius);
	float axisLength = axis.length();
	float axisLengthInv = 1.0f / axisLength;
	pVector axisN = axis;
	axisN.normalize();

	// This one just rotates a particle around the axis. Amount is based on radius, magnitude, and mass.
	for (ParticleList::iterator it = group->begin(); it != group->end(); ++it) {
		Particle &m = (*it);

		// Figure direction to particle from base of line.
		pVector tipToPar = m.pos - tip;

		// Projection of particle onto line
		float axisScale = tipToPar * axisN;
		pVector parOnAxis = axisN * axisScale;

		// Direction from particle to nearest point on line.
		pVector parToAxis = parOnAxis - tipToPar;

		// Distance to axis
		float rSqr = parToAxis.length2();
		float alongAxis = axisScale * axisLengthInv;

		// This is how much to scale the vortex's force by as a function of how far up the axis the particle is.
		float alongAxisPow = powf(alongAxis, tightnessExponent);
		float silhouetteSqr = fsqr(alongAxisPow * max_radius);

		if(rSqr >= max_radiusSqr || axisScale < 0.0f || alongAxis > 1.0f) {
			// m.color = pVector(0,0,1);
			continue;
		}
		pVector AccelUp = axisN * 0.011f * dt; // XXX Make this a param.
		m.vel += AccelUp;

		if(rSqr >= silhouetteSqr) {
			// m.color = pVector(1,0,0);
			continue;
		}

		// m.color = pVector(0,1,0);
		AccelUp = axisN * -0.001f * dt; // XXX Make this a param.
		m.vel += AccelUp;

		// Apply tightness
		float r = sqrtf(rSqr);

		// Accelerate toward axis. Force drops as 1/r^2, normalize by 1/r.
		// Soften by epsilon to avoid tight encounters to infinity
		pVector AccelIn = parToAxis * ((1.0f - alongAxisPow) * magdt / (m.mass * (r * (rSqr + epsilon))));
		m.vel += AccelIn;

		// Accelerate around axis by constructing orthogonal vector frame of axis, parToAxis, and RotAccel.
		pVector RotAccel = Cross(axisN, parToAxis);
		float RA = RotAccel.length();
		float scale = rotSpeed / RA;
		RotAccel *= scale;

		pVector dst = RotAccel - parToAxis;
		float DA = dst.length();
		dst *= (r / DA);
		pVector travel = dst + parToAxis;

		pVector AccelAround = travel * (dt / m.mass);
		m.vel += AccelAround;
	}
}

// This is an experimental Action.
// It's mostly for the purpose of seeing how big the speedup can be
// if we apply all actions to a particle at once,
// rather than doing an action to all particles at once.
void PAFountain::Execute(ParticleGroup *group)
{
	//pCopyVertexB(false, true);
	//pGravity(0.0, 0.0, -0.01);
	//pSinkVelocity(true, PDSphere, 0, 0, 0, 0.01);
	//pBounce(-0.05, 0.35, 0, PDDisc, 0, 0, 0,  0, 0, 1,  5);
	//pSink(false, PDPlane, 0,0,-3, 0,0,1);
	//pMove();

	// For pGravity
	PAGravity *PGr = (PAGravity *)(AL+1);
	pVector ddir(PGr->direction * dt);

	PASinkVelocity *PSV = (PASinkVelocity *)(AL+2);

	// For bounce disc
	PABounce *PB = (PABounce *)(AL+3);
	float r1Sqr = fsqr(PB->position.radius1);
	float r2Sqr = fsqr(PB->position.radius2);

	// For sink
	PASink*PS = (PASink*)(AL+4);

	// Must traverse list carefully so Remove will work
	for (ParticleList::iterator it = group->begin(); it != group->end(); ) {
		Particle &m = (*it);

		//pCopyVertexB(false, true);
		m.velB = m.vel;

		//pGravity(0.0, 0.0, -0.01);
		// Step velocity with acceleration
		m.vel += ddir;

		//pSinkVelocity(true, PDSphere, 0, 0, 0, 0.01);
		// Remove if inside/outside flag matches object's flag
		if(!(PSV->velocity.Within(m.vel) ^ PSV->kill_inside)) {
			group->Remove(it);
			continue;
		}

		//pBounce(-0.05, 0.35, 0, PDDisc, 0, 0, 0,  0, 0, 1,  5);

		// See if particle's current and next positions cross plane.
		// If not, couldn't bounce, so keep going.
		pVector pnext(m.pos + m.vel * dt);

		// p2 stores the plane normal (the a,b,c of the plane eqn).
		// Old and new distances: dist(p,plane) = n * p + d
		// radius1 stores -n*p, which is d. radius1Sqr stores d.
		float distold = m.pos * PB->position.p2 + PB->position.radius1Sqr;
		float distnew = pnext * PB->position.p2 + PB->position.radius1Sqr;

		// Opposite signs if product < 0
		// Is there a faster way to do this?
		if(distold * distnew < 0) {
			// Find position at the crossing point by parameterizing
			// p(t) = pos + vel * t
			// Solve dist(p(t),plane) = 0 e.g.
			// n * p(t) + D = 0 ->
			// n * p + t (n * v) + D = 0 ->
			// t = -(n * p + D) / (n * v)
			// Could factor n*v into distnew = distold + n*v and save a bit.
			// Safe since n*v != 0 assured by quick rejection test.
			// This calc is indep. of dt because we have established that it
			// will hit before dt. We just want to know when.
			float nv = PB->position.p2 * m.vel;
			float t = -distold / nv;

			// Actual intersection point p(t) = pos + vel t
			pVector phit(m.pos + m.vel * t);

			// Offset from origin in plane, phit - origin
			pVector offset(phit - PB->position.p1);

			float rad = offset.length2();

			if(!(rad > r1Sqr || rad < r2Sqr)) {
				// A hit! A most palpable hit!

				// Compute tangential and normal components of velocity
				pVector vn(PB->position.p2 * nv); // Normal Vn = (V.N)N
				pVector vt(m.vel - vn); // Tangent Vt = V - Vn

				// Compute new velocity heading out:
				// Don't apply friction if tangential velocity < cutoff
				if(vt.length2() <= PB->cutoffSqr)
					m.vel = vt - vn * PB->resilience;
				else
					m.vel = vt * PB->oneMinusFriction - vn * PB->resilience;
			}
		}

		//pSink(false, PDPlane, 0,0,-3, 0,0,1);
		// Remove if inside/outside flag matches object's flag
		if(!(PS->position.Within(m.pos) ^ PS->kill_inside)) {
			group->Remove(it);
			continue;
		} else
			++it;

		//pMove();
		m.age += dt;
		m.pos += m.vel * dt;
	}
}
