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

inline void BoxPoint(pVec& jet, pVec& djet, const float A)
{
    jet += djet;
    if (jet.x() > A || jet.x() < -A) {
        djet.x() *= -1.0f;
        djet.y() += pRandf() * 0.0005;
    }
    if (jet.y() > A || jet.y() < -A) {
        djet.y() *= -1.0f;
        djet.z() += pRandf() * 0.0005;
    }
    if (jet.z() > A || jet.z() < -A) {
        djet.z() *= -1.0f;
        djet.x() += pRandf() * 0.0005;
    }
}
} // namespace

ParticleEffects::ParticleEffects(ParticleContext_t& P_, int mp, E_RENDER_GEOMETRY RG) : P(P_), RenderGeometry(RG)
{
    Img = MakeFakeImage();
    maxParticles = mp;
    numSteps = 1;
    particleHandle = -1;
    GravityVec = pVec(0.0f, 0.0f, -0.01f);
    MakeEffects();
}

// EM specifies how you want to run (for different benchmark purposes, mostly).
// Allowed values are Immediate_Mode, Internal_Mode, and Compiled_Mode.
// Set DemoNum to -2 to let NextEffect choose the next demo.
int ParticleEffects::CallDemo(int DemoNum, ExecMode_e EM)
{
    if (DemoNum == -1) DemoNum = NumEffects - 1;
    DemoNum = DemoNum % NumEffects;

    if (DemoNum < 0 || Effects[DemoNum] != Demo) {
        // Transitioning, so call NextEffect() and StartEffect().
        if (DemoNum < 0 && Demo != NULL)
            DemoNum = Demo->NextEffect(*this); // The effect will tell us what the next effect should be
        else if (Demo == NULL)
            DemoNum = irand(NumEffects);

        EASSERT(DemoNum >= 0 && DemoNum < NumEffects);
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

void ParticleEffects::MakeEffects()
{
    Effects[0] = new Atom(*this);
    Effects[1] = new Balloons(*this);
    Effects[2] = new BounceToy(*this);
    Effects[3] = new Explosion(*this);
    Effects[4] = new Fireflies(*this);
    Effects[5] = new Fireworks(*this);
    Effects[6] = new FlameThrower(*this);
    Effects[7] = new Fountain(*this);
    Effects[8] = new GridShape(*this);
    Effects[9] = new JetSpray(*this);
    Effects[10] = new Orbit2(*this);
    Effects[11] = new PhotoShape(*this);
    Effects[12] = new Rain(*this);
    Effects[13] = new Restore(*this);
    Effects[14] = new Shower(*this);
    Effects[15] = new Snake(*this);
    Effects[16] = new Sphere(*this);
    Effects[17] = new Swirl(*this);
    Effects[18] = new Tornado(*this);
    Effects[19] = new WaterfallA(*this);
    Effects[20] = new WaterfallB(*this);
}

void ParticleEffects::MakeActionLists(ExecMode_e EM)
{
    for (int i = 0; i < NumEffects; i++) { Effects[i]->CreateList(EM, *this); }
}

//////////////////////////////////////////////////////////////////////////////

Effect::Effect()
{
    MotionBlur = false;
    DepthTest = false;
    KillAtStart = false;
    AList = -1;
    PrimType = 0; // GL_POINTS
    TexID = -1;
    ParticleSize = 1;

    CompiledFunc = NULL;
    BIND_KIND = P_INTERNAL_CODE;
}

void Effect::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    EASSERT(EM != Emit_Mode);

    ParticleContext_t& P = Efx.P;

    if (EM == Varying_Mode) P.NewActionList(AList);

    if (EM == Immediate_Mode || EM == Varying_Mode) DoActions(Efx);

    if (EM == Varying_Mode) P.EndActionList();

    if (EM == Internal_Mode || EM == Compiled_Mode || EM == Varying_Mode) P.CallActionList(AList);
}

// Non-varying effects call directly to here without a per-effect overload.
void Effect::CreateList(ExecMode_e EM, ParticleEffects& Efx)
{
    EASSERT(EM != Immediate_Mode);

    ParticleContext_t& P = Efx.P;
    if (AList < 0) AList = P.GenActionLists();
    P.NewActionList(AList);
    DoActions(Efx);
    P.EndActionList();
}

// Non-varying effects call directly to here without a per-effect overload.
void Effect::EmitList(ParticleEffects& Efx) { CreateList(Emit_Mode, Efx); }

int Effect::NextEffect(ParticleEffects& Efx) { return irand(Efx.NumEffects); }

void Effect::BindEmitted(ParticleEffects& Efx, P_PARTICLE_EMITTED_ACTION_LIST Func, EmitCodeParams_e Params)
{
    Efx.P.BindEmittedActionList(AList, Func, Params);
    CompiledFunc = Func;
    BIND_KIND = Params;
}

//////////////////////////////////////////////////////////////////////////////

// Particles orbiting a center
void Atom::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDSphere(pVec(0, 0, 0), 0.2));
    S.Color(PDBox(pVec(1, 0, 0), pVec(1, 0, 1)));
    S.Size(pVec(1.5));
    S.StartingAge(0);

    P.Source(particle_rate, PDSphere(pVec(0, 0, 0), 6), S);
    P.OrbitPoint(pVec(0, 0, 0), 0.05, 1.5);
    P.TargetColor(pVec(0, 1, 0), 1, 0.001);
    P.Move(true, false);

    // Keep orbits from being too eccentric.
    P.Sink(true, PDSphere(pVec(0.0, 0.0, 0.0), 1.0));
    P.Sink(false, PDSphere(pVec(0.0, 0.0, 0.0), 8.0));
}

