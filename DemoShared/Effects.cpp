#include "Effects.h"

#include "Particle/pAPI.h"

using namespace PAPI;

#include "Image/ImageAlgorithms.h"
#include "Math/Random.h"

#include <iostream>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace {
// Make an image for use by the PhotoShape effect if -photo wasn't specified on the command line
uc3Image* MakeFakeImage()
{
    const int SZ = 64;
    const float MX = SZ * 0.5;
    const float MN = SZ * 0.25;
    uc3Image* Img = new uc3Image(SZ, SZ);
    for (int y = 0; y < SZ; y++) {
        for (int x = 0; x < SZ; x++) {
            float rad = sqrtf((x - SZ / 2) * (x - SZ / 2) + (y - SZ / 2) * (y - SZ / 2));
            (*Img).operator()(x, y) = (rad < MX && rad > MN) ? uc3Pixel(255, 255, 0) : uc3Pixel(128, 0, 200);
        }
    }
    return Img;
}

// Bounce the point around inside the box
inline void BounceBox(pVec& jet, pVec& djet, float timeStep, const float boxSize)
{
    pVec djet2(djet);

    jet += djet * timeStep;
    if (jet.x() > boxSize) djet2.x() = copySign(djet.x(), -1.f);
    if (jet.y() > boxSize) djet2.y() = copySign(djet.y(), -1.f);
    if (jet.z() > boxSize) djet2.z() = copySign(djet.z(), -1.f);
    if (jet.x() < -boxSize) djet2.x() = copySign(djet.x(), 1.f);
    if (jet.y() < -boxSize) djet2.y() = copySign(djet.y(), 1.f);
    if (jet.z() < -boxSize) djet2.z() = copySign(djet.z(), 1.f);

    if (!(djet == djet2)) djet2 += pRandf() * 0.005f - 0.0025f;
    djet = djet2;
}
} // namespace

//////////////////////////////////////////////////////////////////////////////

Effect::Effect(EffectsManager& Efx)
{
    MotionBlur = false;
    DepthTest = false;
    KillAtStart = false;
    AList = -1;
    PrimType = 0; // GL_POINTS
    TexID = -1;
    particleSize = 1;
    particleLifetime = Efx.demoRunSec;
    particleRate = Efx.maxParticles / particleLifetime;

    CompiledFunc = NULL;
    BIND_KIND = P_INTERNAL_CODE;
}

void Effect::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    EASSERT(EM != Emit_Mode);

    ParticleContext_t& P = Efx.P;

    if (EM == Varying_Mode) P.NewActionList(AList);

    if (EM == Immediate_Mode || EM == Varying_Mode) DoActions(Efx);

    if (EM == Varying_Mode) P.EndActionList();

    if (EM == Internal_Mode || EM == Compiled_Mode || EM == Varying_Mode) P.CallActionList(AList);
}

// Non-varying effects call directly to here without a per-effect overload.
void Effect::CreateList(ExecMode_e EM, EffectsManager& Efx)
{
    EASSERT(EM != Immediate_Mode);

    ParticleContext_t& P = Efx.P;
    if (AList < 0) AList = P.GenActionLists();
    P.NewActionList(AList);
    DoActions(Efx);
    P.EndActionList();
}

// Non-varying effects call directly to here without a per-effect overload.
void Effect::EmitList(EffectsManager& Efx)
{
    CreateList(Emit_Mode, Efx);
    StartEffect(Efx); // Should call the derived class.
}

int Effect::NextEffect(EffectsManager& Efx) { return irand(Efx.getNumEffects()); }

void Effect::BindEmitted(EffectsManager& Efx, P_PARTICLE_EMITTED_ACTION_LIST Func, EmitCodeParams_e Params)
{
    Efx.P.BindEmittedActionList(AList, Func, Params);
    CompiledFunc = Func;
    BIND_KIND = Params;
}

//////////////////////////////////////////////////////////////////////////////

