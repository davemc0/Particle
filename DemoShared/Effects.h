#ifndef Effects_h
#define Effects_h

#include "Image/tImage.h"
#include "Particle/pAPI.h"

#include <string>

using namespace PAPI;

#define EASSERT(x)                                           \
    {                                                        \
        if (!(x)) { throw PErrInternalError("Bad effect"); } \
    }

class EffectsManager;

// Execution modes
enum ExecMode_e {
    Immediate_Mode, // Apply the actions to the particles right now
    Internal_Mode,  // Build an action list once and call it each frame
    Compiled_Mode,  // Execute a compiled action list
    Varying_Mode,   // Create the action list each frame
    Emit_Mode       // Build an action list for emitting
};

class Effect {
protected:
    bool MotionBlur;    // True to enable motion blur when rendering this effect
    bool DepthTest;     // True to enable depth test when rendering this effect
    bool KillAtStart;   // True to kill existing particles when switching to this effect
    int PrimType;       // Which kind of primitive is best for this particle
    int TexID;          // What texture to apply to primitives for this effect
    float ParticleSize; // How big of particles for this effect

public:
    int AList;                                   // The action list handle
    P_PARTICLE_EMITTED_ACTION_LIST CompiledFunc; // Remember how we bound this so we can unbind/rebind it when the app wants to run differently
    EmitCodeParams_e BIND_KIND;

    Effect();

    void CreateList(ExecMode_e EM, EffectsManager& Efx);
    void BindEmitted(EffectsManager& Efx, P_PARTICLE_EMITTED_ACTION_LIST, EmitCodeParams_e);
    virtual const std::string GetName() const = 0;
    virtual void DoActions(EffectsManager& Efx) const = 0;
    virtual void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    virtual void EmitList(EffectsManager& Efx);
    virtual void StartEffect(EffectsManager& Efx) {} // Initialize internal variables and set up anything else for the start of this effect
    virtual int NextEffect(EffectsManager& Efx);     // Some demos are only interesting when followed by certain other ones. Return the next one to run.
};

// Particles orbiting a center
struct Atom : public Effect {
    float particle_rate;