void Atom::EmitList(ParticleEffects& Efx)
{
    particle_rate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Atom::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / 100.0f;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Atom::StartEffect(ParticleEffects& Efx) { particle_rate = Efx.maxParticles / 100.0f; }

// A bunch of balloons
void Balloons::DoActions(ParticleEffects& Efx) const
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
    float BBOX = 1.7;
    float x = 0, y = 0, z = -1;
    P.Source(particle_rate, PDBox(pVec(x - BBOX, y - BBOX, z - BBOX), pVec(x + BBOX, y + BBOX, z + BBOX)), S);

    P.Gravity(pVec(.0005, .005, .0005));
    P.Damping(pVec(0.9, 0.67, 0.9));
    float BOX = .005;
    P.RandomAccel(PDBox(pVec(-BOX, -BOX, -BOX), pVec(BOX, BOX, BOX)));
    P.Move(true, false);
    P.KillOld(700);
}

void Balloons::EmitList(ParticleEffects& Efx)
{
    particle_rate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Balloons::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / 100.0f;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Balloons::StartEffect(ParticleEffects& Efx) { particle_rate = Efx.maxParticles / 100.0f; }

void BounceToy::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;

    // Friction: 0 means don't slow its tangential velocity. Bigger than 0 means do.
    // Cutoff: If less than cutoff, don't apply friction.
    // Resilience: Scale normal velocity by this. Bigger is bouncier.
    const float Fric = 0.5f, Res = 0.50f, Cutoff = 0.07f;

    pSourceState S;
    S.Color(PDLine(pVec(1, 1, 0), pVec(0, 1, 0)));
    S.Velocity(PDDisc(pVec(0, 0, 0), pVec(0, 1, 0.1), 0.01f));
    P.Source(10, PDLine(pVec(-5, 0, 10), pVec(5, 0, 10)), S);

    P.Gravity(Efx.GravityVec);

    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-4, -2, 6), pVec(4, 0, 1), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(4, -2, 8), pVec(4, 0, -3), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-1, -2, 6), pVec(2, 0, -2), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(1, -2, 2), pVec(4, 0, 2), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-6, -2, 6), pVec(3, 0, -5), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(6, -2, 2), pVec(5, 0, 3), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(4, -2, -1), pVec(5, 0, 1.5), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-3, -2, -1), pVec(5, 0, -1), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-8, -2, -4.1), pVec(14, 0, 2), pVec(0, 4, 0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-10, -2, 5), pVec(4, 0, 5), pVec(0, 4, 0)));

    P.Jet(PDBox(pVec(-10, -2, -6), pVec(-7, 0, -1)), PDPoint(pVec(0.0, 0, .15)));
    P.TargetColor(pVec(0, 0, 1), 1, 0.004);
    P.Move(true, false);

    P.Sink(false, PDPlane(pVec(0, 0, -7), pVec(0, 0, 1)));
}