// Particles orbiting a center
void Atom::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDSphere(pVec(0, 0, 0), 90));
    S.Color(PDBox(pVec(1, 0, 0), pVec(1, 0, 1)));
    S.Size(pVec(1.5));
    S.StartingAge(0);

    P.Source(particleRate, PDSphere(Efx.center, 6), S);
    P.OrbitPoint(Efx.center, 100.f);
    P.TargetColor(pVec(0, 1, 0), 1, 0.05);
    P.Move(true, false);

    // Keep orbits from being too eccentric.
    P.Sink(true, PDSphere(Efx.center, 1.0));
    P.Sink(false, PDSphere(Efx.center, 8.0));
}

void Atom::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Atom::PerFrame(ExecMode_e EM, EffectsManager& Efx) { Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx); }

void Atom::StartEffect(EffectsManager& Efx) { particleRate = Efx.maxParticles / particleLifetime; }

// A bunch of balloons
void Balloons::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;

    PDUnion DomList;
    DomList.insert(PDPoint(pVec(1, 0, 0)));
    DomList.insert(PDPoint(pVec(0, 1, 0)));
    DomList.insert(PDPoint(pVec(0, 0, 1)));
    DomList.insert(PDPoint(pVec(0, 1, 1)));
    DomList.insert(PDPoint(pVec(1, 0, 1)));
    DomList.insert(PDPoint(pVec(1, 1, 0)));

    pSourceState S;
    S.Color(DomList);
    S.StartingAge(0, 5);
    S.Velocity(pVec(0.f));
    float BBOX = 2.5;
    P.Source(particleRate, PDBox(Efx.center - pVec(BBOX), Efx.center + pVec(BBOX)), S);

    P.Gravity(pVec(0.03f, -0.03f, 0.3f));
    P.Damping(pVec(0.9, 0.9, 0.67));
    float BOX = 12.0f;
    P.RandomAccel(PDBox(pVec(-BOX), pVec(BOX)));
    P.Move(true, false);
    P.KillOld(7.f);
}

void Balloons::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Balloons::PerFrame(ExecMode_e EM, EffectsManager& Efx) { Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx); }

void Balloons::StartEffect(EffectsManager& Efx) { particleRate = Efx.maxParticles / particleLifetime; }

void BounceToy::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;

    // Friction: 0 means don't slow its tangential velocity. Bigger than 0 means do.
    // Cutoff: If less than cutoff, don't apply friction.
    // Resilience: Scale normal velocity by this. Bigger is bouncier.
    const float Fric = 0.f, Res = 0.80f, Cutoff = 999.f;

    pVec C(Efx.center), Side(0, 4, 0);

    pSourceState S;
    S.Color(PDLine(pVec(1, 1, 0), pVec(0, 1, 0)));
    S.Velocity(PDDisc(pVec(0, 0, 0), pVec(0, 1, 0), 6.f));
    P.Source(600, PDLine(C + pVec(-5, 0, 10), C + pVec(5, 0, 10)), S);

    P.Gravity(Efx.GravityVec);

    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(-4, -2, 6), pVec(4, 0, 1), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(4, -2, 8), pVec(4, 0, -3), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(-1, -2, 6), pVec(2, 0, -2), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(1, -2, 2), pVec(4, 0, 2), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(-6, -2, 6), pVec(3, 0, -5), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(6, -2, 2), pVec(5, 0, 3), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(4, -2, -1), pVec(5, 0, 1.5), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(-3, -2, -1), pVec(5, 0, -1), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(-8, -2, -4.1), pVec(14, 0, 2), Side));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(C + pVec(-10, -2, 5), pVec(4, 0, 5), Side));

    P.Jet(PDBox(C + pVec(-10, -2, -6), C + pVec(-8, 2, -1)), PDPoint(pVec(0.0, 0.0, 9.0f)));
    P.TargetColor(pVec(0, 0, 1), 1, 0.04);
    P.Move(true, false);

    P.Sink(false, PDPlane(pVec(0, 0, -7), pVec(0, 0, 1)));
}

