// pVector.h - yet another 3D vector class.
//
// Copyright 1997 by Jonathan P. Leech
// Modifications Copyright 1997-2005 by David K. McAllister
//
// A simple 3D float vector class for internal use by the particle systems.

#ifndef particle_vector_h
#define particle_vector_h

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433f
#endif

#ifdef unix
#define pRandf() drand48()
#define pSRandf(x) srand48(x)
#else
#include <stdlib.h>
#define pRandf() (((float) rand())/((float) RAND_MAX))
#define pSRandf(x) srand(x)
#endif

#define P_SQRT2PI 2.506628274631000502415765284811045253006f
#define ONEOVERSQRT2PI (1.f / P_SQRT2PI)

#ifdef WIN32
// This is because their stupid compiler thinks it's smart.
#define inline __forceinline
#endif

static inline float fsqr(float f) { return f * f; }

// Return a random number with a normal distribution.
static inline float NRand(float sigma = 1.0f)
{
#define ONE_OVER_SIGMA_EXP (1.0f / 0.7975f)

	if(sigma == 0) return 0;

	float y;
	do {
		y = -logf(pRandf());
	}
	while(pRandf() > expf(-fsqr(y - 1.0f)*0.5f));

	if(rand() & 0x1)
		return y * sigma * ONE_OVER_SIGMA_EXP;
	else
		return -y * sigma * ONE_OVER_SIGMA_EXP;
}

class pVector
{
public:
	float x, y, z;

	inline pVector(float ax, float ay, float az) : x(ax), y(ay), z(az) {}

	inline pVector() {}

	inline float length() const
	{
		return sqrtf(x*x+y*y+z*z);
	}

	inline float length2() const
	{
		return (x*x+y*y+z*z);
	}

	inline float normalize()
	{
		float onel = 1.0f / sqrtf(x*x+y*y+z*z);
		x *= onel;
		y *= onel;
		z *= onel;

		return onel;
	}

	inline float operator*(const pVector &a) const
	{
		return x*a.x + y*a.y + z*a.z;
	}

	inline pVector operator*(const float s) const
	{
		return pVector(x*s, y*s, z*s);
	}

	inline pVector operator/(const float s) const
	{
		float invs = 1.0f / s;
		return pVector(x*invs, y*invs, z*invs);
	}

	inline pVector operator+(const pVector& a) const
	{
		return pVector(x+a.x, y+a.y, z+a.z);
	}

	inline pVector operator-(const pVector& a) const
	{
		return pVector(x-a.x, y-a.y, z-a.z);
	}

	inline pVector operator-()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}

	inline pVector& operator+=(const pVector& a)
	{
		x += a.x;
		y += a.y;
		z += a.z;
		return *this;
	}

	inline pVector& operator-=(const pVector& a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
		return *this;
	}

	inline pVector& operator*=(const float a)
	{
		x *= a;
		y *= a;
		z *= a;
		return *this;
	}

	inline pVector& operator/=(const float a)
	{
		float b = 1.0f / a;
		x *= b;
		y *= b;
		z *= b;
		return *this;
	}

	inline pVector& operator=(const pVector& a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
		return *this;
	}
};
    
	inline pVector Cross(const pVector& a, const pVector& b)
	{
		return pVector(
			a.y*b.z-a.z*b.y,
			a.z*b.x-a.x*b.z,
			a.x*b.y-a.y*b.x);
	}

// To offset [0 .. 1] vectors to [-.5 .. .5]
static pVector vHalf(0.5, 0.5, 0.5);

static inline pVector RandVec()
{
	return pVector(pRandf(), pRandf(), pRandf());
}

#endif