// An explosion from the center of the universe, followed by gravity toward a point
void Explosion::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    P.Damping(pVec(0.999));
    P.OrbitPoint(pVec(0, 0, 0), .02, 1.5);
    P.Explosion(pVec(0, 0, 0), time_since_start, 2, 3, 0.1);
    P.Move(true, false);
    P.Sink(false, PDSphere(pVec(0, 0, 0), 30));
}

void Explosion::EmitList(ParticleEffects& Efx)
{
    time_since_start = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Explosion::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    time_since_start += (1.0f / float(Efx.numSteps));
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Explosion::StartEffect(ParticleEffects& Efx) { time_since_start = 0; }

// Fireflies bobbing around
void Fireflies::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Size(pVec(1.0));
    S.Velocity(PDPoint(pVec(0, 0, 0)));
    S.Color(PDLine(pVec(.1, .5, 0), pVec(.9, .9, .1)));
    S.StartingAge(0);
    P.Source(1, PDBlob(pVec(0, 0, 2), 2), S);

    P.RandomAccel(PDSphere(pVec(0, 0, 0.00001), 0.002));
    P.Move(true, false);

    P.KillOld(600);
}

// Rocket-style fireworks
void Fireworks::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;

    /////////////////////////////////////////
    // The action list for moving the sparks
    pSourceState S;
    S.StartingAge(0, 6);
    S.Velocity(PDBlob(pVec(0.f), 0.006f));

    // For emitting we should have a constant max number of rockets and vary their params.
    for (int i = 0; i < MaxRockets; i++) {
        // cerr << i << " " <<rocketp[i][0]<<" "<<rocketp[i][1]<<" "<<rocketp[i][2]<<"\n";
        // cerr << "c " <<rocketc[i][0]<<" "<<rocketc[i][1]<<" "<<rocketc[i][2]<<"\n";

        S.Color(PDLine(rocketc[i], pVec(1, .5, .5)));
        P.Source((i < NumRockets) ? particle_rate : 0, PDPoint(rocketp[i]), S);
    }

    P.Gravity(Efx.GravityVec);
    // P.Damping(pVec(0.999));
    P.TargetColor(pVec(0, 0, 0), 0, 0.01);
    P.Move(true, false);
    P.KillOld(Lifetime);
}