// An explosion from the center of the universe, followed by gravity toward a point
void Explosion::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    P.Damping(pVec(0.999));
    P.OrbitPoint(Efx.center, 30.f, 1.5);
    P.Explosion(Efx.center, time_since_start * 30.f, 1200.f, 3.f, 0.1);
    P.Move(true, false);
    P.Sink(false, PDSphere(pVec(0, 0, 0), 30));
}

void Explosion::EmitList(EffectsManager& Efx)
{
    time_since_start = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Explosion::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    time_since_start += Efx.timeStep;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Explosion::StartEffect(EffectsManager& Efx) { time_since_start = 0; }

// Fireflies bobbing around
void Fireflies::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Size(pVec(1.0));
    S.Velocity(PDPoint(pVec(0, 0, -2.f)));
    S.Color(PDLine(pVec(.1, .5, 0), pVec(.9, .9, .1)));
    S.StartingAge(0);
    P.Source(120, PDBlob(Efx.center, 5), S);

    P.RandomAccel(PDSphere(pVec(0.f, 0.f, 3.6f), 25.f));
    P.Move(true, false);

    P.KillOld(1.7f);
}

// Rocket-style fireworks
void Fireworks::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;

    /////////////////////////////////////////
    // The actions for moving the sparks
    pSourceState S;
    S.StartingAge(0, 6);
    S.Velocity(PDBlob(pVec(0.f), 0.4f));

    float particleRatePerRocket = particleRate / MaxRockets;
    for (int i = 0; i < MaxRockets; i++) {
        S.Color(PDLine(rocketColor[i], pVec(1, .5, .5)));
        P.Source((i < NumRockets) ? particleRatePerRocket : 0, PDPoint(rocketPos[i]), S);
    }

    P.Gravity(Efx.GravityVec);
    // P.Damping(pVec(0.999));
    P.TargetColor(pVec(0, 0, 0), 0, 0.01);
    P.Move(true, false);
    P.KillOld(particleLifetime);
}

void Fireworks::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;

    /////////////////////////////////////////
    // The actions for moving the rockets
    P.CurrentGroup(RocketGroup);

    pSourceState S;
    float s = 30.f;
    S.Velocity(PDCylinder(pVec(0, 0, 0.3) * s, pVec(0, 0, 0.5) * s, 0.11f * s, 0.07f * s));
    S.Color(PDBox(pVec(0, 0.5, 0), pVec(1, 1, 1)));
    P.Source(1000, PDDisc(pVec(0, 0, 0), pVec(0, 0, 1), 12.f), S);

    P.Sink(false, PDPlane(pVec(0, 0, -1), pVec(0, 0, 1)));
    P.Gravity(Efx.GravityVec);
    P.Move(true, false);

    // Read back the position of the rockets.
    NumRockets = (int)P.GetParticles(0, MaxRockets, (float*)rocketPos, (float*)rocketColor);

    /////////////////////////////////////////
    // The actions for moving the sparks
    if (Efx.particleHandle >= 0) P.CurrentGroup(Efx.particleHandle);

    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Fireworks::EmitList(EffectsManager& Efx)
{
    // For emitting we have a constant max number of rockets and vary their params.
    NumRockets = MaxRockets;
    for (int i = 0; i < NumRockets; i++) {
        rocketPos[i] = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
        rocketColor[i] = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    }
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Fireworks::StartEffect(EffectsManager& Efx)
{
    if (RocketGroup == -1) RocketGroup = Efx.P.GenParticleGroups(1, MaxRockets);

    // Kill any previous rockets
    Efx.P.CurrentGroup(RocketGroup);
    Efx.P.KillOld(0);
    Efx.P.KillOld(1, true);

    if (Efx.particleHandle >= 0) Efx.P.CurrentGroup(Efx.particleHandle);

    particleRate = Efx.maxParticles / particleLifetime; // Total particle rate for the sparks
}

// It's like a flame thrower spinning around
void FlameThrower::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(PDLine(pVec(0.8, 0, 0), pVec(1, 1, 0.3)));
    pVec vvel(P_VARYING_FLOAT, P_VARYING_FLOAT, 0.f);
    // if (dirAng != P_VARYING_FLOAT)
    vvel = pVec(sin(dirAng), cos(dirAng), 0.f) * 18.f;
    S.Velocity(PDBlob(vvel, 0.3f));
    S.StartingAge(0);
    S.Size(pVec(1));
    P.Source(particleRate, PDSphere(Efx.center, 0.5f), S);

    P.Gravity(pVec(0, 0, .6));
    P.Damping(pVec(0.5, 0.5, 0.97));

    const float BOX = 1.7;
    P.RandomAccel(PDBox(pVec(-BOX), pVec(BOX)));

    P.Move(true, false);
    P.KillOld(particleLifetime);
}

