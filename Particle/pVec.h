/// pVec.h - yet another 3D vector class.
///
/// Copyright 1997-2007 by David K. McAllister
/// Based on code Copyright 1997 by Jonathan P. Leech
/// http://www.ParticleSystems.org
///
/// A simple 3D float vector class for internal use by the particle systems.

#ifndef pvec_h
#define pvec_h

#ifndef __CUDACC__
#include <iostream>
#endif
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433f
#endif

#ifdef __CUDACC__
//#define PINLINE __device__ inline
#define PINLINE __device__ __host__ inline
#else
#ifdef WIN32
// This is because their stupid compiler thinks it's smart.
#define PINLINE __forceinline
#else
#define PINLINE inline
#endif
#endif

namespace PAPI
{
    // typedef unsigned long long pdata_t;
    typedef unsigned int pdata_t; // Making it 32 bit so Particle_t is 128B.

    // const float P_SQRT2PI = 2.506628274631000502415765284811045253006f;
    // const float P_ONEOVERSQRT2PI = (1.f / P_SQRT2PI);
#define P_SQRT2PI 2.506628274631000502415765284811045253006f
#define P_ONEOVERSQRT2PI (1.f / P_SQRT2PI)

    ///< These are magic numbers that tell the action list compiler to get the value at runtime instead of action list compilation time.
#define P_VARYING_FLOAT -340282326356119256160033759537265639424.0f
#define P_VARYING_INT   0x7fffffce
#define P_VARYING_BOOL  (bool)0xce

    PINLINE float fsqr(float f) { return f * f; }

#ifdef __CUDACC__
    PINLINE float pRandf() { return 0.34514f; } // XXX Need a CUDA rand
    PINLINE void pSRandf(int x) {  } // XXX
#else
#ifdef unix
    PINLINE float pRandf() { return drand48(); }
    PINLINE void pSRandf(int x) { srand48(x); }
#else
    const float P_ONEOVER_RAND_MAX = (1.0f/((float) RAND_MAX));
    PINLINE float pRandf() { return ((float) rand())*P_ONEOVER_RAND_MAX; }
    PINLINE void pSRandf(int x) { srand(x); }
#endif
#endif

    PINLINE bool pSameSign(const float &a, const float &b) { return a * b >= 0.0f; }

    /// Return a random number with a normal distribution.
    PINLINE float pNRandf(float sigma = 1.0f)
    {
        float x, y, r2;
        do {
            x = pRandf()*2.0f-1.0f;
            y = pRandf()*2.0f-1.0f;
            r2 = x*x+y*y;
        }
        while(r2 > 1.0f || r2 == 0.0f);

        float m = sqrtf(-2.0f * logf(r2)/r2);

        float px = x*m*sigma;
        // float py = y*m*sigma;

        return px;
    }

    // Declare here. Define below.
    class pVec;
    PINLINE pVec pVec_(const float a);
    PINLINE pVec pVec_(const float ax, const float ay, const float az);

    /// A single-precision floating point three-vector.
    ///
    /// This class is used for packaging three floats for the application to pass into the API.
    ///
    /// This is also the class used internally to do vector math.
    class pVec
    {
        float vx, vy, vz;

    public:
#if 0
        PINLINE pVec_() {}
        PINLINE pVec_(float ax, float ay, float az) : vx(ax), vy(ay), vz(az) {}
        PINLINE pVec_(float a) : vx(a), vy(a), vz(a) {}
#endif

        PINLINE const float& x() const { return vx; }
        PINLINE const float& y() const { return vy; }
        PINLINE const float& z() const { return vz; }

        PINLINE float& x() { return vx; }
        PINLINE float& y() { return vy; }
        PINLINE float& z() { return vz; }

        PINLINE float length() const
        {
            return sqrtf(vx*vx+vy*vy+vz*vz);
        }

        PINLINE float length2() const
        {
            return (vx*vx+vy*vy+vz*vz);
        }

        PINLINE float normalize()
        {
            float onel = 1.0f / sqrtf(vx*vx+vy*vy+vz*vz);
            vx *= onel;
            vy *= onel;
            vz *= onel;

            return onel;
        }

        // Dot product
        friend PINLINE float dot(const pVec &a, const pVec &b)
        {
            return b.x()*a.x() + b.y()*a.y() + b.z()*a.z();
        }

        // Scalar multiply
        PINLINE pVec operator*(const float s) const
        {
            return pVec_(vx*s, vy*s, vz*s);
        }

        PINLINE pVec operator/(const float s) const
        {
            float invs = 1.0f / s;
            return pVec_(vx*invs, vy*invs, vz*invs);
        }

        PINLINE pVec operator+(const pVec& a) const
        {
            return pVec_(vx+a.x(), vy+a.y(), vz+a.z());
        }

        PINLINE pVec operator-(const pVec& a) const
        {
            return pVec_(vx-a.x(), vy-a.y(), vz-a.z());
        }

        PINLINE bool operator==(const pVec &a) const
        {
            return vx==a.x() && vy==a.y() && vz==a.z();
        }

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

#if 0
        PINLINE pVec& operator=(const pVec& a)
        {
            vx = a.x();
            vy = a.y();
            vz = a.z();
            return *this;
        }
#endif

        // Component-wise absolute value
        friend PINLINE pVec Abs(const pVec &a)
        {
            return pVec_(fabs(a.x()), fabs(a.y()), fabs(a.z()));
        }

        // Component-wise multiply
        friend PINLINE pVec CompMult(const pVec &a, const pVec& b)
        {
            return pVec_(b.x()*a.x(), b.y()*a.y(), b.z()*a.z());
        }

        friend PINLINE pVec Cross(const pVec& a, const pVec& b)
        {
            return pVec_(
                a.y()*b.z()-a.z()*b.y(),
                a.z()*b.x()-a.x()*b.z(),
                a.x()*b.y()-a.y()*b.x());
        }

#ifndef __CUDACC__
        friend PINLINE std::ostream& operator<<(std::ostream& os, const pVec& v)
        {
            os << &v << '[' << v.x() << ", " << v.y() << ", " << v.z() << ']';

            return os;
        }
#endif
    };

#if 1
    // Create a pVec from a single float, replicated across the channels
    // I do this to avoid having constructors an
    PINLINE pVec pVec_(const float a)
    {
        pVec p;
        p.x() = p.y() = p.z() = a;
        return p;
    }

    // Create a pVec from three floats
    PINLINE pVec pVec_(const float ax, const float ay, const float az)
    {
        pVec p;
        p.x() = ax;
        p.y() = ay;
        p.z() = az;
        return p;
    }

    PINLINE pVec pRandVec()
    {
        return pVec_(pRandf(), pRandf(), pRandf());
    }

    PINLINE pVec pNRandVec(float sigma)
    {
        float x, y, r2;
        do {
            x = pRandf()*2.0f-1.0f;
            y = pRandf()*2.0f-1.0f;
            r2 = x*x+y*y;
        }
        while(r2 > 1.0f || r2 == 0.0f);

        float m = sqrtf(-2.0f * logf(r2)/r2);

        float px = x*m*sigma;
        float py = y*m*sigma;
        return pVec_(px, py, pNRandf(sigma));
    }
#endif
};

#endif