void Fireworks::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    ParticleContext_t& P = Efx.P;

    /////////////////////////////////////////
    // The actions for moving the rockets.
    P.CurrentGroup(RocketGroup);

    pSourceState S;
    S.Velocity(PDCylinder(pVec(0, 0, 0.3), pVec(0, 0, 0.5), 0.11, 0.07));
    S.Color(PDBox(pVec(0, 0.5, 0), pVec(1, 1, 1)));
    P.Source(1, PDDisc(pVec(0, 0, 0), pVec(0, 0, 1), 6), S);

    P.Sink(false, PDPlane(pVec(0, 0, -1), pVec(0, 0, 1)));
    P.Gravity(Efx.GravityVec);
    P.Move(true, false);

    // Read back the position of the rockets.
    NumRockets = (int)P.GetParticles(0, MaxRockets, (float*)rocketp, (float*)rocketc);

    if (Efx.particleHandle >= 0) P.CurrentGroup(Efx.particleHandle);

    particle_rate = (Efx.maxParticles * 2.0f) / float(Lifetime * MaxRockets);

    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Fireworks::EmitList(ParticleEffects& Efx)
{
    NumRockets = MaxRockets;
    for (int i = 0; i < NumRockets; i++) {
        rocketp[i] = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
        rocketc[i] = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    }
    particle_rate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Fireworks::StartEffect(ParticleEffects& Efx)
{
    if (RocketGroup == -1)
        RocketGroup = Efx.P.GenParticleGroups(1, MaxRockets);
    else {
        Efx.P.CurrentGroup(RocketGroup);
        Efx.P.KillOld(-1000);
        if (Efx.particleHandle >= 0) Efx.P.CurrentGroup(Efx.particleHandle);
    }
    particle_rate = (Efx.maxParticles * 2.0f) / float(Lifetime * MaxRockets);
}

// It's like a flame thrower spinning around
void FlameThrower::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(PDLine(pVec(0.8, 0, 0), pVec(1, 1, 0.3)));
    S.Velocity(
        PDBlob(pVec(dirAng == P_VARYING_FLOAT ? P_VARYING_FLOAT : sin(dirAng) * .8, dirAng == P_VARYING_FLOAT ? P_VARYING_FLOAT : cos(dirAng) * .8, 0), 0.03));
    S.StartingAge(0);
    S.Size(pVec(1));
    P.Source(particle_rate, PDDisc(pVec(0, 0, 2), pVec(0, 0, 1), 0.5), S);

    P.Gravity(pVec(0, 0, .01));
    P.Damping(pVec(0.9, 0.97, 0.9));

    const float BOX = .017;
    P.RandomAccel(PDBox(pVec(-BOX, -BOX, -BOX), pVec(BOX, BOX, BOX)));

    P.Move(true, false);
    P.KillOld(Lifetime);
}

void FlameThrower::EmitList(ParticleEffects& Efx)
{
    particle_rate = P_VARYING_FLOAT;
    dirAng = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void FlameThrower::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    dirAng += 0.02;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void FlameThrower::StartEffect(ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / (float)Lifetime;
    dirAng = 0;
}

// A fountain spraying up in the middle of the screen
void Fountain::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDCylinder(pVec(0.0, -0.01, 0.35), pVec(0.0, -0.01, 0.37), 0.021, 0.019));
    S.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    P.Source(particle_rate, PDLine(pVec(0.0, 0.0, 0.0), pVec(0.0, 0.0, 0.405)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(-0.05, 0.35, 0, PDDisc(pVec(0, 0, 0), pVec(0, 0, 1), 5));
    P.Move(true, false);

    P.Sink(false, PDPlane(pVec(0, 0, -3), pVec(0, 0, 1)));
    P.SinkVelocity(true, PDSphere(pVec(0, 0, 0), 0.01));
}

void Fountain::EmitList(ParticleEffects& Efx)
{
    particle_rate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Fountain::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / 100.0f;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Fountain::StartEffect(ParticleEffects& Efx) { particle_rate = Efx.maxParticles / 100.0f; }

// A bunch of particles in a grid shape
void GridShape::DoActions(ParticleEffects& Efx) const
{
    // XXX Doesn't do anything. Just sets up initial particle placement
}

void GridShape::StartEffect(ParticleEffects& Efx)
{
    ParticleContext_t& P = Efx.P;

    pSourceState S;
    S.Velocity(PDBlob(pVec(0, 0, 0), 0.001));
    P.KillOld(-100000);
    int dim = int(powf(float(Efx.maxParticles), 0.33333333f));

    const float XX = 8;
    const float YY = 12;
    const float ZZ = 9;

    float dx = 2 * XX / float(dim);
    float dy = 2 * YY / float(dim);
    float dz = 2 * ZZ / float(dim);

    float z = -ZZ;
    for (int k = 0; k < dim; k++, z += dz) {
        float y = -YY;
        for (int l = 0; l < dim; l++, y += dy) {
            float x = -XX;
            for (int j = 0; j < dim; j++, x += dx) {
                // Make the particles.
                S.Color(j / float(dim), k / float(dim), l / float(dim));
                S.StartingAge(0);
                P.Vertex(pVec(x, y, z), S);
            }
        }
    }
}

// It's like a fan cruising around under a floor, blowing up on some ping pong balls.
// Like you see in real life.
void JetSpray::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0, 0, 0), 0.01));
    S.Size(pVec(1.5));
    S.Color(PDSphere(pVec(.8, .4, .1), .1));
    P.Source(1, PDRectangle(pVec(-1, -1, 0.1), pVec(2, 0, 0), pVec(0, 2, 0)), S);

    S.Color(PDSphere(pVec(.5, .4, .1), .1));
    P.Source(300, PDRectangle(pVec(-10, -10, 0.1), pVec(20, 0, 0), pVec(0, 20, 0)), S);

    P.Gravity(Efx.GravityVec);

    P.Jet(PDSphere(jet, 1.5), PDBlob(pVec(0, 0, .05), 0.01));
    P.Bounce(0.1, 0.3, 0.1, PDRectangle(pVec(-10, -10, 0.0), pVec(20, 0, 0), pVec(0, 20, 0)));
    P.Sink(false, PDPlane(pVec(0, 0, -20), pVec(0, 0, 1)));
    P.Move(true, false);
}