void FlameThrower::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    dirAng = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void FlameThrower::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    const float rotRateInRadPerSec = 1.f;
    dirAng += rotRateInRadPerSec * Efx.timeStep;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void FlameThrower::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    dirAng = 0;
}

// A fountain spraying up in the middle of the screen
void Fountain::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    float s = 0.35f;
    S.Velocity(PDCylinder(pVec(0.f, -1.f, 35.f) * s, pVec(0.0f, -1.f, 37.f) * s, 2.1f * s, 1.9f * s));
    S.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    P.Source(particleRate, PDLine(pVec(0.0, 0.0, 1.f), pVec(0.0, 0.0, 1.4f)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0.f, 0.5f, 0.f, PDDisc(pVec(0, 0, 1.f), pVec(0, 0, 1.f), 5));
    P.Move(true, false);

    P.Sink(false, PDPlane(pVec(0, 0, -3), pVec(0, 0, 1)));
    P.SinkVelocity(true, PDSphere(pVec(0, 0, 0), 0.01));
}

void Fountain::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Fountain::PerFrame(ExecMode_e EM, EffectsManager& Efx) { Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx); }

void Fountain::StartEffect(EffectsManager& Efx) { particleRate = Efx.maxParticles / particleLifetime; }

// A bunch of particles in a grid shape
void GridShape::DoActions(EffectsManager& Efx) const
{
    // Doesn't do anything. Just sets up initial particle placement
}

void GridShape::StartEffect(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;

    pSourceState S;
    S.Velocity(PDBlob(pVec(0, 0, 0), 0.1));
    P.KillOld(0);
    P.KillOld(1, true);
    int dim = int(pow(float(Efx.maxParticles), 0.33333333f));

    const pVec DD(8, 12, 9);
    const pVec dd(DD * 2.f / (float)dim);

    pVec p;
    p.z() = -DD.z();
    for (int k = 0; k < dim; k++, p.z() += dd.z()) {
        p.y() = -DD.y();
        for (int l = 0; l < dim; l++, p.y() += dd.y()) {
            p.x() = -DD.x();
            for (int j = 0; j < dim; j++, p.x() += dd.x()) {
                // Make the particle
                S.Color(pVec(j, k, l) / (float)dim);
                S.StartingAge(0);
                P.Vertex(p + Efx.center, S);
            }
        }
    }
}

// It's like a fan cruising around under a floor, blowing up on some ping pong balls.
// Like you see in real life.
void JetSpray::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0, 0, 0), 0.6));
    S.Size(pVec(1.5));
    S.Color(PDSphere(pVec(.8, .4, .1), .1));
    P.Source(60, PDRectangle(pVec(-1, -1, 0.1), pVec(2, 0, 0), pVec(0, 2, 0)), S);

    S.Color(PDSphere(pVec(.5, .4, .1), .1));
    P.Source(particleRate, PDRectangle(pVec(-10, -10, 0.1), pVec(20, 0, 0), pVec(0, 20, 0)), S);

    P.Gravity(Efx.GravityVec);

    P.Jet(PDSphere(jet, 1.5), PDBlob(pVec(0, 0, 200.f), 40.f));
    P.Bounce(0.1, 0.3, 0.1, PDRectangle(pVec(-10, -10, 0.0), pVec(20, 0, 0), pVec(0, 20, 0)));
    P.Sink(false, PDPlane(pVec(0, 0, -10), pVec(0, 0, 1)));
    P.Move(true, false);
}

