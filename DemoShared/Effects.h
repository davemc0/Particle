#ifndef Effects_h
#define Effects_h

#include "Image/tImage.h"
#include "Particle/pAPI.h"

#include <memory>
#include <string>

using namespace PAPI;

#define EASSERT(x)                                           \
    {                                                        \
        if (!(x)) { throw PErrInternalError("Bad effect"); } \
    }

class EffectsManager;

// Execution modes:
// 1) Immediate mode - just do the actions each frame; the only mode that works with varying inputs right now
// 2) Internal mode  - compile an action list at program start and call it each frame
// 3) Compiled mode  - set it to Emit_Mode at program start to compile the action lists,
//                     then per frame set it to Varying_Mode and do the actions to grab the current values of all the varying parameters
//                     then bind the compiled program and call it in Compiled_Mode to run it with the varying parameters
enum ExecMode_e {
    Immediate_Mode, // Apply the actions to the particles right now with no action list
    Internal_Mode,  // Build an action list once at program start and call it each frame
    Compiled_Mode,  // Execute a compiled action list
    Varying_Mode,   // Create the action list each frame just for passing the varying structure into the previously compiled action list
    Emit_Mode       // Build an action list for emitting
};

enum SteerShape_e { STEER_SPHERE, STEER_TRIANGLE, STEER_RECTANGLE, STEER_PLANE, STEER_DISC, STEER_VARYING };

enum PrimType_e { PRIM_GAUSSIAN_SPRITE, PRIM_SPHERE_SPRITE, PRIM_QUAD, PRIM_POINT, PRIM_LINE, PRIM_DISPLAY_LIST, PRIM_NONE, PRIM_TYPE_COUNT };
static char const* PrimTypeNames[] = {"PRIM_GAUSSIAN_SPRITE", "PRIM_SPHERE_SPRITE", "PRIM_QUAD", "PRIM_POINT", "PRIM_LINE", "PRIM_DISPLAY_LIST", "PRIM_NONE"};

class Effect {
protected:
    PrimType_e PrimType;     // Which kind of primitive is best for this particle
    bool WhiteBackground;    // True for white; false for black
    bool DepthTest;          // True to enable depth test when rendering this effect
    bool MotionBlur;         // True to enable motion blur when rendering this effect
    bool SortParticles;      // True to sort particles each frame for rendering
    bool UseRenderingParams; // True to suggest paying attention to the above rendering preferences
    float particleLifetime;  // Time in seconds the average particle will live
    float particleRate;      // Particles to create per second
    float particleSize;      // How big of particles for this effect

public:
    int AList;                                   // The action list handle
    P_PARTICLE_EMITTED_ACTION_LIST CompiledFunc; // Remember how we bound this so we can unbind/rebind it when the app wants to run differently
    EmitCodeParams_e BIND_KIND;
    std::vector<std::shared_ptr<pDomain>> Renderables; // A list of domains to render in the app

    Effect(EffectsManager& Efx);

    void CreateList(ExecMode_e EM, EffectsManager& Efx);
    void BindEmitted(EffectsManager& Efx, P_PARTICLE_EMITTED_ACTION_LIST, EmitCodeParams_e);
    const pDomain& Effect::Render(const pDomain& dom);

    virtual const std::string GetName() const = 0;
    virtual void DoActions(EffectsManager& Efx) = 0;           // Call the actions the go in the action list and get executed per frame
    virtual void PerFrame(ExecMode_e EM, EffectsManager& Efx); // Set varying state to values for current frame
    virtual void EmitList(EffectsManager& Efx);                // Set varying state to VARYING for emitting the action list as code
    virtual void StartEffect(EffectsManager& Efx) {}           // Initialize internal variables and set up anything else for the start of this effect
    virtual int NextEffect(EffectsManager& Efx);               // Specify next demo to run in case only a particular one is interesting

    PrimType_e getPrimType() { return PrimType; }               // Which kind of primitive is best for this particle
    bool getDepthTest() { return DepthTest; }                   // True to enable depth test when rendering this effect
    bool getMotionBlur() { return MotionBlur; }                 // True to enable motion blur when rendering this effect
    bool getSortParticles() { return SortParticles; }           // True to sort particles each frame for rendering
    bool getUseRenderingParams() { return UseRenderingParams; } // True to suggest paying attention to the above rendering preferences
};

