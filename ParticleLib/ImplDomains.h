/// ImplDomains.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// This file contains inline implementations of the domain functions for use in a single-particle API, such as for CUDA.

#ifndef impldomains_h
#define impldomains_h

#include "Particle/pDomain.h"

namespace PAPI {

union Dword_t {
    float f;
    int i;
    pdata_t d;
    bool b;
};

// Return a pDomain of the specified variety based on its INTERNAL VARIABLES
// These are used when no variable in the domain's input parameters was VARYING,
// so the construction process doesn't need to be repeated for each call to the kernel.

#if 1
// This should be a generic way of doing it that will work on all domains (except PDUnion).
PINLINE pDomain PDData_(PAPI::pDomainType_E Wh, unsigned int v00, unsigned int v01, unsigned int v02, unsigned int v03, unsigned int v04, unsigned int v05,
                        unsigned int v06, unsigned int v07, unsigned int v08, unsigned int v09, unsigned int v0a, unsigned int v0b, unsigned int v0c,
                        unsigned int v0d, unsigned int v0e, unsigned int v0f, unsigned int v10, unsigned int v11, unsigned int v12, unsigned int v13,
                        unsigned int v14, unsigned int v15, unsigned int v16, unsigned int v17, unsigned int v18, unsigned int v19, unsigned int v1a,
                        unsigned int v1b, unsigned int v1c, unsigned int v1d)
{
    pDomain D;
    D.Which = Wh;
    D.Varying = false;
    D.PDData_V[0x00] = v00;
    D.PDData_V[0x01] = v01;
    D.PDData_V[0x02] = v02;
    D.PDData_V[0x03] = v03;
    D.PDData_V[0x04] = v04;
    D.PDData_V[0x05] = v05;
    D.PDData_V[0x06] = v06;
    D.PDData_V[0x07] = v07;
    D.PDData_V[0x08] = v08;
    D.PDData_V[0x09] = v09;
    D.PDData_V[0x0a] = v0a;
    D.PDData_V[0x0b] = v0b;
    D.PDData_V[0x0c] = v0c;
    D.PDData_V[0x0d] = v0d;
    D.PDData_V[0x0e] = v0e;
    D.PDData_V[0x0f] = v0f;
    D.PDData_V[0x10] = v10;
    D.PDData_V[0x11] = v11;
    D.PDData_V[0x12] = v12;
    D.PDData_V[0x13] = v13;
    D.PDData_V[0x14] = v14;
    D.PDData_V[0x15] = v15;
    D.PDData_V[0x16] = v16;
    D.PDData_V[0x17] = v17;
    D.PDData_V[0x18] = v18;
    D.PDData_V[0x19] = v19;
    D.PDData_V[0x1a] = v1a;
    D.PDData_V[0x1b] = v1b;
    D.PDData_V[0x1c] = v1c;
    D.PDData_V[0x1d] = v1d;
    return D;
}

// Create any kind of domain from its input values
PINLINE pDomain PDFromVarying_(const PAPI::pDomainType_E Wh, const pVec& v0, const pVec& v1, const pVec& v2, const float f0, const float f1)
{
    switch (Wh) {
    case PDUnion_e: return PDUnion_();
    case PDPoint_e: return PDPoint_(v0);
    case PDLine_e: return PDLine_(v0, v1);
    case PDTriangle_e: return PDTriangle_(v0, v1, v2);
    case PDRectangle_e: return PDRectangle_(v0, v1, v2);
    case PDDisc_e: return PDDisc_(v0, v1, f0, f1);
    case PDPlane_e: return PDPlane_(v0, v1);
    case PDBox_e: return PDBox_(v0, v1);
    case PDCylinder_e: return PDCylinder_(v0, v1, f0, f1);
    case PDCone_e: return PDCone_(v0, v1, f0, f1);
    case PDSphere_e: return PDSphere_(v0, f0, f1);
    case PDBlob_e:
        return PDBlob_(v0, f0);
        // default: return false;
    };
}

#else

PINLINE pDomain PDUnion_Intl()
{
    pDomain D;
    D.Which = PDUnion_e;
    return D;
}

PINLINE pDomain PDPoint_Intl(const pVec& p_)
{
    pDomain D;
    D.Which = PDPoint_e;
    D.PDPoint_V.p = p_;
    return D;
}

PINLINE pDomain PDLine_Intl(const pVec& p0_, const pVec& vec_, const pVec& vecNrm_, float len_)
{
    pDomain D;
    D.Which = PDLine_e;
    D.PDLine_V.p0 = p0_;
    D.PDLine_V.vec = vec_;
    D.PDLine_V.vecNrm = vecNrm_;
    D.PDLine_V.len = len_;
    return D;
}

PINLINE pDomain PDTriangle_Intl(const pVec& p_, const pVec& u_, const pVec& v_, const pVec& uNrm_, const pVec& vNrm_, const pVec& nrm_, const pVec& s1_,
                                const pVec& s2_, float uLen_, float vLen_, float D_, float area_)
{
    pDomain D;
    D.Which = PDTriangle_e;
    D.PDTriangle_V.p = p_;
    D.PDTriangle_V.u = u_;
    D.PDTriangle_V.v = v_;
    D.PDTriangle_V.uNrm = uNrm_;
    D.PDTriangle_V.vNrm = vNrm_;
    D.PDTriangle_V.nrm = nrm_;
    D.PDTriangle_V.s1 = s1_;
    D.PDTriangle_V.s2 = s2_;
    D.PDTriangle_V.uLen = uLen_;
    D.PDTriangle_V.vLen = vLen_;
    D.PDTriangle_V.D = D_;
    D.PDTriangle_V.area = area_;
    return D;
}

PINLINE pDomain PDRectangle_Intl(const pVec& p_, const pVec& u_, const pVec& v_, const pVec& uNrm_, const pVec& vNrm_, const pVec& nrm_, const pVec& s1_,
                                 const pVec& s2_, float uLen_, float vLen_, float D_, float area_)
{
    pDomain D;
    D.Which = PDRectangle_e;
    D.PDRectangle_V.p = p_;
    D.PDRectangle_V.u = u_;
    D.PDRectangle_V.v = v_;
    D.PDRectangle_V.uNrm = uNrm_;
    D.PDRectangle_V.vNrm = vNrm_;
    D.PDRectangle_V.nrm = nrm_;
    D.PDRectangle_V.s1 = s1_;
    D.PDRectangle_V.s2 = s2_;
    D.PDRectangle_V.uLen = uLen_;
    D.PDRectangle_V.vLen = vLen_;
    D.PDRectangle_V.D = D_;
    D.PDRectangle_V.area = area_;
    return D;
}

PINLINE pDomain PDDisc_Intl(const pVec& p_, const pVec& nrm_, const pVec& u_, const pVec& v_, float radIn_, float radOut_, float radInSqr_, float radOutSqr_,
                            float dif_, float D_, float area_)
{
    pDomain D;
    D.Which = PDDisc_e;
    D.PDDisc_V.p = p_;
    D.PDDisc_V.nrm = nrm_;
    D.PDDisc_V.u = u_;
    D.PDDisc_V.v = v_;
    D.PDDisc_V.radIn = radIn_;
    D.PDDisc_V.radOut = radOut_;
    D.PDDisc_V.radInSqr = radInSqr_;
    D.PDDisc_V.radOutSqr = radOutSqr_;
    D.PDDisc_V.dif = dif_;
    D.PDDisc_V.D = D_;
    D.PDDisc_V.area = area_;
    return D;
}

PINLINE pDomain PDPlane_Intl(const pVec& p_, const pVec& nrm_, float D_)
{
    pDomain D;
    D.Which = PDPlane_e;
    D.PDPlane_V.p = p_;
    D.PDPlane_V.nrm = nrm_;
    D.PDPlane_V.D = D_;
    return D;
}

PINLINE pDomain PDBox_Intl(const pVec& p0_, const pVec& p1_, const pVec& dif_, float vol_)
{
    pDomain D;
    D.Which = PDBox_e;
    D.PDBox_V.p0 = p0_;
    D.PDBox_V.p1 = p1_;
    D.PDBox_V.dif = dif_;
    D.PDBox_V.vol = vol_;
    return D;
}

PINLINE pDomain PDCylinder_Intl(const pVec& apex_, const pVec& axis_, const pVec& u_, const pVec& v_, float radIn_, float radOut_, float radInSqr_,
                                float radOutSqr_, float radDif_, float axisLenInvSqr_, float vol_, bool ThinShell_)
{
    pDomain D;
    D.Which = PDCylinder_e;
    D.PDCylinder_V.apex = apex_;
    D.PDCylinder_V.axis = axis_;
    D.PDCylinder_V.u = u_;
    D.PDCylinder_V.v = v_;
    D.PDCylinder_V.radIn = radIn_;
    D.PDCylinder_V.radOut = radOut_;
    D.PDCylinder_V.radInSqr = radInSqr_;
    D.PDCylinder_V.radOutSqr = radOutSqr_;
    D.PDCylinder_V.radDif = radDif_;
    D.PDCylinder_V.axisLenInvSqr = axisLenInvSqr_;
    D.PDCylinder_V.vol = vol_;
    D.PDCylinder_V.ThinShell = ThinShell_;
    return D;
}

PINLINE pDomain PDCone_Intl(const pVec& apex_, const pVec& axis_, const pVec& u_, const pVec& v_, float radIn_, float radOut_, float radInSqr_,
                            float radOutSqr_, float radDif_, float axisLenInvSqr_, float vol_, bool ThinShell_)
{
    pDomain D;
    D.Which = PDCone_e;
    D.PDCone_V.apex = apex_;
    D.PDCone_V.axis = axis_;
    D.PDCone_V.u = u_;
    D.PDCone_V.v = v_;
    D.PDCone_V.radIn = radIn_;
    D.PDCone_V.radOut = radOut_;
    D.PDCone_V.radInSqr = radInSqr_;
    D.PDCone_V.radOutSqr = radOutSqr_;
    D.PDCone_V.radDif = radDif_;
    D.PDCone_V.axisLenInvSqr = axisLenInvSqr_;
    D.PDCone_V.vol = vol_;
    D.PDCone_V.ThinShell = ThinShell_;
    return D;
}

PINLINE pDomain PDSphere_Intl(const pVec& ctr_, float radIn_, float radOut_, float radInSqr_, float radOutSqr_, float radDif_, float vol_, bool ThinShell_)
{
    pDomain D;
    D.Which = PDSphere_e;
    D.PDSphere_V.ctr = ctr_;
    D.PDSphere_V.radIn = radIn_;
    D.PDSphere_V.radOut = radOut_;
    D.PDSphere_V.radInSqr = radInSqr_;
    D.PDSphere_V.radOutSqr = radOutSqr_;
    D.PDSphere_V.radDif = radDif_;
    D.PDSphere_V.vol = vol_;
    D.PDSphere_V.ThinShell = ThinShell_;
    return D;
}

PINLINE pDomain PDBlob_Intl(const pVec& ctr_, float stdev_, float Scale1_, float Scale2_)
{
    pDomain D;
    D.Which = PDBlob_e;
    D.PDBlob_V.ctr = ctr_;
    D.PDBlob_V.stdev = stdev_;
    D.PDBlob_V.Scale1 = Scale1_;
    D.PDBlob_V.Scale2 = Scale2_;
    return D;
}
#endif
}; // namespace PAPI

#endif