void JetSpray::EmitList(EffectsManager& Efx)
{
    jet = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    Effect::EmitList(Efx);
}

void JetSpray::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    BounceBox(jet, djet, Efx.timeStep, 10);
    djet.z() = 0;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void JetSpray::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    jet = pVec(0.f);
    djet = pRandVec() * 20.f;
    djet.z() = 0.0f;
}

// A sprayer with particles that orbit two points
void Orbit2::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.02, -0.2, 0), 0.015));
    S.Size(pVec(1.0));
    const pVec tjet = Abs(jet) * 0.1 + pVec(0.4, 0.4, 0.4);
    S.Color(PDSphere(tjet, 0.1));
    P.Source(particleRate, PDPoint(jet), S);

    P.OrbitPoint(pVec(2, 0, 3), 0.1, 1.5);
    P.OrbitPoint(pVec(-2, 0, -3), 0.1, 1.5);
    P.Damping(pVec(0.994));
    P.Move(true, false);

    P.KillOld(particleLifetime);
}

void Orbit2::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    jet = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    Effect::EmitList(Efx);
}

void Orbit2::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    BounceBox(jet, djet, Efx.timeStep, 10);
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Orbit2::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    jet = pVec(-4, 0, -2.4);
    djet = pRandVec() * 0.5f;
}

// A bunch of particles in the shape of a photo
void PhotoShape::DoActions(EffectsManager& Efx) const
{
    // Doesn't do anything. Just sets up initial particle placement
}

void PhotoShape::StartEffect(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;

    // Load the particles from the photo
    pSourceState S;
    S.Velocity(PDBlob(pVec(0, 0, 0), 0.001));
    S.StartingAge(0);
    P.KillOld(0);
    P.KillOld(1, true);
    int d = (int)sqrtf(P.GetMaxParticles());

    float sx = Efx.Img->w() / float(d);
    float sy = Efx.Img->h() / float(d);
    float fy = 0.0f;
    for (int y = 0; y < d; y++, fy += sy) {
        float fx = 0.0f;
        for (int x = 0; x < d; x++, fx += sx) {
            uc3Pixel puc;
            sample2(puc, *Efx.Img, fx, fy);
            f3Pixel p(puc);
            S.Color(p.r(), p.g(), p.b());
            pVec v = pVec(fx, 0, fy);
            v /= float(Efx.Img->w());

            P.Vertex(v * 6.0f - pVec(3.0f, 0, 0), S);
        }
    }
}

// It kinda looks like rain hitting a parking lot
void Rain::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(pVec(0.f));
    S.Color(PDSphere(pVec(0.4, 0.4, 0.9), .1));
    S.Size(pVec(1.5));
    S.StartingAge(0);
    float D = 80;
    P.Source(particleRate, PDRectangle(pVec(-D / 2, -D / 2, 12), pVec(D, 0, 0), pVec(0, D, 0)), S);

    P.RandomAccel(PDBlob(pVec(0.002, 0, -0.01), 0.003));
    P.Bounce(0.3, 0.3, 0, PDPlane(pVec(0, 0, 0), pVec(0, 0, 1)));
    P.Move(true, false);

    P.KillOld(particleLifetime);
}

void Rain::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Rain::PerFrame(ExecMode_e EM, EffectsManager& Efx) { Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx); }

void Rain::StartEffect(EffectsManager& Efx) { particleRate = Efx.maxParticles / particleLifetime; }

