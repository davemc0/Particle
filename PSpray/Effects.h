#ifndef _Effects_h
#define _Effects_h

#include <Image/tImage.h>

#include <Particle/pAPI.h>

using namespace PAPI;

class ParticleEffects
{
private:
    char *EffectName;
    uc3Image *Img; // For the PhotoShape effect
    pVec GravityVec;
    ParticleContext_t &P;
    bool ChangesEachFrame; // True if we need to call back into the effect each frame to make a new action list.

    void MakeFakeImage();

public:
    static const int NumEffects = 23;

    int maxParticles; // The number of particles the app wants in this demo
    int numSteps; // The number of simulation time steps per rendered frame
    int particle_handle; // The handle of the particle group
    int action_handle; // The handle of the action list being created or used

    void SetPhoto(uc3Image *Im) {Img = Im; if(Img == NULL || Img->size() < 1) std::cerr << "Bad image.\n";}
    char *GetCurEffectName() {return EffectName;}

    ParticleEffects(ParticleContext_t &_P, int mp = 100);

    // Call this to get a demo by number.
    // Returns the DemoNum % num_demos.
    int CallDemo(int DemoNum, bool FirstTime, bool immediate);

    // Particles orbiting a center
    void Atom(bool FirstTime, bool Immediate);

    // A bunch of balloons
    void Balloons(bool FirstTime, bool Immediate);

    // Particles fall in from the top and bounce off panels
    void BounceToy(bool FirstTime, bool Immediate);

    // Whatever junk I want to throw in here
    void Experimental(bool FirstTime, bool Immediate);

    // An explosion from the center of the universe, followed by gravity
    void Explosion(bool FirstTime, bool Immediate);

    // Fireflies bobbing around
    void Fireflies(bool FirstTime, bool Immediate);

    // Rocket-style fireworks
    void Fireworks(bool FirstTime, bool Immediate);

    // It's like a flame thrower spinning around
    void FlameThrower(bool FirstTime, bool Immediate);

    // A fountain spraying up in the middle of the screen
    void Fountain(bool FirstTime, bool Immediate);

    // A bunch of particles in a grid shape
    void GridShape(bool FirstTime, bool Immediate);

    // It's like a fan cruising around under a floor, blowing up on some ping pong balls
    void JetSpray(bool FirstTime, bool Immediate);

    // A sprayer with particles that orbit two points
    void Orbit2(bool FirstTime, bool Immediate);

    // A bunch of particles in the shape of a photo
    void PhotoShape(bool FirstTime, bool Immediate);

    // It kinda looks like rain hitting a parking lot
    void Rain(bool FirstTime, bool Immediate);

    // Restore particles to their positionB.
    void Restore(bool FirstTime, bool Immediate);

    // A sheet of particles falling down, avoiding various-shaped obstacles
    // SteerShape is one of PDSphere, PDTriangle, PDRectangle
    void Shower(bool FirstTime, bool Immediate, int SteerShape);

    // A bunch of particles in a line that are attracted to the guy ahead of them in line
    void Snake(bool FirstTime, bool Immediate);

    // A bunch of particles inside a sphere
    void Sphere(bool FirstTime, bool Immediate);

    // A sprayer with particles orbiting a line
    void Swirl(bool FirstTime, bool Immediate);

    // A waterfall bouncing off invisible rocks
    void Waterfall1(bool FirstTime, bool Immediate);

    // A waterfall bouncing off invisible rocks
    void Waterfall2(bool FirstTime, bool Immediate);
};

#endif
