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
    const int SZ = 512;
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
    AList = -1;
    WhiteBackground = true;
    MotionBlur = false;
    DepthTest = false;
    SortParticles = false;
    PrimType = PRIM_POINT;
    UseRenderingParams = true;
    particleSize = 0.15f; // World space units
    particleLifetime = Efx.demoRunSec;
    particleRate = Efx.maxParticles / particleLifetime;

    CompiledFunc = NULL;
    BIND_KIND = P_INTERNAL_CODE;
}

void Effect::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    EASSERT(EM != Emit_Mode);

    ParticleContext_t& P = Efx.P;

    if (EM == Varying_Mode) P.NewActionList(AList); // Create an action list to store current values of varying parameters

    if (EM == Immediate_Mode || EM == Varying_Mode) {
        Renderables.clear(); // DoActions fills in the Renderables, so clear it before calling DoActions
        DoActions(Efx);
    }

    if (EM == Varying_Mode) P.EndActionList();

    if (EM == Internal_Mode || EM == Compiled_Mode || EM == Varying_Mode) P.CallActionList(AList);
}

// Non-varying effects call directly to here without a per-effect overload.
void Effect::CreateList(ExecMode_e EM, EffectsManager& Efx)
{
    EASSERT(EM != Immediate_Mode);

    ParticleContext_t& P = Efx.P;
    if (AList < 0) AList = P.GenActionLists();
    Renderables.clear(); // DoActions fills in the Renderables, so clear it before calling DoActions
    P.NewActionList(AList);
    DoActions(Efx);
    P.EndActionList();
}

// Non-varying effects call directly to here without a per-effect overload.
void Effect::EmitList(EffectsManager& Efx)
{
    CreateList(Emit_Mode, Efx);
    StartEffect(Efx); // Calls the derived class.
}

int Effect::NextEffect(EffectsManager& Efx) { return irand(Efx.getNumEffects()); }

void Effect::BindEmitted(EffectsManager& Efx, P_PARTICLE_EMITTED_ACTION_LIST Func, EmitCodeParams_e Params)
{
    Efx.P.BindEmittedActionList(AList, Func, Params);
    CompiledFunc = Func;
    BIND_KIND = Params;
}

// A nonvirtual function to insert the renderable domain into this effect's renderable list
const pDomain& Effect::Render(const pDomain& dom)
{
    Renderables.push_back(std::shared_ptr<pDomain>(dom.copy()));
    return dom;
}

//////////////////////////////////////////////////////////////////////////////

// Particles orbiting a center
void Atom::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDSphere(pVec(0, 0, 0), 60));
    S.Color(PDBox(pVec(1, 0, 0), pVec(1, 0, 1)));
    S.Size(particleSize);
    S.StartingAge(0);

    P.Source(particleRate, PDSphere(Efx.center, 4.f, 3.f), S);
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

void Atom::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    PrimType = PRIM_SPHERE_SPRITE;
    WhiteBackground = false;
    DepthTest = true;
    MotionBlur = true;
    SortParticles = false;
}

// A bunch of balloons
void Balloons::DoActions(EffectsManager& Efx)
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
    S.Size(particleSize);
    S.Velocity(pVec(0.f));
    float BBOX = 2.5;
    P.Source(particleRate, PDBox(Efx.center - pVec(BBOX), Efx.center + pVec(BBOX)), S);

    P.Gravity(pVec(0.03f, -0.03f, 0.3f));
    P.Damping(pVec(0.9, 0.9, 0.67));
    float BOX = 12.0f;
    P.RandomAccel(PDBox(pVec(-BOX), pVec(BOX)));
    P.Move(true, false);
    P.KillOld(7.f);
    P.Sink(false, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));
}

void Balloons::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Balloons::PerFrame(ExecMode_e EM, EffectsManager& Efx) { Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx); }

void Balloons::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    PrimType = PRIM_SPHERE_SPRITE;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