// Restore particles to their PositionB and UpVecB.
void Restore::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    P.Restore(time_left);
    P.Move(true, false);
}

void Restore::EmitList(EffectsManager& Efx)
{
    time_left = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Restore::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    time_left -= Efx.timeStep;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Restore::StartEffect(EffectsManager& Efx) { time_left = Efx.demoRunSec; }

// A sheet of particles falling down, avoiding various-shaped obstacles
void Shower::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.f), 0.001f));
    S.Size(pVec(1.5));
    S.StartingAge(0);
    S.Color(PDBlob(pVec(.7, .7, .2), .2));
    P.Source(particleRate, PDPoint(jet), S);

    P.Gravity(Efx.GravityVec * .1);

    const float LA = 50.0f;

    if (SteerShape == 0) {
        P.Avoid(0.2, 1.0, LA, PDSphere(pVec(0, 0, 0), 1.1));
    } else if (SteerShape == 1) {
        P.Avoid(2, 1.0, LA, PDTriangle(pVec(0, -1, 0), pVec(2, 0, 0), pVec(0, 2, 0)));
    } else if (SteerShape == 2) {
        P.Avoid(2, 1.0, LA, PDRectangle(pVec(0, -1, 0), pVec(2, 1, 0), pVec(0, 2, 0)));
    } else if (SteerShape == 3) {
        P.Avoid(2, 1.0, LA, PDPlane(pVec(0, 0, 0), pVec(0, 0, 1)));
    } else if (SteerShape == P_VARYING_INT) {
        P.Avoid(2, 1.0, LA, PDVarying());
    }

    P.Move(true, false);

    P.KillOld(particleLifetime);
}

void Shower::EmitList(EffectsManager& Efx)
{
    SteerShape = P_VARYING_INT;
    jet = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    Effect::EmitList(Efx);
}

void Shower::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    BounceBox(jet, djet, Efx.timeStep, 2);
    jet += djet;
    djet.z() = 0;

    if (Efx.RenderGeometry != NULL) Efx.RenderGeometry(SteerShape);

    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Shower::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    SteerShape = irand(4);
    jet = pVec(0, 0, 5);
    djet = pRandVec() * 0.001f;
    djet.z() = 0.0f;
}

// A bunch of particles in a line that are attracted to the one ahead of them in line
void Snake::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(1, 0, 0);
    S.Velocity(pVec(0.001, 0, 0)); // This makes it able to compute a binormal.
    float BOX = .005;
    P.Source(particleRate, PDBox(pVec(-BOX, -BOX, -BOX), pVec(BOX, BOX, BOX)), S);

    // Either of these gives an interesting effect.
    P.Follow(0.01, 1.0);
    // P.Gravitate(0.01, 1.0);
    P.Damping(pVec(0.9));
    P.Move(true, false);
    P.KillOld(particleLifetime);
}

void Snake::StartEffect(EffectsManager& Efx)
{
    particleRate = 1.0f; // Make a few additional particles

    ParticleContext_t& P = Efx.P;

    pSourceState S;
    S.Velocity(pVec(0.f));
    S.Size(pVec(1.0));
    S.Color(PDSphere(pVec(0.93, 0.93, 0), 0.05));

    for (float x = -10.0; x < 2.0; x += 0.05) P.Vertex(pVec(x, 0, 0), S);
}

// A bunch of particles inside a sphere
void Sphere::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(PDLine(pVec(0, 1, 0), pVec(0, 0, 1)));
    S.Velocity(PDBlob(pVec(dirAng == P_VARYING_FLOAT ? P_VARYING_FLOAT : sin(dirAng) * .1, dirAng == P_VARYING_FLOAT ? P_VARYING_FLOAT : cos(dirAng) * .1, 0.1),
                      0.01));
    S.StartingAge(0);
    S.Size(pVec(1));
    P.Source(particleRate, PDPoint(pVec(1, 1, 6)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0, 0.55, 0, PDSphere(pVec(0, 0, 4), 6));
    P.Move(true, false);

    P.KillOld(particleLifetime);
}