void JetSpray::EmitList(ParticleEffects& Efx)
{
    jet = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    Effect::EmitList(Efx);
}

void JetSpray::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    BoxPoint(jet, djet, 10);
    djet.z() = 0;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void JetSpray::StartEffect(ParticleEffects& Efx)
{
    jet = pVec(0.f);
    djet = pRandVec() * 0.5f;
    djet.z() = 0.0f;
}

// A sprayer with particles that orbit two points
void Orbit2::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.02, -0.2, 0), 0.015));
    S.Size(pVec(1.0));
    const pVec tjet = Abs(jet) * 0.1 + pVec(0.4, 0.4, 0.4);
    S.Color(PDSphere(tjet, 0.1));
    P.Source(particle_rate, PDPoint(jet), S);

    P.OrbitPoint(pVec(2, 0, 3), 0.1, 1.5);
    P.OrbitPoint(pVec(-2, 0, -3), 0.1, 1.5);
    P.Damping(pVec(0.994));
    P.Move(true, false);

    P.KillOld(Lifetime);
}

void Orbit2::EmitList(ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / (float)Lifetime;
    particle_rate = P_VARYING_FLOAT;
    jet = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    Effect::EmitList(Efx);
}

void Orbit2::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    BoxPoint(jet, djet, 10);
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Orbit2::StartEffect(ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / (float)Lifetime;
    jet = pVec(-4, 0, -2.4);
    djet = pRandVec() * 0.5f;
}

// A bunch of particles in the shape of a photo
void PhotoShape::DoActions(ParticleEffects& Efx) const
{
    // XXX Doesn't do anything. Just sets up initial particle placement
}

void PhotoShape::StartEffect(ParticleEffects& Efx)
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

            P.Vertex(v * 6.0f - pVec(3.0f), S);
        }
    }
}

// It kinda looks like rain hitting a parking lot
void Rain::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(pVec(0.f));
    S.Color(PDSphere(pVec(0.4, 0.4, 0.9), .1));
    S.Size(pVec(1.5));
    S.StartingAge(0);
    float D = 80;
    P.Source(particle_rate, PDRectangle(pVec(-D / 2, -D / 2, 12), pVec(D, 0, 0), pVec(0, D, 0)), S);

    P.RandomAccel(PDBlob(pVec(0.002, 0, -0.01), 0.003));
    P.Bounce(0.3, 0.3, 0, PDPlane(pVec(0, 0, 0), pVec(0, 0, 1)));
    P.Move(true, false);

    P.KillOld(Lifetime);
}

