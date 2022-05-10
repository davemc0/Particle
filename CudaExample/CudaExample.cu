// Example.cpp - An example of the Particle System API in OpenGL
//
// Copyright 1999-2006, 2022 by David K. McAllister

#include "CudaExample.h"

#include "Particle/pAPI.h"
using namespace PAPI;

#include "Particle/pActionImpls.h"

#include <cuda.h>
#include <curand.h>
#include <curand_kernel.h>
extern ParticleContext_t P;

Particle_t* devParticles = nullptr;
curandState_t* devRandState = nullptr;

__global__ void ParticleSetupKernel(curandState_t* devRandState, int maxParticles)
{
    int xi = blockIdx.x * blockDim.x + threadIdx.x;
    if (xi > maxParticles) return;

    devRandState[xi] = pSRandf(xi);
}

void CudaParticleSetup(int maxParticles)
{
    cudaMalloc(&devParticles, sizeof(Particle_t) * maxParticles);
    cudaMalloc(&devRandState, sizeof(curandState_t) * maxParticles);
}

__global__ void ParticleDynamics(Particle_t* devParticles, int maxParticles, float dt)
{
    int xi = blockIdx.x * blockDim.x + threadIdx.x;
    if (xi > maxParticles) return;

    Particle_t& p_ = devParticles[xi];

    // Gravity
    PAGravity_Impl(p_, dt, pVec(0.f, 0.f, -0.01f));

    // Bounce particles off a disc of radius 5
    PABounceDisc_Impl(p_, dt, PDDisc(pVec(0.f, 0.f, 0.f), pVec(0.f, 0.f, 1.f), 5.f), 0.f, 0.5f, 0.f);

    // Kill particles below Z=-3
    PASink_Impl(p_, dt, false, PDPlane(pVec(0.f, 0.f, -3.f), pVec(0.f, 0.f, 1.f)));

    // Move particles to their new positions
    PAMove_Impl(p_, dt, true, false);
}

// A water fountain spraying upward
void ComputeParticles()
{
    // Set the state of the new particles to be generated
    pSourceState S;
    S.Velocity(PDCylinder(pVec(0.0f, -0.01f, 0.25f), pVec(0.0f, -0.01f, 0.27f), 0.021f, 0.019f));
    S.Color(PDLine(pVec(0.8f, 0.9f, 1.0f), pVec(1.0f, 1.0f, 1.0f)));

    // Generate particles along a very small line in the nozzle
    P.Source(200, PDLine(pVec(0.f, 0.f, 0.f), pVec(0.f, 0.f, 0.4f)), S);

    dim3 threads(32);
    dim3 grid(P.GetGroupCount());

    ParticleDynamics<<<grid, threads>>>(devParticles, P.GetGroupCount(), P.GetTimeStep());

    P.CommitKills();
}
