#include "Effects.h"

#include <Particle/pAPI.h>

using namespace PAPI;

#include <Image/ImageAlgorithms.h>

#ifndef NO_OGL_OBSTACLES
// This is for drawing the obstacles for Shower().
#include <GL/glut.h>
#endif

#include <iostream>

#ifdef WIN32
#define lrand48() ((rand() << 16) ^ rand())
#endif

ParticleEffects::ParticleEffects(ParticleContext_t &_P, int mp) : P(_P)
{
    maxParticles = mp;
    numSteps = 1;
    particle_handle = -1;
    action_handle = P.GenActionLists(1);
    EffectName = "NoEffect";
    Img = NULL;
    GravityVec = pVec(0.0f, 0.0f, -0.01f);
}

// Make an image for use by the PhotoShape effect if -photo wasn't specified on the command line
void ParticleEffects::MakeFakeImage()
{
    if(Img) delete Img;
    const int SZ = 64;
    const float MX = SZ * 0.5;
    const float MN = SZ * 0.25;
    Img = new uc3Image(SZ, SZ);
    for(int y=0; y<SZ; y++) {
        for(int x=0; x<SZ; x++) {
            float rad = sqrtf((x-SZ/2)*(x-SZ/2)+(y-SZ/2)*(y-SZ/2));
            (*Img).operator ()(x,y) = (rad < MX && rad > MN) ? uc3Pixel(255, 255, 0) : uc3Pixel(128, 0, 200);
        }
    }
}

int ParticleEffects::CallDemo(int DemoNum, bool FirstTime, bool Immediate)
{
    if(!Immediate && !FirstTime && !ChangesEachFrame) {
        P.CallActionList(action_handle);
        return DemoNum;
    }

    switch(DemoNum) {
    case  0: Atom(FirstTime, Immediate); break;
    case  1: Balloons(FirstTime, Immediate); break;
    case  2: Fireflies(FirstTime, Immediate); break;
    case  3: Fireworks(FirstTime, Immediate); break;
    case  4: Waterfall2(FirstTime, Immediate); break;
    case  5: JetSpray(FirstTime, Immediate); break;
    case  6: Fountain(FirstTime, Immediate); break;
    case  7: FlameThrower(FirstTime, Immediate); break;
    case  8: Rain(FirstTime, Immediate); break;
    case  9: GridShape(FirstTime, Immediate); break;
    case 10: Explosion(FirstTime, Immediate); break;
    case 11: PhotoShape(FirstTime, Immediate); break;
    case 12: Orbit2(FirstTime, Immediate); break;
    case 13: Restore(FirstTime, Immediate); break;
    case 14: Shower(FirstTime, Immediate, 0); break;
    case 15: BounceToy(FirstTime, Immediate); break;
    case 16: Shower(FirstTime, Immediate, 1); break;
    case 17: Swirl(FirstTime, Immediate); break;
    case 18: Shower(FirstTime, Immediate, 2); break;
    case 19: Snake(FirstTime, Immediate); break;
    case 20: Waterfall1(FirstTime, Immediate); break;
    case 21: Sphere(FirstTime, Immediate); break;
    case 22: Experimental(FirstTime, Immediate); break;
    default:
        while(DemoNum < 0) DemoNum+= NumEffects;
        return CallDemo(DemoNum % NumEffects, FirstTime, Immediate);
        break;
    }

    return DemoNum;
}