void Rain::EmitList(ParticleEffects& Efx)
{
    particle_rate = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Rain::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / (float)Lifetime;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Rain::StartEffect(ParticleEffects& Efx) { particle_rate = Efx.maxParticles / (float)Lifetime; }

// Restore particles to their PositionB and UpVecB.
void Restore::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    P.Restore(time_left);
    P.Move(true, false);
}

void Restore::EmitList(ParticleEffects& Efx)
{
    time_left = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Restore::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    time_left -= (1.0f / float(Efx.numSteps));
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Restore::StartEffect(ParticleEffects& Efx) { time_left = 200; }

// A sheet of particles falling down, avoiding various-shaped obstacles
void Shower::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.f), 0.001f));
    S.Size(pVec(1.5));
    S.StartingAge(0);
    S.Color(PDBlob(pVec(.7, .7, .2), .2));
    P.Source(50, PDPoint(jet), S);

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

    P.KillOld(130);
}

void Shower::EmitList(ParticleEffects& Efx)
{
    SteerShape = P_VARYING_INT;
    jet = pVec(P_VARYING_FLOAT, P_VARYING_FLOAT, P_VARYING_FLOAT);
    Effect::EmitList(Efx);
}

void Shower::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    BoxPoint(jet, djet, 2);
    jet += djet;
    djet.z() = 0;

    if (Efx.RenderGeometry != NULL) Efx.RenderGeometry(SteerShape);

    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Shower::StartEffect(ParticleEffects& Efx)
{
    SteerShape = irand(4);
    jet = pVec(0, 0, 5);
    djet = pRandVec() * 0.01f;
    djet.z() = 0.0f;
}

// A bunch of particles in a line that are attracted to the one ahead of them in line
void Snake::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(1, 0, 0);
    S.Velocity(pVec(0.001, 0, 0)); // This makes it able to compute a binormal.
    float BOX = .005;
    P.Source(0.1, PDBox(pVec(-BOX, -BOX, -BOX), pVec(BOX, BOX, BOX)), S);

    // Either of these gives an interesting effect.
    P.Follow(0.01, 1.0);
    // P.Gravitate(0.01, 1.0);
    P.Damping(pVec(0.9));
    P.Move(true, false);
    P.KillOld(2000);
}

void Snake::StartEffect(ParticleEffects& Efx)
{
    ParticleContext_t& P = Efx.P;

    pSourceState S;
    S.Velocity(pVec(0.f));
    S.Size(pVec(1.0));
    S.Color(PDSphere(pVec(0.93, 0.93, 0), 0.05));

    for (float x = -10.0; x < 2.0; x += 0.05) P.Vertex(pVec(x, 0, 0), S);
}

// A bunch of particles inside a sphere
void Sphere::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Color(PDLine(pVec(0, 1, 0), pVec(0, 0, 1)));
    S.Velocity(PDBlob(pVec(dirAng == P_VARYING_FLOAT ? P_VARYING_FLOAT : sin(dirAng) * .1, dirAng == P_VARYING_FLOAT ? P_VARYING_FLOAT : cos(dirAng) * .1, 0.1),
                      0.01));
    S.StartingAge(0);
    S.Size(pVec(1));
    P.Source(50, PDPoint(pVec(1, 1, 6)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0, 0.55, 0, PDSphere(pVec(0, 0, 4), 6));
    P.Move(true, false);

    P.KillOld(1000);
}

void Sphere::EmitList(ParticleEffects& Efx)
{
    dirAng = P_VARYING_FLOAT;
    Effect::EmitList(Efx);
}

void Sphere::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    dirAng += 0.02;
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Sphere::StartEffect(ParticleEffects& Efx) { dirAng = 0; }

