// This project encapsulates the compiled form of the effects.
// It depends on the effects being created by CUDA_Effects_Gen.
// Link this with PSpray and ParBench, but not CUDA_Effects_Gen or Effects.

#include "Effects/Effects.h"

#include "Particle/pAPI.h"

using namespace PAPI;

#include <iostream>

using namespace std;

// You must create the action lists first.
void BindEffects(ParticleEffects &Efx)
{
    // The kernel calling functions that we bind to the action lists

    extern void E0_Atom(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E1_Balloons(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E2_BounceToy(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E3_Explosion(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E4_Fireflies(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E5_Fireworks(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E6_FlameThrower(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E7_Fountain(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E8_GridShape(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E9_JetSpray(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E10_Orbit2(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E11_PhotoShape(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E12_Rain(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E13_Restore(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E14_Shower(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E15_Snake(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E16_Sphere(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E17_Swirl(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E18_Tornado(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E19_WaterfallA(const void *A, void *P, const float dt, const int block_size); // GREP0
    extern void E20_WaterfallB(const void *A, void *P, const float dt, const int block_size); // GREP0

    EmitCodeParams_e BIND_KIND = P_CPU_CPP_CODE;

    Efx.Effects[0]->BindEmitted(Efx, E0_Atom, BIND_KIND); // GREP1
    Efx.Effects[1]->BindEmitted(Efx, E1_Balloons, BIND_KIND); // GREP1
    Efx.Effects[2]->BindEmitted(Efx, E2_BounceToy, BIND_KIND); // GREP1
    Efx.Effects[3]->BindEmitted(Efx, E3_Explosion, BIND_KIND); // GREP1
    Efx.Effects[4]->BindEmitted(Efx, E4_Fireflies, BIND_KIND); // GREP1
    Efx.Effects[5]->BindEmitted(Efx, E5_Fireworks, BIND_KIND); // GREP1
    Efx.Effects[6]->BindEmitted(Efx, E6_FlameThrower, BIND_KIND); // GREP1
    Efx.Effects[7]->BindEmitted(Efx, E7_Fountain, BIND_KIND); // GREP1
    Efx.Effects[8]->BindEmitted(Efx, E8_GridShape, BIND_KIND); // GREP1
    Efx.Effects[9]->BindEmitted(Efx, E9_JetSpray, BIND_KIND); // GREP1
    Efx.Effects[10]->BindEmitted(Efx, E10_Orbit2, BIND_KIND); // GREP1
    Efx.Effects[11]->BindEmitted(Efx, E11_PhotoShape, BIND_KIND); // GREP1
    Efx.Effects[12]->BindEmitted(Efx, E12_Rain, BIND_KIND); // GREP1
    Efx.Effects[13]->BindEmitted(Efx, E13_Restore, BIND_KIND); // GREP1
    Efx.Effects[14]->BindEmitted(Efx, E14_Shower, BIND_KIND); // GREP1
    Efx.Effects[15]->BindEmitted(Efx, E15_Snake, BIND_KIND); // GREP1
    Efx.Effects[16]->BindEmitted(Efx, E16_Sphere, BIND_KIND); // GREP1
    Efx.Effects[17]->BindEmitted(Efx, E17_Swirl, BIND_KIND); // GREP1
    Efx.Effects[18]->BindEmitted(Efx, E18_Tornado, BIND_KIND); // GREP1
    Efx.Effects[19]->BindEmitted(Efx, E19_WaterfallA, BIND_KIND); // GREP1
    Efx.Effects[20]->BindEmitted(Efx, E20_WaterfallB, BIND_KIND); // GREP1
}
