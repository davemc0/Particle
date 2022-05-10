/// PVec.h - yet another 3D vector class
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// A simple 3D float vector class for internal use by the particle systems.

#ifndef pvec_h
#define pvec_h

#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433f
#endif

#ifdef __CUDACC__
#include <curand.h>
#include <curand_kernel.h>

#define PCONSTANT __constant__ const
#ifdef __CUDA_ARCH__
#define PINLINE __host__ __device__
#define PINLINEH __host__
#define PINLINEX
#else
#define PINLINE __forceinline __host__ __device__
#define PINLINEH __forceinline __host__
#define PINLINEX __forceinline
#endif
#else
#define PCONSTANT const
#ifdef WIN32
#define PINLINE __forceinline
#define PINLINEH __forceinline
#define PINLINEX __forceinline
#else
#define PINLINE inline
#define PINLINEH inline
#define PINLINEX inline
#endif
#endif

namespace PAPI {
PCONSTANT float P_SQRT2PI = 2.506628274631000502415765284811045253006f;
PCONSTANT float constexpr P_ONEOVERSQRT2PI = (1.f / 2.506628274631000502415765284811045253006f);

PINLINE float fsqr(float f) { return f * f; }

#ifdef unix
PINLINE float pRandf() { return drand48(); }
PINLINE void pSRandf(int x) { srand48(x); }
#else
#ifdef __CUDA_ARCH__
PINLINE float pRandf(curandState& st) { return curand_uniform(&st); }
PINLINE curandState pSRandf(int x)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    curand_init(clock64(), i, 0, &st);
}
#else
const float P_ONEOVER_RAND_MAX = (1.0f / ((float)RAND_MAX));
PINLINEH float pRandf() { return ((float)rand()) * P_ONEOVER_RAND_MAX; }
PINLINEH void pSRandf(int x) { srand(x); }
#endif
#endif

PINLINE bool pSameSign(const float& a, const float& b) { return a * b >= 0.0f; }

/// Return a random number with a normal distribution.
PINLINE float pNRandf(float sigma = 1.0f)
{
    float x, y, r2;
    do {
        x = pRandf() * 2.0f - 1.0f;
        y = pRandf() * 2.0f - 1.0f;
        r2 = x * x + y * y;
    } while (r2 > 1.0f || r2 == 0.0f);

    float m = sqrtf(-2.0f * logf(r2) / r2);

    float px = x * m * sigma;
    // float py = y*m*sigma;

    return px;
}

/// A single-precision floating point three-vector.
///
/// This class is used for packaging three floats for the application to pass into the API.
///
/// This is also the class used internally to do vector math.
class pVec {
    float vx, vy, vz;

public:
    PINLINE pVec(float ax, float ay, float az) : vx(ax), vy(ay), vz(az) {}
    PINLINE pVec(float a) : vx(a), vy(a), vz(a) {}
    PINLINEX pVec() = default;
    PINLINE pVec(float* v) : vx(v[0]), vy(v[1]), vz(v[2]) {}

    PINLINE const float& x() const { return vx; }
    PINLINE const float& y() const { return vy; }
    PINLINE const float& z() const { return vz; }

    PINLINE float& x() { return vx; }
    PINLINE float& y() { return vy; }
    PINLINE float& z() { return vz; }

    PINLINE float length() const { return sqrtf(vx * vx + vy * vy + vz * vz); }

    PINLINE float lenSqr() const { return (vx * vx + vy * vy + vz * vz); }

    PINLINE float normalize(const float toLen = 1.f)
    {
        float onel = toLen / sqrtf(vx * vx + vy * vy + vz * vz);
        vx *= onel;
        vy *= onel;
        vz *= onel;

        return onel;
    }

    // Dot product
    friend PINLINE float dot(const pVec& a, const pVec& b) { return b.x() * a.x() + b.y() * a.y() + b.z() * a.z(); }

    // Scalar multiply
    PINLINE pVec operator*(const float s) const { return pVec(vx * s, vy * s, vz * s); }

    PINLINE pVec operator/(const float s) const
    {
        float invs = 1.0f / s;
        return pVec(vx * invs, vy * invs, vz * invs);
    }

    PINLINE pVec operator+(const pVec& a) const { return pVec(vx + a.x(), vy + a.y(), vz + a.z()); }

    PINLINE pVec operator-(const pVec& a) const { return pVec(vx - a.x(), vy - a.y(), vz - a.z()); }

    PINLINE bool operator==(const pVec& a) const { return vx == a.x() && vy == a.y() && vz == a.z(); }

    PINLINE pVec operator-()
    {
        vx = -vx;
        vy = -vy;
        vz = -vz;
        return *this;
    }

    PINLINE pVec& operator+=(const pVec& a)
    {
        vx += a.x();
        vy += a.y();
        vz += a.z();
        return *this;
    }

    PINLINE pVec& operator-=(const pVec& a)
    {
        vx -= a.x();
        vy -= a.y();
        vz -= a.z();
        return *this;
    }

    PINLINE pVec& operator*=(const float a)
    {
        vx *= a;
        vy *= a;
        vz *= a;
        return *this;
    }

    PINLINE pVec& operator/=(const float a)
    {
        float b = 1.0f / a;
        vx *= b;
        vy *= b;
        vz *= b;
        return *this;
    }

    PINLINEH bool isNan() { return std::isnan(x()) || std::isnan(y()) || std::isnan(z()); }

    // Component-wise absolute value
    friend PINLINE pVec Abs(const pVec& a) { return pVec(fabs(a.x()), fabs(a.y()), fabs(a.z())); }

    // Component-wise multiply
    friend PINLINE pVec CompMult(const pVec& a, const pVec& b) { return pVec(b.x() * a.x(), b.y() * a.y(), b.z() * a.z()); }

    friend PINLINE pVec Cross(const pVec& a, const pVec& b)
    {
        return pVec(a.y() * b.z() - a.z() * b.y(), a.z() * b.x() - a.x() * b.z(), a.x() * b.y() - a.y() * b.x());
    }

    friend PINLINEH std::ostream& operator<<(std::ostream& os, const pVec& v)
    {
        os << &v << '[' << v.x() << ", " << v.y() << ", " << v.z() << ']';

        return os;
    }
};

PINLINE pVec pRandVec() { return pVec(pRandf(), pRandf(), pRandf()); }

PINLINE pVec pNRandVec(float sigma)
{
    float x, y, r2;
    do {
        x = pRandf() * 2.0f - 1.0f;
        y = pRandf() * 2.0f - 1.0f;
        r2 = x * x + y * y;
    } while (r2 > 1.0f || r2 == 0.0f);

    float m = sqrtf(-2.0f * logf(r2) / r2);

    float px = x * m * sigma;
    float py = y * m * sigma;
    return pVec(px, py, pNRandf(sigma));
}
}; // namespace PAPI

#endif