void Boids::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;

    pVec C(Efx.center), Side(0, 4, 0);
    const float radius = 1.5f, minSpeed = 3.f, maxSpeed = 6.f;

    pSourceState S;
    S.Color(pVec(1.f));
    S.Velocity(PDSphere(pVec(0.f), 0.f));
    S.Size(particleSize);
    P.Source(particleRate, PDSphere(pVec(0, 15, 25), 10.f, 0.f), S);

    // P.Gravity(Efx.GravityVec * 0.2f);
    P.OrbitPoint(goalPoint, 300.f, 10.f); // Follow goal
    Render(PDSphere(goalPoint, 0.25f));
    P.Damping(0.98f, minSpeed, P_MAXFLOAT);

    P.Gravitate(0.15f, 0.005f); // Flock centering

    P.MatchVelocity(0.3f, 0.5f, radius); // Velocity matching

    P.Gravitate(-1.f, 0.1f, radius); // Neighbor collision avoidance

    P.Avoid(5.f, 0.1f, 1.5f, Render(PDRectangle(pVec(0, -8, 2), pVec(0, 0, 8), pVec(0, 16, 0))));
    P.Avoid(5.f, 0.1f, 1.5f, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));

    P.SpeedClamp(minSpeed, maxSpeed);
    P.TargetColor(pVec(0, 0, 0), 1, 0.04f);
    P.Move(true, false);
    P.Sink(false, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));
    P.Sink(false, PDSphere(pVec(0.f), 40.f));
}

void Boids::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    time_since_start += Efx.timeStep;

    goalPoint = (fmod(time_since_start, 2.f * Efx.demoRunSec) > Efx.demoRunSec) ? pVec(-10.f, 0, 6.f) : pVec(10.f, 0, 6.f);

    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Boids::StartEffect(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    if (Efx.particleHandle >= 0 && P.GetMaxParticles() > 3000) {
        size_t pmax = P.GetMaxParticles();
        P.SetMaxParticles(3000);
        P.SetMaxParticles(pmax);
    }

    time_since_start = 0;
    particleRate = 100;
    PrimType = PRIM_DISPLAY_LIST;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

void BounceToy::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;

    // Friction: 0 means don't slow its tangential velocity. Bigger than 0 means do.
    // FricMinTanVel: If tangential velocity magnitude less than FricMinTanVel, don't apply friction.
    // Resilience: Scale normal velocity by this. Bigger is bouncier.
    const float Fric = 0.f, Res = 0.80f, FricMinTanVel = 999.f;

    pVec C(Efx.center), Side(0, 4, 0);

    pSourceState S;
    S.Color(PDLine(pVec(1, 1, 0), pVec(0, 1, 0)));
    S.Velocity(PDDisc(pVec(0, 0, 0), pVec(0, 1, 0), 6.f));
    S.Size(particleSize);
    P.Source(particleRate, PDLine(C + pVec(-5, 0, 10), C + pVec(5, 0, 10)), S);

    P.Gravity(Efx.GravityVec);

    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(-4, -2, 6), pVec(4, 0, 1), Side)));
    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(4, -2, 8), pVec(4, 0, -3), Side)));
    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(-1, -2, 6), pVec(2, 0, -2), Side)));
    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(1, -2, 2), pVec(4, 0, 2), Side)));
    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(-6, -2, 6), pVec(3, 0, -5), Side)));
    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(6, -2, 2), pVec(5, 0, 3), Side)));
    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(4, -2, -1), pVec(5, 0, 1.5), Side)));
    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(-3, -2, -1), pVec(5, 0, -1), Side)));
    P.Bounce(0.05f, Res, 0, Render(PDRectangle(C + pVec(-8, -2, -4.1), pVec(14, 0, 2), Side)));
    P.Bounce(Fric, Res, FricMinTanVel, Render(PDRectangle(C + pVec(-10, -2, 5), pVec(4, 0, 5), Side)));

    P.Jet(Render(PDBox(C + pVec(-10, -2, -6), C + pVec(-8, 2, -1))), PDPoint(pVec(0.0, 0.0, 100.f)));
    P.TargetColor(pVec(0, 0, 1), 1, 0.04);
    P.Move(true, false);

    P.Sink(false, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));
}