// Particles orbiting a center
void ParticleEffects::Atom(bool FirstTime, bool Immediate)
{
    P.Velocity(PDSphere(pVec(0, 0, 0), 0.2));
    P.Color(PDBox(pVec(0, 0, 0), pVec(1.0, 0.5, 0.5)));
    P.Size(1.5);
    P.StartingAge(0);

    if(FirstTime) {
        EffectName = "Atom";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    P.Source(maxParticles / 100, PDSphere(pVec(0, 0, 0), 6));

    // Orbit about the origin.
    P.OrbitPoint(pVec(0, 0, 0), 0.05, 1.5);

    P.Move(true, false);

    // Keep orbits from being too eccentric.
    P.Sink(true, PDSphere(pVec(0.0, 0.0, 0.0), 1.0));
    P.Sink(false, PDSphere(pVec(0.0, 0.0, 0.0), 8.0));

    if(FirstTime)
        P.EndActionList();
}

// A bunch of balloons
void ParticleEffects::Balloons(bool FirstTime, bool Immediate)
{
    if(FirstTime) {
        EffectName = "Balloons";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    float x=0, y=0, z=-1;

    int nPar = (int)P.GetGroupCount();
    float qty = (maxParticles - nPar) / 6.0f;    // Because of 6 colors
    qty /= 100.0f;
    float BBOX = 1.7;

    P.StartingAge(0, 5);
    P.Velocity(pVec(0));
    P.Color(1,0,0); // These attributes don't get stored in the particle list.
    P.Source(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

    P.Color(1,1,0);
    P.Source(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

    P.Color(0,1,0);
    P.Source(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

    P.Color(0,1,1);
    P.Source(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

    P.Color(0,0,1);
    P.Source(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

    P.Color(1,0,1);
    P.Source(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

    P.Gravity(pVec(.0005, .005, .0005));

    P.Damping(pVec(0.9, 0.67, 0.9));

    float BOX = .005;

    P.RandomAccel(PDBox(pVec(-BOX, -BOX, -BOX), pVec(BOX, BOX, BOX)));

    P.Move(true, false);

    P.KillOld(500);

    if(FirstTime)
        P.EndActionList();
}

// An explosion from the center of the universe, followed by gravity toward a point
void ParticleEffects::Explosion(bool FirstTime, bool Immediate)
{
    static float i=0;
    if(FirstTime) {
        EffectName = "Explosion";
        ChangesEachFrame = true;
        i = 0;
    }

    if(!Immediate)
        P.NewActionList(action_handle);

    P.Damping(pVec(0.999));
    P.OrbitPoint(pVec(0, 0, 0), .02, 1.5);
    P.Explosion(pVec(0, 0, 0), i += (1.0f / float(numSteps)), 2, 3, 0.1);
    P.Move(true, false);

    P.Sink(false, PDSphere(pVec(0, 0, 0), 30));

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

void ParticleEffects::BounceToy(bool FirstTime, bool Immediate)
{
    if(FirstTime) {
        EffectName = "BounceToy";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    // Friction: 0 means don't slow its tangential velocity. Bigger than 0 means do.
    // Cutoff: If less than cutoff, don't apply friction.
    // Resilience: Scale normal velocity by this. Bigger is bouncier.
    const float Fric = 0.5f, Res = 0.50f, Cutoff = 0.07f;

    P.Color(PDLine(pVec(1,1,0), pVec(0,1,0)));
    P.Velocity(PDDisc(pVec(0,0,0), pVec(0,1,0.1), 0.01f));
    P.Source(10, PDLine(pVec(-5,0,10), pVec(5,0,10)));

    P.Gravity(GravityVec);

    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-4,-2,6), pVec(4,0,1), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(4,-2,8), pVec(4,0,-3), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-1,-2,6), pVec(2,0,-2), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(1,-2,2), pVec(4,0,2), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-6,-2,6), pVec(3,0,-5), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(6,-2,2), pVec(5,0,3), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(4,-2,-1), pVec(5,0,1.5), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-3,-2,-1), pVec(5,0,-1), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-8,-2,-4.1), pVec(14,0,2), pVec(0,4,0)));
    P.Bounce(Fric, Res, Cutoff, PDRectangle(pVec(-10,-2,5), pVec(4,0,5), pVec(0,4,0)));

    P.Jet(PDBox(pVec(-10,-2,-6), pVec(-7,0,-1)), PDPoint(pVec(0.0,0,.15)));
    P.TargetColor(pVec(0,0,1), 1, 0.004);
    P.Move(true, false);

    P.Sink(false, PDPlane(pVec(0,0,-7), pVec(0,0,1)));

    if(FirstTime)
        P.EndActionList();
}

// Whatever junk I want to throw in here
void ParticleEffects::Experimental(bool FirstTime, bool Immediate)
{
#if 0
    if(FirstTime) {
        EffectName = "BounceBox";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    // Friction: 0 means don't slow its tangential velocity. Bigger than 0 means do.
    // Cutoff: If less than cutoff, don't apply friction.
    // Resilience: Scale normal velocity by this. Bigger is bouncier.
    const float Fric = 1.0f, Res = 0.95f, Cutoff = 0.0f;

    // Don't apply friction if tangential velocity < cutoff
    // float tanscale = (vt.length2() <= cutoffSqr) ? 1.0f : oneMinusFriction;
    // m.vel = vt * tanscale + vn * resilience;

    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(4,0,0), pVec(1,0,0)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(-4,0,0), pVec(1,0,0)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(0,1,0), pVec(0,1,0)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(0,-4,0), pVec(0,1,0)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(0,0,4), pVec(0,0,1)));
    P.Bounce(Fric, Res, Cutoff, PDPlane(pVec(0,0,0), pVec(0,0,1)));

    P.Move(true, false);

    if(FirstTime)
        P.EndActionList();
#endif
#if 1
    //Vortex
    P.Size(1.0);
    P.Velocity(PDPoint(pVec(0,0,0)));
    P.Color(PDLine(pVec(.0,.8,.8), pVec(1, 1, 1)));
    P.StartingAge(0);

    if(FirstTime) {
        EffectName = "Tornado";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    P.Source(10, PDLine(pVec(-10,0,15), pVec(10,0,15)));

    P.Damping(pVec(.95));
    P.Gravity(GravityVec);
    P.Vortex(pVec(0,0,-2), pVec(0,0,11), 1.8, 7, 0.05, -GravityVec.length(), 0.1);
    P.Move(true, false);

    P.KillOld(1000);
    P.Sink(false, PDPlane(pVec(0,0,-2), pVec(0,0,1)));

    if(FirstTime)
        P.EndActionList();
#endif
}

// Fireflies bobbing around
void ParticleEffects::Fireflies(bool FirstTime, bool Immediate)
{
    P.Size(1.0);
    P.Velocity(PDPoint(pVec(0,0,0)));
    P.Color(PDLine(pVec(.1, .5, 0), pVec(.9, .9, .1)));
    P.StartingAge(0);

    if(FirstTime) {
        EffectName = "Fireflies";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    P.Source(1, PDBlob(pVec(0, 0, 2), 2));

    P.RandomAccel(PDSphere(pVec(0, 0, 0.00001), 0.002));
    P.Move(true, false);

    P.KillOld(600);

    if(FirstTime)
        P.EndActionList();
}

// Rocket-style fireworks
void ParticleEffects::Fireworks(bool FirstTime, bool Immediate)
{
    if(FirstTime) {
        EffectName = "Fireworks";
        ChangesEachFrame = true;
    }

    const int MAX_ROCKETS = 50;
    static int RocketSystem = -1;

    if(RocketSystem < 0)
        RocketSystem = P.GenParticleGroups(1, MAX_ROCKETS);

    // Move the rockets.
    P.CurrentGroup(RocketSystem);
    P.Velocity(PDCylinder(pVec(0,0,0.3), pVec(0,0,0.5), 0.11, 0.07));
    P.Color(PDBox(pVec(0,0.5,0), pVec(1,1,1)));
    P.StartingAge(0);

    P.Source(0.05, PDDisc(pVec(0,0,0), pVec(0,0,1), 6));
    P.Sink(false, PDPlane(pVec(0,0,-1), pVec(0,0,1)));
    P.Gravity(GravityVec);
    P.Move(true, false);

    // Read back the position of the rockets.
    pVec rocketp[MAX_ROCKETS], rocketc[MAX_ROCKETS], rocketv[MAX_ROCKETS];
    int rcount = (int)P.GetParticles(0, MAX_ROCKETS, (float *)rocketp, (float *)rocketc, (float *)rocketv);

    P.CurrentGroup(particle_handle);

    if(!Immediate)
        P.NewActionList(action_handle);

    P.Size(1.0);
    P.StartingAge(0, 6);

    for(int i=0; i<rcount; i++)
    {
        pVec rv(rocketv[i]);
        rv.normalize();
        rv *= -0.026;
        //cerr << i << " " <<rocketp[i][0]<<" "<<rocketp[i][1]<<" "<<rocketp[i][2]<<"\n";
        //cerr << "c " <<rocketc[i][0]<<" "<<rocketc[i][1]<<" "<<rocketc[i][2]<<"\n";

        P.Color(PDLine(rocketc[i], pVec(1,.5,.5)));
        P.Velocity(PDBlob(rv, 0.006));
        P.Source(70, PDPoint(rocketp[i]));
    }

    P.Gravity(GravityVec);
    //P.Damping(pVec(0.999));
    P.TargetColor(pVec(0,0,0), 0, 0.02);
    P.Move(true, false);
    P.KillOld(90);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

// It's like a flame thrower spinning around
void ParticleEffects::FlameThrower(bool FirstTime, bool Immediate)
{
    static double dirAng = 0;

    if(FirstTime) {
        EffectName = "Flame Thrower";
        ChangesEachFrame = true;
        double Ang = pRandf() * 2.0 * M_PI;
    }

    dirAng += 0.02;

    if(!Immediate)
        P.NewActionList(action_handle);

    P.Color(PDLine(pVec(0.8,0,0), pVec(1,1,0.3)));
    P.Velocity(PDBlob(pVec(sin(dirAng)*.8,cos(dirAng)*.8,0), 0.03));
    P.StartingAge(0);
    P.Size(1);

    int LifeTime = 100;
    P.KillOld(LifeTime);
    P.Source(maxParticles / LifeTime, PDDisc(pVec(0, 0, 2), pVec(0, 0, 1), 0.5));
    P.Gravity(pVec(0, 0, .01));
    P.Damping(pVec(0.9, 0.97, 0.9));

    float BOX = .017;
    P.RandomAccel(PDBox(pVec(-BOX, -BOX, -BOX), pVec(BOX, BOX, BOX)));

    P.Move(true, false);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

// A fountain spraying up in the middle of the screen
void ParticleEffects::Fountain(bool FirstTime, bool Immediate)
{
    P.ResetSourceState();
    P.Velocity(PDCylinder(pVec(0.0, -0.01, 0.35), pVec(0.0, -0.01, 0.37), 0.021, 0.019));
    P.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));

    if(FirstTime) {
        EffectName = "Fountain";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    P.Source(maxParticles * 0.01f, PDLine(pVec(0.0, 0.0, 0.0), pVec(0.0, 0.0, 0.405)));

    P.Gravity(GravityVec);
    P.Bounce(-0.05, 0.35, 0, PDDisc(pVec(0, 0, 0), pVec(0, 0, 1), 5));
    P.Move(true, false);

    P.Sink(false, PDPlane(pVec(0,0,-3), pVec(0,0,1)));
    P.SinkVelocity(true, PDSphere(pVec(0, 0, 0), 0.01));

    if(FirstTime)
        P.EndActionList();
}

// A bunch of particles in the shape of a photo
void ParticleEffects::PhotoShape(bool FirstTime, bool Immediate)
{
    if(Img == NULL) MakeFakeImage();

    if(FirstTime) {
        EffectName = "PhotoShape";
        ChangesEachFrame = false;

        P.Velocity(PDBlob(pVec(0, 0, 0), 0.001));
        P.StartingAge(0);
        P.KillOld(0);
        P.KillOld(1, true);
        int d = (int)sqrtf(maxParticles);

        float sx = Img->w() / float(d);
        float sy = Img->h() / float(d);
        float fy = 0.0f;
        for(int y=0; y<d; y++, fy+=sy) {
            float fx = 0.0f;
            for(int x=0; x<d; x++, fx+=sx) {
                f3Pixel p;
                sample2(p, *Img, fx, fy);
                P.Color(p.r(), p.g(), p.b());
                pVec v(fx, 0, fy);
                v /= float(Img->w());

                P.Vertex(v*6.0f - 3.0f);
            }
        }

        P.NewActionList(action_handle);
        P.EndActionList();
    }
}

// A bunch of particles in a grid shape
void ParticleEffects::GridShape(bool FirstTime, bool Immediate)
{
    if(FirstTime) {
        EffectName = "GridShape";
        ChangesEachFrame = false;

        P.Velocity(PDBlob(pVec(0, 0, 0), 0.001));
        P.KillOld(-100000);
        int dim = int(powf(float(maxParticles), 0.33333333f));
#define XX 8
#define YY 12
#define ZZ 9

        float dx = 2*XX / float(dim);
        float dy = 2*YY / float(dim);
        float dz = 2*ZZ / float(dim);

        float z = -ZZ;
        for(int k=0; k<dim; k++, z += dz) {
            float y = -YY;
            for(int l=0; l<dim; l++, y += dy) {
                float x = -XX;
                for(int j=0; j<dim; j++, x += dx) {
                    // Make the particles.
                    P.Color(j / float(dim), k / float(dim), l / float(dim));
                    P.StartingAge(0);
                    P.Vertex(pVec(x, y, z));
                }
            }
        }

        P.NewActionList(action_handle);
        P.EndActionList();
    }
}

// It's like a fan cruising around under a floor, blowing up on some ping pong balls.
// Like you see in real life.
void ParticleEffects::JetSpray(bool FirstTime, bool Immediate)
{
    P.Velocity(PDBlob(pVec(0, 0, 0), 0.01));
    P.Size(1.5);

    static pVec jet, dj;

    if(FirstTime) {
        EffectName = "JetSpray";
        ChangesEachFrame = true;

        jet = pVec(0);
        dj = pRandVec() * 0.5f;
        dj.z() = 0.0f;
    }

    if(!Immediate)
        P.NewActionList(action_handle);

    P.Color(PDSphere(pVec(.8, .4, .1), .1));
    P.Source(1, PDRectangle(pVec(-1, -1, 0.1), pVec(2, 0, 0), pVec(0, 2, 0)));

    P.Color(PDSphere(pVec(.5, .4, .1), .1));
    P.Source(300, PDRectangle(pVec(-10, -10, 0.1), pVec(20, 0, 0), pVec(0, 20, 0)));

    P.Gravity(GravityVec);

    jet += dj;
    if(jet.x() > 10 || jet.x() < -10) {dj.x() *= -1.0f; dj.y() += pRandf() * 0.005;}
    if(jet.y() > 10 || jet.y() < -10) {dj.y() *= -1.0f; dj.x() += pRandf() * 0.005;}

    P.Jet(PDSphere(jet, 1.5), PDBlob(pVec(0,0,.05), 0.01));

    P.Bounce(0.1, 0.3, 0.1, PDRectangle(pVec(-10, -10, 0.0), pVec(20, 0, 0), pVec(0, 20, 0)));

    P.Sink(false, PDPlane(pVec(0,0,-20), pVec(0,0,1)));

    P.Move(true, false);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

// A sprayer with particles that orbit two points
void ParticleEffects::Orbit2(bool FirstTime, bool Immediate)
{
    if(FirstTime) {
        EffectName = "Orbit2";
        ChangesEachFrame = true;
    }

    if(!Immediate)
        P.NewActionList(action_handle);

    P.Velocity(PDBlob(pVec(0.02, -0.2, 0), 0.015));
    P.Size(1.0);

    static float jetx=-4, jety=0, jetz=-2.4;

    static float djx = pRandf() * 0.15;
    static float djy = pRandf() * 0.15;
    static float djz = pRandf() * 0.15;

    jetx += djx;
    jety += djy;
    jetz += djz;

    if(jetx > 10 || jetx < -10) djx = -djx;
    if(jety > 10 || jety < -10) djy = -djy;
    if(jetz > 10 || jetz < -10) djz = -djz;

    int LifeTime = 350;

    P.Color(PDSphere(pVec(0.4+fabs(jetx*0.1), 0.4+fabs(jety*0.1), 0.4+fabs(jetz*0.1)), 0.1));
    P.Source(maxParticles / LifeTime, PDPoint(pVec(jetx, jety, jetz)));

    P.OrbitPoint(pVec(2, 0, 3), 0.1, 1.5);
    P.OrbitPoint(pVec(-2, 0, -3), 0.1, 1.5);
    P.Damping(pVec(0.994));
    P.Move(true, false);

    P.KillOld(LifeTime);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

// It kinda looks like rain hitting a parking lot
void ParticleEffects::Rain(bool FirstTime, bool Immediate)
{
    P.Velocity(pVec(0));
    P.Color(PDSphere(pVec(0.4, 0.4, 0.9), .1));
    P.Size(1.5);
    P.StartingAge(0);

    if(FirstTime) {
        EffectName = "Rain";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    P.Source(100, PDRectangle(pVec(-11, -10, 12), pVec(20, 0, 0), pVec(0, 20, 0)));

    P.RandomAccel(PDBlob(pVec(0.002, 0, -0.01), 0.003));
    P.Bounce(0.3, 0.3, 0, PDPlane(pVec(0,0,0), pVec(0,0,1)));
    P.Move(true, false);

    P.KillOld(100);

    if(FirstTime)
        P.EndActionList();
}

// Restore particles to their PositionB and UpVecB.
void ParticleEffects::Restore(bool FirstTime, bool Immediate)
{
    static float i = 0;
    if(FirstTime) {
        EffectName = "Restore";
        ChangesEachFrame = true;
        i = 200;
    }

    if(!Immediate)
        P.NewActionList(action_handle);

    P.Restore(i -= (1.0f / float(numSteps)));
    P.Move(true, false);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

// A sheet of particles falling down, avoiding various-shaped obstacles
void ParticleEffects::Shower(bool FirstTime, bool Immediate, int SteerShape)
{
    static float jetx=0, jety=0, jetz=0;
    static float djx, djy;

    if(FirstTime) {
        EffectName = "Shower";
        ChangesEachFrame = true;

        jetx = 1;
        jety = 1;
        djx = pRandf() * 0.1;
        djy = pRandf() * 0.1;
    }

    jetx += djx;
    jety += djy;

    if(jetx > 1 || jetx < 0) {djx = -djx; djy += pRandf() * 0.0005;}
    if(jety > 2 || jety < -2) {djy = -djy; djx += pRandf() * 0.0005;}

    if(!Immediate)
        P.NewActionList(action_handle);

    P.Velocity(pVec(0));
    P.Size(1.5);
    P.StartingAge(0);
    P.Color(jetx, jety, 1);

    P.Source(120, PDLine(pVec(-5,jety,8), pVec(5,jety,8)));

    P.Gravity(GravityVec);

    if(SteerShape == 0) {
        P.Avoid(0.2, 1.0, 20, PDSphere(pVec(0,0,0), 1.1));
    } else if(SteerShape == 1) {
        P.Avoid(2, 1.0, 20, PDTriangle(pVec(0,-1,0), pVec(2,0,0), pVec(0,2,0)));
    } else if(SteerShape == 2) {
        P.Avoid(2, 1.0, 20, PDRectangle(pVec(0,-1,0), pVec(2,1,0), pVec(0,2,0)));
    } else if(SteerShape == 3) {
        P.Avoid(2, 1.0, 20, PDPlane(pVec(0,0,0), pVec(0,0,1)));
    }

    P.Move(true, false);

    P.KillOld(130);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }

#ifndef NO_OGL_OBSTACLES
    glColor3f(1,1,0);
    if(SteerShape == 0) {
        glutSolidSphere(1, 16, 8);
    } else if(SteerShape == 1) {
        glBegin(GL_TRIANGLES);
        glVertex3f(0,-1,0);
        glVertex3f(2,0,0);
        glVertex3f(0,2,0);
        glEnd();
    } else if(SteerShape == 2) {
        glBegin(GL_QUADS);
        glVertex3f(0,-1,0);
        glVertex3f(2,0,0);
        glVertex3f(2,2,0);
        glVertex3f(0,1,0);
        glEnd();
    } else if(SteerShape == 3) {
        glBegin(GL_QUADS);
        glVertex3f(-2,-2,0);
        glVertex3f(2,-2,0);
        glVertex3f(2,2,0);
        glVertex3f(-2,2,0);
        glEnd();
    }
#endif
}

// A bunch of particles in a line that are attracted to the guy ahead of them in line
void ParticleEffects::Snake(bool FirstTime, bool Immediate)
{
    if(FirstTime) {
        EffectName = "Snake";
        ChangesEachFrame = true;

        P.Velocity(pVec(0));
        P.Size(1.0);
        P.StartingAge(0);

        //P.KillOld(-300);
        P.Color(PDSphere(pVec(0.93, 0.93, 0), 0.05));
        for(float x=-10.0; x<2.0; x+=0.05)
            P.Vertex(pVec(x, 0, 0));
    }

    // Move a lead particle to give them all something to gravitate toward
    static float jetx=0, jety=0, jetz=0;

    static float djx = pRandf() * 0.05;
    static float djy = pRandf() * 0.05;
    static float djz = pRandf() * 0.05;

    jetx += djx;
    jety += djy;
    jetz += djz;

    if(jetx > 6 || jetx < -6) djx = -djx;
    if(jety > 6 || jety < -6) djy = -djy;
    if(jetz > 6 || jetz < -6) djz = -djz;

    if(!Immediate)
        P.NewActionList(action_handle);

    P.KillOld(-1.0, true); // Kill the lead particle
    P.StartingAge(-10.0); // Tag the new lead particle as having a negative age
    P.Color(1, 0, 0);
    P.Velocity(pVec(0.001, 0, 0)); // This makes it able to compute a binormal.
    P.Vertex(pVec(jetx, jety, jetz));
    P.StartingAge(0);

    // Either of these gives an interesting effect.
    P.Follow(0.01, 1.0);
    // P.Gravitate(0.01, 1.0);
    P.Damping(pVec(0.9));
    P.Move(true, false);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

// A bunch of particles inside a sphere
void ParticleEffects::Sphere(bool FirstTime, bool Immediate)
{
    static double dirAng = 0;

    if(FirstTime) {
        EffectName = "Sphere";
        ChangesEachFrame = true;
    }

    dirAng += 0.02;

    if(!Immediate)
        P.NewActionList(action_handle);

    P.Color(PDLine(pVec(0,1,0), pVec(0,0,1)));
    P.Velocity(PDBlob(pVec(sin(dirAng)*.1,cos(dirAng)*.1, 0.1), 0.01));
    P.StartingAge(0);
    P.Size(1);

    P.Source(0.8, PDPoint(pVec(1, 1, 6)));

    P.Gravity(GravityVec);
    P.Bounce(0, 0.55, 0, PDSphere(pVec(0, 0, 4), 6));
    P.Move(true, false);

    P.KillOld(600);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

// A sprayer with particles orbiting a line
void ParticleEffects::Swirl(bool FirstTime, bool Immediate)
{
    if(FirstTime) {
        EffectName = "Swirl";
        ChangesEachFrame = true;
    }

    static pVec jet(-4, 0, -2.4);
    static pVec djet = pRandVec() * 0.05;

    jet += djet;

    if(jet.x() > 10 || jet.x() < -10) djet.x() = -djet.x();
    if(jet.y() > 10 || jet.y() < -10) djet.y() = -djet.y();
    if(jet.z() > 10 || jet.z() < -10) djet.z() = -djet.z();

    const int LifeTime = 300;

    if(!Immediate)
        P.NewActionList(action_handle);

    pVec tjet = Abs(jet) * 0.1 + pVec(0.4, 0.4, 0.4);
    P.Color(PDSphere(tjet, 0.1));
    P.Velocity(PDBlob(pVec(0.02, -0.2, 0), 0.015));
    P.Size(1.0);
    P.StartingAge(0);

    P.Source(maxParticles / LifeTime, PDPoint(jet));

    P.OrbitLine(pVec(0, 0, 1), pVec(1, 0.1, 0), 0.1, 1.5);
    P.Damping(pVec(1, 0.994, 0.994));
    P.Move(true, false);

    P.Sink(false, PDSphere(pVec(0, 0, 0), 15));
    P.KillOld(LifeTime);

    if(!Immediate) {
        P.EndActionList();
        P.CallActionList(action_handle);
    }
}

// A waterfall bouncing off invisible rocks
void ParticleEffects::Waterfall1(bool FirstTime, bool Immediate)
{
    P.Velocity(PDBlob(pVec(0.03, -0.001, 0.01), 0.002));
    P.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    P.Size(1.5);

    if(FirstTime) {
        EffectName = "Waterfall1";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    P.Source(100, PDLine(pVec(-5, -1, 8), pVec(-5, -3, 8)));

    P.Gravity(GravityVec);
    P.Bounce(0, 0.35, 0, PDRectangle(pVec(-7, -4, 7), pVec(3, 0, 0), pVec(0, 3, 0)));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(-4, -2, 4), 0.2));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(-3.5, 0, 2), 2));
    P.Bounce(0, 0.5, 0, PDSphere(pVec(3.8, 0, 0), 2));
    P.Bounce(-0.01, 0.35, 0, PDPlane(pVec(0,0,0), pVec(0,0,1)));
    P.Move(true, false);

    P.KillOld(300);
    P.Sink(false, PDSphere(pVec(0,0,0), 20));

    if(FirstTime)
        P.EndActionList();
}

// A waterfall bouncing off invisible rocks
void ParticleEffects::Waterfall2(bool FirstTime, bool Immediate)
{
    P.Velocity(PDBlob(pVec(0.1, 0, 0.1), 0.004));
    P.Color(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
    P.Size(1.5);
    P.StartingAge(0);

    if(FirstTime) {
        EffectName = "Waterfall2";
        ChangesEachFrame = false;
        P.NewActionList(action_handle);
    }

    P.Source(100, PDPoint(pVec(-4, 0, 6)));

    P.Gravity(GravityVec);
    P.Bounce(0, 0.01, 0, PDSphere(pVec(-1, 0, 4), 1));
    P.Bounce(0, 0.01, 0, PDSphere(pVec(-2.5, 0, 2), 1));
    P.Bounce(0, 0.01, 0, PDSphere(pVec(0.7, -0.5, 2), 1));
    P.Bounce(-0.01, 0.35, 0, PDPlane(pVec(0,0,0), pVec(0,0,1)));
    P.Move(true, false);

    P.KillOld(250);

    if(FirstTime)
        P.EndActionList();
}