void Sphere::EmitList(EffectsManager& Efx)
{
    dirAng = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Sphere::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    const float rotRateInRadPerSec = 0.5f;
    dirAng += rotRateInRadPerSec * Efx.timeStep;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Sphere::StartEffect(EffectsManager& Efx)
{
    dirAng = 0;
    particleRate = Efx.maxParticles / particleLifetime;
}

// A sprayer with particles orbiting a line
void Swirl::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    const pVec tjet = Abs(jet) * 0.1 + pVec(0.4, 0.4, 0.4);
    S.Color(PDSphere(tjet, 0.1));
    S.Velocity(PDBlob(pVec(0.02, -0.2, 0), 0.015));
    S.Size(pVec(1.0));
    S.StartingAge(0);
    P.Source(particleRate, PDPoint(jet), S);

    P.OrbitLine(pVec(0, 0, 1), pVec(1, 0.1, 0), 0.1, 1.5);
    P.Damping(pVec(1, 0.994, 0.994));
    P.Move(true, false);

    P.Sink(false, PDSphere(pVec(0, 0, 0), 15));
    P.KillOld(particleLifetime);
}

void Swirl::EmitList(EffectsManager& Efx) { Effect::EmitList(Efx); }

void Swirl::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    BounceBox(jet, djet, Efx.timeStep, 10);
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Swirl::StartEffect(EffectsManager& Efx)
{
    jet = pVec(-4, 0, -2.4);
    djet = pRandVec() * 0.05;
    particleRate = Efx.maxParticles / particleLifetime;
}

// A tornado that tests the vortex action
// TODO: Change it to move around a bed of particles like JetSpray and suck them up
void Tornado::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Size(pVec(1.0));
    S.Velocity(PDPoint(pVec(0, 0, 0)));
    S.Color(PDLine(pVec(.0, .8, .8), pVec(1, 1, 1)));
    S.StartingAge(0);
    P.Source(particleRate, PDLine(pVec(-8, 0, 15), pVec(8, 0, 15)), S);

    P.Damping(pVec(.95));
    P.Gravity(Efx.GravityVec);
    P.Vortex(Efx.center + pVec(0, 0, -5.f), pVec(0, 0, 11), 1.8f, 7.f, 80.f, -2000, 1000.0f);
    P.Move(true, false);

    P.KillOld(particleLifetime);
    P.Sink(false, PDPlane(pVec(0, 0, -2), pVec(0, 0, 1)));
}

void Tornado::StartEffect(EffectsManager& Efx) { particleRate = Efx.maxParticles / particleLifetime; }

// A waterfall bouncing off invisible rocks
void Waterfall::DoActions(EffectsManager& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.03, -0.001, 0.01), 0.002));
    S.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    S.Size(pVec(1.5));
    P.Source(particleRate, PDLine(pVec(-5, -1, 8), pVec(-5, -3, 8)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0, 0.35, 0, PDRectangle(pVec(-7, -4, 7), pVec(3, 0, 0), pVec(0, 3, 0)));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(-4, -2, 4), 0.2));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(-3.5, 0, 2), 2));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(3.8, 0, 0), 2));
    P.Bounce(-0.01, 0.35, 0, PDPlane(pVec(0, 0, 0), pVec(0, 0, 1)));
    P.Move(true, false);

    P.KillOld(particleLifetime);
    P.Sink(false, PDSphere(pVec(0, 0, 0), 20));
}

void Waterfall::StartEffect(EffectsManager& Efx) { particleRate = Efx.maxParticles / particleLifetime; }

//////////////////////////////////////////////////////////////////////////////

EffectsManager::EffectsManager(ParticleContext_t& P_, int mp, E_RENDER_GEOMETRY RG) : P(P_), RenderGeometry(RG)
{
    Img = MakeFakeImage();
    maxParticles = mp;
    simStepsPerFrame = 1;
    timeStep = 1.f;
    demoRunSec = 10.0f;
    particleHandle = -1;
    GravityVec = pVec(0.0f, 0.0f, -9.8f);
    MakeEffects();
}