    Atom(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Atom"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of balloons
struct Balloons : public Effect {
    float particle_rate;

    Balloons(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Balloons"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// Particles fall in from the top and bounce off panels
struct BounceToy : public Effect {
    BounceToy(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "BounceToy"; }
    void DoActions(EffectsManager& Efx) const;
};

// An explosion from the center of the universe, followed by gravity
struct Explosion : public Effect {
    float time_since_start;

    Explosion(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Explosion"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// Fireflies bobbing around
struct Fireflies : public Effect {
    Fireflies(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Fireflies"; }
    void DoActions(EffectsManager& Efx) const;
};

// Rocket-style fireworks
struct Fireworks : public Effect {
    static const int MaxRockets = 16;
    static const int Lifetime = 25;
    float particle_rate;
    int RocketGroup, NumRockets;
    pVec rocketp[MaxRockets], rocketc[MaxRockets];

    Fireworks(EffectsManager& Efx)
    {
        RocketGroup = -1;
        StartEffect(Efx);
    }
    const std::string GetName() const { return "Fireworks"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// It's like a flame thrower spinning around
struct FlameThrower : public Effect {
    static const int Lifetime = 100;
    float dirAng, particle_rate;

    FlameThrower(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "FlameThrower"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A fountain spraying up in the middle of the screen
struct Fountain : public Effect {
    float particle_rate;

    Fountain(EffectsManager& Efx) { StartEffect(Efx); }
    void EmitList(EffectsManager& Efx);
    const std::string GetName() const { return "Fountain"; }
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of particles in a grid shape
struct GridShape : public Effect {
    GridShape(EffectsManager& Efx) {}
    const std::string GetName() const { return "GridShape"; }
    void DoActions(EffectsManager& Efx) const;
    void StartEffect(EffectsManager& Efx);
};

// It's like a fan cruising around under a floor, blowing up on some ping pong balls
struct JetSpray : public Effect {
    pVec jet, djet;

    JetSpray(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "JetSpray"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A sprayer with particles that orbit two points
struct Orbit2 : public Effect {
    static const int Lifetime = 350;
    float particle_rate;
    pVec jet, djet;

    Orbit2(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Orbit2"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of particles in the shape of a photo
struct PhotoShape : public Effect {
    PhotoShape(EffectsManager& Efx) {}
    const std::string GetName() const { return "PhotoShape"; }
    void DoActions(EffectsManager& Efx) const;
    void StartEffect(EffectsManager& Efx);

private:
};

// It kinda looks like rain hitting a parking lot
struct Rain : public Effect {
    static const int Lifetime = 100;
    float particle_rate;

    Rain(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Rain"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// Restore particles to their positionB.
struct Restore : public Effect {
    float time_left;

    Restore(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Restore"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A sheet of particles falling down, avoiding a domain obstacle
struct Shower : public Effect {
    pVec jet, djet;
    int SteerShape;

    Shower(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Shower"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of particles in a line that are attracted to the one ahead of them in line
struct Snake : public Effect {
    Snake(EffectsManager& Efx) {}
    const std::string GetName() const { return "Snake"; }
    void DoActions(EffectsManager& Efx) const;
    void StartEffect(EffectsManager& Efx);
};

// A bunch of particles inside a sphere
struct Sphere : public Effect {
    float dirAng;

    Sphere(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Sphere"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A sprayer with particles orbiting a line
struct Swirl : public Effect {
    static const int Lifetime = 300;
    pVec jet, djet;
    float particle_rate;

    Swirl(EffectsManager& Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Swirl"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx) const;
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A tornado just twirling and twisting everything it sees
struct Tornado : public Effect {
    Tornado(EffectsManager& Efx) {}
    const std::string GetName() const { return "Tornado"; }
    void DoActions(EffectsManager& Efx) const;
};

// A waterfall bouncing off invisible rocks
struct WaterfallA : public Effect {
    WaterfallA(EffectsManager& Efx) {}
    const std::string GetName() const { return "WaterfallA"; }
    void DoActions(EffectsManager& Efx) const;
};

// Another waterfall bouncing off invisible rocks
struct WaterfallB : public Effect {
    WaterfallB(EffectsManager& Efx) {}
    const std::string GetName() const { return "WaterfallB"; }
    void DoActions(EffectsManager& Efx) const;
};

//////////////////////////////////////////////////////////////////////////////

typedef void (*E_RENDER_GEOMETRY)(const int SteerShape);

class EffectsManager {
public:
    uc3Image* Img; // For the PhotoShape effect

    static const int NumEffects = 21;
    Effect* Effects[NumEffects];

    ParticleContext_t& P;
    E_RENDER_GEOMETRY RenderGeometry;
    pVec GravityVec;
    Effect* Demo;
    int maxParticles;     // The number of particles the app wants in this demo
    int simStepsPerFrame; // The number of simulation time steps per rendered frame
    float timeStep;       // Dt, duration of time step (after acounting for simStepsPerFrame)
    int particleHandle;   // The handle of the particle group
    pVec worldCenter = {0.f, 0.f, 3.f};

    void SetPhoto(uc3Image* Im)
    {
        Img = Im;
        EASSERT(Img != NULL && Img->size() > 0);
    }
    const std::string GetCurEffectName() { return Demo->GetName(); }

    EffectsManager(ParticleContext_t& P_, int mp = 100, E_RENDER_GEOMETRY RG = NULL);

    // Call this to get a demo by number.
    // Returns the DemoNum % num_demos.
    int CallDemo(int DemoNum, ExecMode_e EM);

    void MakeEffects();
    void MakeActionLists(ExecMode_e EM);
};

#endif