void BounceToy::StartEffect(EffectsManager& Efx)
{
    particleRate = 60000;
    PrimType = PRIM_LINE;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// An explosion from the center of the universe, followed by gravity toward a point
void Explosion::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    P.Damping(pVec(0.999f));
    P.OrbitPoint(Efx.center, 30.f, 1.5);
    P.Explosion(Efx.center, time_since_start * 30.f, 1000.f, 3.f, 0.1);
    P.Move(true, false);
    // P.Sink(false, PDSphere(Efx.center, 50.f));
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

void Explosion::StartEffect(EffectsManager& Efx)
{
    time_since_start = 0;
    UseRenderingParams = false;
}

// Fireflies bobbing around
void Fireflies::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDPoint(pVec(0, 0, -1.f)));
    S.Color(PDLine(pVec(.1, .5, 0), pVec(.9, .9, .1)));
    S.StartingAge(100.f, 0.f);
    S.Size(particleSize);
    P.Source(particleRate, PDBox(Efx.center - pVec(150, 25, 5), Efx.center + pVec(150, 150, 5)), S);

    P.RandomAccel(PDSphere(pVec(0.f, 0.f, 3.6f), 25.f));
    P.Move(true, false);

    P.KillOld(101.7f);
    P.Sink(false, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));
}

void Fireflies::StartEffect(EffectsManager& Efx)
{
    particleRate = 5000.f;
    PrimType = PRIM_GAUSSIAN_SPRITE;
    WhiteBackground = false;
    DepthTest = true;
    MotionBlur = true;
    SortParticles = true;
}

// Rocket-style fireworks
void Fireworks::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;

    /////////////////////////////////////////
    // The actions for moving the sparks
    pSourceState S;
    S.StartingAge(0, 6.f);
    S.Size(particleSize);
    S.Velocity(PDBlob(pVec(0.f), 0.4f));

    float particleRatePerRocket = particleRate / MaxRockets;
    for (int i = 0; i < NumRockets; i++) {
        S.Color(PDLine(rocketColor[i], pVec(1, .5, .5)));
        P.Source(particleRatePerRocket, PDPoint(rocketPos[i]), S);
    }

    P.Gravity(Efx.GravityVec);
    // P.Damping(pVec(0.999));
    P.TargetColor(pVec(0, 0, 0), 0, 0.01);
    P.Move(true, false);
    P.Sink(false, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));
}

void Fireworks::EmitList(EffectsManager& Efx)
{
    EASSERT(0); // Just make sure this code isn't used for now.
    // For emitting we have a constant max number of rockets and vary their params.
    NumRockets = MaxRockets;
    for (int i = 0; i < NumRockets; i++) {
        rocketPos[i] = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
        rocketColor[i] = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    }
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
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
    S.Size(particleSize);
    S.Color(PDBox(pVec(0, 0.5, 0), pVec(1, 1, 1)));
    P.Source(1000, PDDisc(pVec(0, 0, 0.1f), pVec(0, 0, 1), 12.f), S);

    P.Sink(false, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));
    P.Gravity(Efx.GravityVec);
    P.Move(true, false);

    // Read back the position of the rockets.
    NumRockets = (int)P.GetParticles(0, MaxRockets, (float*)rocketPos, false, (float*)rocketColor);

    /////////////////////////////////////////
    // The actions for moving the sparks
    if (Efx.particleHandle >= 0) P.CurrentGroup(Efx.particleHandle);

    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
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
    PrimType = PRIM_GAUSSIAN_SPRITE;
    WhiteBackground = false;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = true;
}

// It's like a flame thrower spinning around
void FlameThrower::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(PDLine(pVec(0.8, 0, 0), pVec(1, 1, 0.3)));
    pVec vvel(P_VARYING_FLOAT, P_VARYING_FLOAT, 0.f);
    if (dirAng != P_VARYING_FLOAT) vvel = pVec(sin(dirAng), cos(dirAng), 0.f) * 18.f;
    S.Velocity(PDBlob(vvel, 0.3f));
    S.StartingAge(0);
    S.Size(particleSize);
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
    PrimType = PRIM_GAUSSIAN_SPRITE;
    WhiteBackground = true;
    DepthTest = false;
    MotionBlur = false;
    SortParticles = true;
}