// EM specifies how you want to run (for different benchmark purposes, mostly).
// Allowed values are Immediate_Mode, Internal_Mode, and Compiled_Mode.
// Set DemoNum to -2 to let NextEffect choose the next demo.
int EffectsManager::CallDemo(int DemoNum, ExecMode_e EM)
{
    if (DemoNum == -1) DemoNum = getNumEffects() - 1;
    DemoNum = DemoNum % getNumEffects();

    if (DemoNum < 0 || Effects[DemoNum] != Demo) {
        // Transitioning, so call NextEffect() and StartEffect().
        if (DemoNum < 0 && Demo != NULL)
            DemoNum = Demo->NextEffect(*this); // The effect will tell us what the next effect should be
        else if (Demo == NULL)
            DemoNum = irand(getNumEffects());

        EASSERT(DemoNum >= 0 && DemoNum < getNumEffects());
        Demo = Effects[DemoNum];

        Demo->StartEffect(*this);
    }

    EASSERT(EM == Immediate_Mode || EM == Internal_Mode || EM == Compiled_Mode);

    if (EM == Internal_Mode && Demo->CompiledFunc != NULL) {
        // The AL is set up for compiled mode; need to switch it to internal mode
        P.BindEmittedActionList(Demo->AList, NULL, P_INTERNAL_CODE);
    }

    Demo->PerFrame(EM, *this);

    if (EM == Internal_Mode && Demo->CompiledFunc != NULL) {
        // We switched the AL to internal mode. Now switch it back.
        P.BindEmittedActionList(Demo->AList, Demo->CompiledFunc, Demo->BIND_KIND);
    }

    return DemoNum;
}

void EffectsManager::MakeEffects()
{
    Effects.push_back(new Atom(*this));
    Effects.push_back(new Balloons(*this));
    Effects.push_back(new BounceToy(*this));
    Effects.push_back(new Explosion(*this));
    Effects.push_back(new Fireflies(*this));
    Effects.push_back(new Fireworks(*this));
    Effects.push_back(new FlameThrower(*this));
    Effects.push_back(new Fountain(*this));
    Effects.push_back(new GridShape(*this));
    Effects.push_back(new JetSpray(*this));
    Effects.push_back(new Orbit2(*this));
    Effects.push_back(new PhotoShape(*this));
    Effects.push_back(new Rain(*this));
    Effects.push_back(new Restore(*this));
    Effects.push_back(new Shower(*this));
    Effects.push_back(new Snake(*this));
    Effects.push_back(new Sphere(*this));
    Effects.push_back(new Swirl(*this));
    Effects.push_back(new Tornado(*this));
    Effects.push_back(new Waterfall(*this));
}

void EffectsManager::MakeActionLists(ExecMode_e EM)
{
    for (auto e : Effects) { e->CreateList(EM, *this); }
}

//////////////////////////////////////////////////////////////////////////////

#if 0
    // Friction: 0 means don't slow its tangential velocity. Bigger than 0 means do.
    // Cutoff: If less than cutoff, don't apply friction.
    // Resilience: Scales velocity in normal direction. Bigger is bouncier.
    const float Fric = 1.0f, Res = 0.95f, Cutoff = 0.0f;

    // Don't apply friction if tangential velocity < cutoff
    // float tanscale = (vt.lenSqr() <= cutoffSqr) ? 1.0f : oneMinusFriction;
    // m.vel = vt * tanscale + vn * resilience;

    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(4,0,0), pVec(1,0,0)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(-4,0,0), pVec(1,0,0)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(0,1,0), pVec(0,1,0)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(0,-4,0), pVec(0,1,0)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(0,0,4), pVec(0,0,1)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(0,0,0), pVec(0,0,1)));

    P.Move(true, false);
#endif