// Particles orbiting a center
struct Atom : public Effect {
    Atom(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Atom"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of balloons
struct Balloons : public Effect {
    Balloons(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Balloons"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// Particles fall in from the top and bounce off panels
struct BounceToy : public Effect {
    BounceToy(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "BounceToy"; }
    void DoActions(EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// An explosion from the center of the universe, followed by gravity
struct Explosion : public Effect {
    float time_since_start;

    Explosion(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Explosion"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// Fireflies bobbing around
struct Fireflies : public Effect {
    Fireflies(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Fireflies"; }
    void DoActions(EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// Rocket-style fireworks
struct Fireworks : public Effect {
    static const int MaxRockets = 20;
    int RocketGroup = -1, NumRockets; // Use separate particle system for rockets
    pVec rocketPos[MaxRockets], rocketColor[MaxRockets];

    Fireworks(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Fireworks"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// It's like a flame thrower spinning around
struct FlameThrower : public Effect {
    float dirAng;

    FlameThrower(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "FlameThrower"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A fountain spraying up in the middle of the screen
struct Fountain : public Effect {
    Fountain(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    void EmitList(EffectsManager& Efx);
    const std::string GetName() const { return "Fountain"; }
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of particles in a grid shape
struct GridShape : public Effect {
    GridShape(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "GridShape"; }
    void DoActions(EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// It's like a fan cruising around under a floor, blowing up on some ping pong balls
struct JetSpray : public Effect {
    pVec jet, djet;

    JetSpray(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "JetSpray"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A sprayer with particles that orbit two points
struct Orbit2 : public Effect {
    pVec jet, djet;

    Orbit2(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Orbit2"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of particles in the shape of a photo
struct PhotoShape : public Effect {
    PhotoShape(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "PhotoShape"; }
    void DoActions(EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);

private:
};

// It kinda looks like rain hitting a parking lot
struct Rain : public Effect {
    Rain(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Rain"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// Restore particles to their positionB.
struct Restore : public Effect {
    float time_left;

    Restore(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Restore"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A sheet of particles falling down, avoiding a domain obstacle
struct Shower : public Effect {
    pVec jet, djet;
    int SteerShape;

    Shower(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Shower"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of particles in a line that are attracted to the one ahead of them in line
struct Snake : public Effect {
    Snake(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Snake"; }
    void DoActions(EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A bunch of particles inside a sphere
struct Sphere : public Effect {
    float dirAng;

    Sphere(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Sphere"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A sprayer with particles orbiting a line
struct Swirl : public Effect {
    pVec jet, djet;

    Swirl(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Swirl"; }
    void EmitList(EffectsManager& Efx);
    void DoActions(EffectsManager& Efx);
    void PerFrame(ExecMode_e EM, EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A tornado just twirling and twisting everything it sees
struct Tornado : public Effect {
    Tornado(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Tornado"; }
    void DoActions(EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

// A waterfall bouncing off invisible rocks
struct Waterfall : public Effect {
    Waterfall(EffectsManager& Efx) : Effect(Efx) { StartEffect(Efx); }
    const std::string GetName() const { return "Waterfall"; }
    void DoActions(EffectsManager& Efx);
    void StartEffect(EffectsManager& Efx);
};

//////////////////////////////////////////////////////////////////////////////

class EffectsManager {
public:
    uc3Image* Img; // For the PhotoShape effect

    std::vector<Effect*> Effects;

    ParticleContext_t& P;
    pVec GravityVec;               // Gravity goes in this direction for all effects that use it
    pVec center = {0.f, 0.f, 5.f}; // Coordinates of the center of interest of particle effects
    Effect* Demo;                  // Pointer to the current demo
    int demoNum;                   // Index of the current demo
    int maxParticles;              // The number of particles the app wants in this demo
    int simStepsPerFrame;          // The number of simulation time steps per rendered frame
    float timeStep;                // Dt, duration of time step (after acounting for simStepsPerFrame)
    float demoRunSec;              // Seconds to run each demo before randomly changing
    int particleHandle;            // The handle of the particle group

    void SetPhoto(uc3Image* Im)
    {
        Img = Im;
        EASSERT(Img != NULL && Img->size() > 0);
    }
    const std::string GetCurEffectName() { return Demo->GetName(); }

    EffectsManager(ParticleContext_t& P_, int mp = 100);

    void ChooseDemo(int newDemoNum, ExecMode_e EM); // Choose a demo by number
    void RunDemoFrame(ExecMode_e EM);

    void MakeEffects();
    void MakeActionLists(ExecMode_e EM);
    int getNumEffects() { return (int)Effects.size(); }
};

#endif