// A sprayer with particles orbiting a line
void Swirl::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    const pVec tjet = Abs(jet) * 0.1 + pVec(0.4, 0.4, 0.4);
    S.Color(PDSphere(tjet, 0.1));
    S.Velocity(PDBlob(pVec(0.02, -0.2, 0), 0.015));
    S.Size(pVec(1.0));
    S.StartingAge(0);
    P.Source(particle_rate, PDPoint(jet), S);

    P.OrbitLine(pVec(0, 0, 1), pVec(1, 0.1, 0), 0.1, 1.5);
    P.Damping(pVec(1, 0.994, 0.994));
    P.Move(true, false);

    P.Sink(false, PDSphere(pVec(0, 0, 0), 15));
    P.KillOld(Lifetime);
}

void Swirl::EmitList(ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / (float)Lifetime;
    Effect::EmitList(Efx);
}

void Swirl::PerFrame(ExecMode_e EM, ParticleEffects& Efx)
{
    particle_rate = Efx.maxParticles / (float)Lifetime;
    BoxPoint(jet, djet, 10);
    Effect::PerFrame(EM == Immediate_Mode ? EM : Varying_Mode, Efx);
}

void Swirl::StartEffect(ParticleEffects& Efx)
{
    jet = pVec(-4, 0, -2.4);
    djet = pRandVec() * 0.05;
    particle_rate = Efx.maxParticles / Lifetime;
}

// A tornado that tests the vortex action
void Tornado::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Size(pVec(1.0));
    S.Velocity(PDPoint(pVec(0, 0, 0)));
    S.Color(PDLine(pVec(.0, .8, .8), pVec(1, 1, 1)));
    S.StartingAge(0);
    P.Source(10, PDLine(pVec(-10, 0, 15), pVec(10, 0, 15)), S);

    P.Damping(pVec(.95));
    P.Gravity(Efx.GravityVec);
    P.Vortex(pVec(0, 0, -2), pVec(0, 0, 11), 1.8, 7, 0.05, -Efx.GravityVec.length(), 0.1);
    P.Move(true, false);

    P.KillOld(1000);
    P.Sink(false, PDPlane(pVec(0, 0, -2), pVec(0, 0, 1)));
}

// A waterfall bouncing off invisible rocks
void WaterfallA::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.03, -0.001, 0.01), 0.002));
    S.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    S.Size(pVec(1.5));
    P.Source(100, PDLine(pVec(-5, -1, 8), pVec(-5, -3, 8)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0, 0.35, 0, PDRectangle(pVec(-7, -4, 7), pVec(3, 0, 0), pVec(0, 3, 0)));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(-4, -2, 4), 0.2));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(-3.5, 0, 2), 2));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(3.8, 0, 0), 2));
    P.Bounce(-0.01, 0.35, 0, PDPlane(pVec(0, 0, 0), pVec(0, 0, 1)));
    P.Move(true, false);

    P.KillOld(300);
    P.Sink(false, PDSphere(pVec(0, 0, 0), 20));
}

// A waterfall bouncing off invisible rocks
void WaterfallB::DoActions(ParticleEffects& Efx) const
{
    ParticleContext_t& P = Efx.P;
    pSourceState S;
    S.Velocity(PDBlob(pVec(0.1, 0, 0.1), 0.004));
    S.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    S.Size(pVec(1.5));
    P.Source(100, PDPoint(pVec(-4, 0, 6)), S);

    P.Gravity(Efx.GravityVec);
    P.Bounce(0, 0.01, 0, PDSphere(pVec(-1, 0, 4), 1));
    P.Bounce(0, 0.01, 0, PDSphere(pVec(-2.5, 0, 2), 1));
    P.Bounce(0, 0.01, 0, PDSphere(pVec(0.7, -0.5, 2), 1));
    P.Bounce(-0.01, 0.35, 0, PDPlane(pVec(0, 0, 0), pVec(0, 0, 1)));
    P.Move(true, false);

    P.KillOld(250);
}

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