// A fountain spraying up in the middle of the screen
void Fountain::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    float s = 0.35f;
    S.Velocity(PDCylinder(pVec(0.f, -1.f, 35.f) * s, pVec(0.0f, -1.f, 37.f) * s, 2.1f * s, 1.9f * s));
    S.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    S.Size(particleSize);
    P.Source(particleRate, PDLine(pVec(0.0, 0.0, 1.f), pVec(0.0, 0.0, 1.4f)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0.f, 0.5f, 0.f, Render(PDDisc(pVec(0, 0, 1.f), pVec(0, 0, 1.f), 5)));
    P.Move(true, false);

    P.Sink(false, Render(PDPlane(pVec(0, 0, -3), pVec(0, 0, 1))));
    P.SinkVelocity(true, PDSphere(pVec(0, 0, 0), 0.01));
}

void Fountain::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Fountain::PerFrame(ExecMode_e EM, EffectsManager& Efx) { Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx); }

void Fountain::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    PrimType = PRIM_SPHERE_SPRITE; // None of these look like water.
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// A bunch of particles in a grid shape
void GridShape::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    P.Move(true, false);
}

void GridShape::StartEffect(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;

    if (P.GetGroupCount() * 2 > P.GetMaxParticles()) { // Is there room to add the formation without deleting anything?
        P.SetMaxParticles(P.GetMaxParticles() / 2);
        P.SetMaxParticles(P.GetMaxParticles() * 2);
    }
    int numNewParticles = (int)P.GetMaxParticles() - (int)P.GetGroupCount();

    pSourceState S;
    S.Velocity(PDBlob(pVec(0.f), 0.0001f));
    S.Size(particleSize);
    int dim = int(pow(float(numNewParticles), 0.33333333f));

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

    PrimType = PRIM_SPHERE_SPRITE;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// It's like a fan cruising around under a floor, blowing up on some ping pong balls.
// Like you see in real life.
void JetSpray::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0, 0, 0), 0.6));
    S.Size(particleSize);
    S.Color(PDSphere(pVec(.8, .4, .1), .1));
    P.Source(60, PDRectangle(pVec(-1, -1, 0.1), pVec(2, 0, 0), pVec(0, 2, 0)), S);

    S.Color(PDSphere(pVec(.5, .4, .1), .1));
    P.Source(particleRate, PDRectangle(pVec(-10, -10, 0.1), pVec(20, 0, 0), pVec(0, 20, 0)), S);

    P.Gravity(Efx.GravityVec);

    P.Jet(Render(PDSphere(jet, 1.5)), PDBlob(pVec(0, 0, 200.f), 40.f));
    P.Bounce(0.1, 0.3, 0.1, Render(PDRectangle(pVec(-10, -10, 0.0), pVec(20, 0, 0), pVec(0, 20, 0))));
    P.Sink(false, Render(PDPlane(pVec(0, 0, -10), pVec(0, 0, 1))));
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
    PrimType = PRIM_GAUSSIAN_SPRITE;
    WhiteBackground = false;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// A sprayer with particles that orbit two points
void Orbit2::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(2.f, -2.f, 0), 1.5f));
    S.Size(particleSize);
    const pVec tjet = Abs(jet) * 0.1 + pVec(0.4, 0.4, 0.4);
    S.Color(PDSphere(tjet, 0.1));
    P.Source(particleRate, PDPoint(jet), S);

    P.OrbitPoint(pVec(2, 0, 3), 100.f, 1.5f);
    P.OrbitPoint(pVec(-2, 0, 3), 100.f, 1.5f);
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
    jet = pVec(-4, 0, 2.4);
    djet = pRandVec() * 20.f;
    PrimType = PRIM_GAUSSIAN_SPRITE;
    WhiteBackground = true;
    DepthTest = false;
    MotionBlur = true;
    SortParticles = true;
}

// A bunch of particles in the shape of a photo
void PhotoShape::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    P.Move(true, false);
}

