#pragma once

#include "papi.h"
#include "pVector.h"

class pDomain
{
public:
	PDomainEnum type;	// PABoxDomain, PASphereDomain, PAConeDomain...
	pVector p1, p2;		// Box vertices, Sphere center, Cylinder/Cone ends
	pVector u, v;		// Orthonormal basis vectors for Cylinder/Cone
	float radius1;		// Outer radius
	float radius2;		// Inner radius
	float radius1Sqr;	// Used for fast Within test of spheres,
	float radius2Sqr;	// and for mag. of u and v vectors for plane.

	bool Within(const pVector &) const;
	void Generate(pVector &) const;

	// This constructor is used when default constructing a
	// PABaseClass that has a pDomain.
	inline pDomain(void) {}

	// Construct a domain in the standard way.
	pDomain(PDomainEnum dtype,
		float a0=0.0f, float a1=0.0f, float a2=0.0f,
		float a3=0.0f, float a4=0.0f, float a5=0.0f,
		float a6=0.0f, float a7=0.0f, float a8=0.0f);

	~pDomain(void) {}
};