void PhotoShape::StartEffect(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;

    if (P.GetGroupCount() * 2 > P.GetMaxParticles()) { // Is there room to add the formation without deleting anything?
        P.SetMaxParticles(P.GetMaxParticles() / 2);
        P.SetMaxParticles(P.GetMaxParticles() * 2);
    }
    int numNewParticles = (int)P.GetMaxParticles() - (int)P.GetGroupCount();

    // Load the particles from the photo
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.f), 0.0001f));
    S.StartingAge(0);
    S.Size(particleSize);
    int d = (int)sqrtf(numNewParticles);

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
            pVec v = pVec(fx, 0, Efx.Img->h() - fy);
            v /= float(Efx.Img->w());

            P.Vertex(v * 6.0f - pVec(3.0f, 0, -0.1f), S);
        }
    }

    PrimType = PRIM_GAUSSIAN_SPRITE;
    WhiteBackground = true;
    DepthTest = false;
    MotionBlur = false;
    SortParticles = false;
}

// It kinda looks like rain hitting a parking lot
void Rain::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDSphere(pVec(0.f), 1.f));
    S.Color(PDSphere(pVec(0.4, 0.4, 0.9), .1));
    S.Size(particleSize);
    S.StartingAge(0, 5);
    float D = 200;
    P.Source(particleRate, PDRectangle(pVec(-D / 2, -D / 2, 15), pVec(D, 0, 0), pVec(0, D, 0)), S);
    P.Gravity(Efx.GravityVec);
    P.Bounce(0.3, 0.3, 0, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));
    P.Move(true, false);

    P.KillOld(particleLifetime);
}

void Rain::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Rain::PerFrame(ExecMode_e EM, EffectsManager& Efx) { Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx); }

void Rain::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    PrimType = PRIM_SPHERE_SPRITE;
    WhiteBackground = false;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// Restore particles to their PositionB and UpVecB.
void Restore::DoActions(EffectsManager& Efx)
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

void Restore::StartEffect(EffectsManager& Efx)
{
    time_left = Efx.demoRunSec * 0.8f; // Save some time at the end to appreciate the result
    UseRenderingParams = false;
}

// A sheet of particles falling down, avoiding various-shaped obstacles
void Shower::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.f), 0.1f));
    S.Size(particleSize);
    S.StartingAge(0);
    S.Color(PDBlob(pVec(.7, .7, .2), .2));
    P.Source(particleRate, PDPoint(jet), S);

    P.Gravity(Efx.GravityVec * 0.1f);

    const float lookAheadTime = 2.f;

    if (SteerShape == STEER_SPHERE) {
        P.Avoid(3.f, 0.1f, lookAheadTime, Render(PDSphere(pVec(0, 0, 0), 1.f)));
    } else if (SteerShape == STEER_TRIANGLE) {
        P.Avoid(3.f, 0.1f, lookAheadTime, Render(PDTriangle(pVec(0, -1, 0.1f), pVec(2, 0, 0.1f), pVec(0, 2, 0.1f))));
    } else if (SteerShape == STEER_RECTANGLE) {
        P.Avoid(3.f, 0.1f, lookAheadTime, Render(PDRectangle(pVec(0, -1, 0.1f), pVec(2, 1, 0), pVec(0, 2, 0))));
    } else if (SteerShape == STEER_PLANE) {
        P.Avoid(10.f, 1.f, lookAheadTime, Render(PDPlane(pVec(0, 0, 0.1f), pVec(0, 0, 1))));
    } else if (SteerShape == STEER_DISC) {
        P.Avoid(3.f, 0.1f, lookAheadTime, Render(PDDisc(pVec(0, 0, 0.1f), pVec(0, 0, 1), 1.f, 0.f)));
    } else if (SteerShape == P_VARYING_INT) {
        P.Avoid(3.f, 0.1f, lookAheadTime, PDVarying());
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

    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Shower::StartEffect(EffectsManager& Efx)
{
    particleRate = min(100.f, Efx.maxParticles / particleLifetime);
    SteerShape = irand(STEER_VARYING);
    jet = Efx.center;
    djet = pRandVec() * 0.02f;
    djet.z() = 0.0f;

    PrimType = PRIM_LINE;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// A bunch of particles in a line that are attracted to the one after them in the list
void Snake::DoActions(EffectsManager& Efx)
{
    // Add new particles to attract the older ones back toward the center
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(1, 0, 0);
    S.Velocity(PDSphere(pVec(0.f), 0.1f)); // This makes it able to compute a binormal.
    S.Size(particleSize);
    float BOX = .5f;
    P.Source(particleRate, Render(PDBox(pVec(-BOX), pVec(BOX))), S);

    P.Follow(10.f, 1.0f);
    // P.Gravitate(10.f, 1.0f); // Gives an interesting effect, but very slow
    P.Damping(pVec(0.9));
    P.Move(true, false);
    P.Sink(false, PDSphere(pVec(0, 0, 0), 25));
}

void Snake::StartEffect(EffectsManager& Efx)
{
    particleRate = 10.0f; // Make a few additional particles
    PrimType = PRIM_DISPLAY_LIST;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// A bunch of particles inside a sphere
void Sphere::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(PDLine(pVec(0, 1, 0), pVec(0, 0, 1)));
    pVec vvel(P_VARYING_FLOAT, P_VARYING_FLOAT, 0.f);
    if (dirAng != P_VARYING_FLOAT) vvel = pVec(sin(dirAng), cos(dirAng), 0.f) * 12.f;
    S.Velocity(PDBlob(vvel, 0.4f));
    S.StartingAge(0);
    S.Size(particleSize);
    P.Source(particleRate, PDPoint(pVec(1, 0, 8)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0, 0.55, 0, Render(PDSphere(Efx.center, 5)));
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

    PrimType = PRIM_DISPLAY_LIST;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

void Sphere::StartEffect(EffectsManager& Efx)
{
    dirAng = 0;
    particleRate = Efx.maxParticles / particleLifetime;
    PrimType = PRIM_LINE;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// A sprayer with particles orbiting a line
void Swirl::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(2.f, -2.f, 0), 1.5f));
    S.Size(particleSize);
    const pVec tjet = Abs(jet) * 0.1 + pVec(0.4, 0.4, 0.4);
    S.Color(PDSphere(tjet, 0.1));
    P.Source(particleRate, PDPoint(jet), S);

    P.OrbitLine(pVec(2, 0, 3), pVec(1.f, 0.f, 0.f), 100.f, 1.5f);
    P.Damping(pVec(0.995f));
    P.Move(true, false);

    P.Sink(false, PDSphere(Efx.center, 25.f));
    P.KillOld(particleLifetime);
}

void Swirl::EmitList(EffectsManager& Efx)
{
    particleRate = P_VARYING_FLOAT;
    jet = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    Effect::EmitList(Efx);
}

void Swirl::PerFrame(ExecMode_e EM, EffectsManager& Efx)
{
    BounceBox(jet, djet, Efx.timeStep, 10);
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Swirl::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    jet = pVec(-4, 0, 2.4);
    djet = pRandVec() * 20.f;
    PrimType = PRIM_GAUSSIAN_SPRITE;
    WhiteBackground = true;
    DepthTest = false;
    MotionBlur = true;
    SortParticles = true;
}

// A tornado that tests the vortex action
// TODO: Change it to move around a bed of particles like JetSpray and suck them up
void Tornado::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Size(particleSize);
    S.Velocity(PDBlob(pVec(0.f), 0.5f));
    S.Color(PDLine(pVec(.0, .8, .8), pVec(1, 1, 1)));
    S.StartingAge(0);
    S.Size(particleSize);
    P.Source(particleRate, Render(PDBox(pVec(-7, 0, 12), pVec(7, 0, 16))), S);

    P.Damping(pVec(.95));
    P.Gravity(Efx.GravityVec);
    P.Vortex(Efx.center + pVec(0, 0, -5.f), pVec(0, 0, 11), 1.8f, 7.f, 80.f, -200.f, 1000.0f);
    P.Move(true, false);

    P.KillOld(particleLifetime);
    P.Sink(false, Render(PDPlane(pVec(0, 0, -2), pVec(0, 0, 1))));
}

void Tornado::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    PrimType = PRIM_SPHERE_SPRITE;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

// A waterfall bouncing off invisible rocks
void Waterfall::DoActions(EffectsManager& Efx)
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    float s = 0.3f;
    S.Velocity(PDBlob(pVec(3.f, -0.1f, 1.f) * s, 0.2f * s));
    S.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    S.Size(particleSize);
    P.Source(particleRate, Render(PDLine(pVec(-5, -1, 8), pVec(-5, 1, 8))), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0, 0.3, 0, Render(PDRectangle(pVec(-7, -2, 7), pVec(3, 0, 0), pVec(0, 4, 0))));
    P.Bounce(0, 0.5, 0, Render(PDSphere(pVec(-3.7, 1, 6), 0.5)));
    P.Bounce(0, 0.5, 0, Render(PDSphere(pVec(-3.5, 0, 2), 2)));
    P.Bounce(0, 0.5, 0, Render(PDSphere(pVec(3.8, 0, 0), 2)));
    P.Bounce(-0.01, 0.35, 0, Render(PDPlane(pVec(0, 0, 0), pVec(0, 0, 1))));
    P.Move(true, false);

    P.KillOld(particleLifetime);
    P.Sink(false, PDSphere(pVec(0, 0, 0), 25));
}

void Waterfall::StartEffect(EffectsManager& Efx)
{
    particleRate = Efx.maxParticles / particleLifetime;
    PrimType = PRIM_SPHERE_SPRITE;
    WhiteBackground = true;
    DepthTest = true;
    MotionBlur = false;
    SortParticles = false;
}

//////////////////////////////////////////////////////////////////////////////

EffectsManager::EffectsManager(ParticleContext_t& P_, int mp) : P(P_)
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

void EffectsManager::SetPhoto(uc3Image* Im)
{
    Img = Im;
    EASSERT(Img != NULL && Img->size() > 0);
}

// EM specifies how you want to run (for different benchmark purposes, mostly).
// Allowed values are Immediate_Mode, Internal_Mode, and Compiled_Mode.
// Set demoNum to -2 to let NextEffect choose the next demo.
void EffectsManager::ChooseDemo(int newDemoNum, ExecMode_e EM)
{
    EASSERT(EM == Immediate_Mode || EM == Internal_Mode || EM == Compiled_Mode);

    demoNum = newDemoNum;
    if (demoNum == -1) demoNum = getNumEffects() - 1;
    demoNum = demoNum % getNumEffects();

    if (demoNum < 0)
        if (Demo != NULL)
            demoNum = Demo->NextEffect(*this); // The effect will tell us what the next effect should be
        else
            demoNum = irand(getNumEffects());

    EASSERT(demoNum >= 0 && demoNum < getNumEffects());
    Demo = Effects[demoNum];

    std::cerr << Demo->GetName() << '\n';
    Demo->StartEffect(*this);
}

// EM specifies how you want to run (for different benchmark purposes, mostly).
// Allowed values are Immediate_Mode, Internal_Mode, and Compiled_Mode.
void EffectsManager::RunDemoFrame(ExecMode_e EM)
{
    EASSERT(EM == Immediate_Mode || EM == Internal_Mode || EM == Compiled_Mode);

    if (EM == Internal_Mode && Demo->CompiledFunc != NULL) {
        // User requested internal mode but the AL is set up for compiled mode; need to switch it to internal mode
        P.BindEmittedActionList(Demo->AList, NULL, P_INTERNAL_CODE);
    }

    Demo->PerFrame(EM, *this);

    if (EM == Internal_Mode && Demo->CompiledFunc != NULL) {
        // We switched the AL to internal mode. Now switch it back.
        P.BindEmittedActionList(Demo->AList, Demo->CompiledFunc, Demo->BIND_KIND);
    }
}

void EffectsManager::MakeEffects()
{
    Effects.push_back(std::shared_ptr<Effect>(new Atom(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Balloons(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Boids(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new BounceToy(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Explosion(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Fireflies(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Fireworks(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new FlameThrower(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Fountain(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new GridShape(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new JetSpray(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Orbit2(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new PhotoShape(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Rain(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Restore(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Shower(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Snake(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Sphere(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Swirl(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Tornado(*this)));
    Effects.push_back(std::shared_ptr<Effect>(new Waterfall(*this)));
}

void EffectsManager::MakeActionLists(ExecMode_e EM)
{
    if (EM != Immediate_Mode)
        for (auto e : Effects) { e->CreateList(EM, *this); }
}
