
#include "Effects.h"

#include <particle/papi.h>

#include <Image/ImageAlgorithms.h>

// This is for drawing the obstacles for Shower().
#include <GL/glut.h>

#include <iostream>

#ifdef WIN32
#pragma warning (disable:4305) /* disable bogus conversion warnings */
#pragma warning (disable:4244) /* disable bogus conversion warnings */
#define lrand48() ((rand() << 16) ^ rand())
#endif

ParticleEffects::ParticleEffects(int mp)
{
	maxParticles = mp;
	numSteps = 1;
	particle_handle = -1;
	action_handle = pGenActionLists(1);
	EffectName = "NoEffect";
    Img = NULL;
    GravityVec = pVec(0.0f, 0.0f, -0.01f);
}

int ParticleEffects::CallDemo(int DemoNum, bool FirstTime, bool Immediate)
{
	if(!Immediate && !FirstTime) {
		pCallActionList(action_handle);
		return DemoNum;
	}

	switch(DemoNum) {
	case  0: Atom(FirstTime); break;
	case  1: Balloons(FirstTime); break;
	case  2: Fireflies(FirstTime); break;
	case  3: Fireworks(FirstTime); break;
	case  4: Waterfall2(FirstTime); break;
	case  5: JetSpray(FirstTime); break;
	case  6: Fountain(FirstTime); break;
	case  7: FlameThrower(FirstTime); break;
	case  8: Rain(FirstTime); break;
	case  9: GridShape(FirstTime); break;
	case 10: Explosion(FirstTime); break;
	case 11: PhotoShape(FirstTime); break;
	case 12: Orbit2(FirstTime); break;
	case 13: Restore(FirstTime); break;
	case 14: Shower(FirstTime, 0); break;
	case 15: Shower(FirstTime, 1); break;
	case 16: Shower(FirstTime, 2); break;
	case 17: Swirl(FirstTime); break;
	case 18: Snake(FirstTime); break;
	case 19: Waterfall1(FirstTime); break;
	case 20: Sphere(FirstTime); break;
	case 21: Experimental(FirstTime); break;
	default:
		while(DemoNum < 0) DemoNum+= NumEffects;
		return CallDemo(DemoNum % NumEffects, FirstTime, Immediate);
		break;
	}

	return DemoNum;
}

// Particles orbiting a center
void ParticleEffects::Atom(bool FirstTime)
{
	pVelocityD(PDSphere(pVec(0, 0, 0), 0.2));
	pColorD(PDBox(pVec(0, 0, 0), pVec(1.0, 0.5, 0.5)));
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Atom";
		pNewActionList(action_handle);
	}

	pSource(maxParticles / 100, PDSphere(pVec(0, 0, 0), 6));

	// Orbit about the origin.
	pOrbitPoint(pVec(0, 0, 0), 0.05, 0.5);

	// Keep orbits from being too eccentric.
	pSink(true, PDSphere(pVec(0.0, 0.0, 0.0), 1.0));
	pSink(false, PDSphere(pVec(0.0, 0.0, 0.0), 8.0));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A bunch of balloons
void ParticleEffects::Balloons(bool FirstTime)
{
	if(FirstTime) {
		EffectName = "Balloons";
		pNewActionList(action_handle);
	}

	float x=0, y=0, z=-1;

	int nPar = (int)pGetGroupCount();
	float qty = (maxParticles - nPar) / 6.0f;    // Because of 6 colors
	qty /= 100.0f;
	float BBOX = 1.7;

	pStartingAge(0, 5);
	pVelocity(pVec(0));
	pColor(1,0,0); // These attributes don't get stored in the particle list.
	pSource(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

	pColor(1,1,0);
	pSource(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

	pColor(0,1,0);
	pSource(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

	pColor(0,1,1);
	pSource(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

	pColor(0,0,1);
	pSource(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

	pColor(1,0,1);
	pSource(qty, PDBox(pVec(x-BBOX, y-BBOX, z-BBOX), pVec(x+BBOX, y+BBOX, z+BBOX)));

	pGravity(pVec(.0005, .005, .0005));

	pDamping(0.9, 0.67, 0.9);

	float BOX = .005;

	pRandomAccel(PDBox(pVec(-BOX, -BOX, -BOX), pVec(BOX, BOX, BOX)));
	pKillOld(500);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// An explosion from the center of the universe, followed by gravity toward a point
void ParticleEffects::Explosion(bool FirstTime)
{
	pVelocityD(PDSphere(pVec(0,0,0), 0.01,0.01));
	pColorD(PDSphere(pVec(0.5, 0.7, 0.5), .3));
	pSize(1.0);
	pStartingAge(0);

	static float i=0;
	if(FirstTime) {
		EffectName = "Explosion";
		pNewActionList(action_handle);
		i = 0;
	}

	pDamping(0.999, 0.999, 0.999);

	pOrbitPoint(pVec(0, 0, 0), .02, 0.5);

	pExplosion(pVec(0, 0, 0), 1, 2, 3, 0.1, i+= (1.0f / float(numSteps)));

	pSink(false, PDSphere(pVec(0, 0, 0), 30));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// Whatever junk I want to throw in here
void ParticleEffects::Experimental(bool FirstTime)
{
#if 1
    if(FirstTime) {
		EffectName = "BounceToy";
		pNewActionList(action_handle);
	}

    // Friction: 0 means don't slow its tangential velocity. Bigger than 0 means do.
    // Cutoff: If less than cutoff, don't apply friction.
    // Resilience: Scale normal velocity by this. Bigger is bouncier.
    const float Fric = 0.5f, Res = 0.50f, Cutoff = 0.07f;

    pColorD(PDLine(pVec(1,1,0), pVec(0,1,0)));
    pVelocityD(PDDisc(pVec(0,0,0), pVec(0,1,0.1), 0.01f));
    pSource(10, PDLine(pVec(-5,0,10), pVec(5,0,10)));

    pGravity(GravityVec);

    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(-4,-2,6), pVec(4,0,1), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(4,-2,8), pVec(4,0,-3), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(-1,-2,6), pVec(2,0,-2), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(1,-2,2), pVec(4,0,2), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(-6,-2,6), pVec(3,0,-5), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(6,-2,2), pVec(5,0,3), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(4,-2,-1), pVec(5,0,1.5), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(-3,-2,-1), pVec(5,0,-1), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(-8,-2,-4.1), pVec(14,0,2), pVec(0,4,0)));
    pBounce(Fric, Res, Cutoff, PDRectangle(pVec(-10,-2,5), pVec(4,0,5), pVec(0,4,0)));

	pJet(PDBox(pVec(-10,-2,-6), pVec(-7,2,-1)), PDPoint(pVec(0.0,0,.15)));

	pMove();

    pSink(false, PDPlane(pVec(0,0,-7), pVec(0,0,1)));
    pTargetColor(pVec(0,0,1), 1, 0.004);
	
	if(FirstTime)
		pEndActionList();
#endif
#if 0
    if(FirstTime) {
		EffectName = "BounceBox";
		pNewActionList(action_handle);
	}

    // Friction: 0 means don't slow its tangential velocity. Bigger than 0 means do.
    // Cutoff: If less than cutoff, don't apply friction.
    // Resilience: Scale normal velocity by this. Bigger is bouncier.
    const float Fric = 1.0f, Res = 0.95f, Cutoff = 0.0f;

    // Don't apply friction if tangential velocity < cutoff
    // float tanscale = (vt.length2() <= cutoffSqr) ? 1.0f : oneMinusFriction;
    // m.vel = vt * tanscale + vn * resilience;

    pBounce(Fric, Res, Cutoff, PDPlane(pVec(4,0,0), pVec(1,0,0)));
	pBounce(Fric, Res, Cutoff, PDPlane(pVec(-4,0,0), pVec(1,0,0)));
	pBounce(Fric, Res, Cutoff, PDPlane(pVec(0,1,0), pVec(0,1,0)));
	pBounce(Fric, Res, Cutoff, PDPlane(pVec(0,-4,0), pVec(0,1,0)));
	pBounce(Fric, Res, Cutoff, PDPlane(pVec(0,0,4), pVec(0,0,1)));
	pBounce(Fric, Res, Cutoff, PDPlane(pVec(0,0,0), pVec(0,0,1)));

	pMove();

	if(FirstTime)
		pEndActionList();
#endif
#if 0
	pVelocityD(PDCylinder(pVec(0.0, -0.01, 0.35), pVec(0.0, -0.01, 0.37), 0.021, 0.019));
	pColorD(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "TestFountain";
		pNewActionList(action_handle);
	}

	if(FirstTime) {
		std::cerr << "Doing compiled fountain.\n";
		pSource(maxParticles * 0.01f, PDLine(pVec(0.0, 0.0, 0.401), pVec(0.0, 0.0, 0.405)));

		pFountain();

		pGravity(GravityVec);

		pSinkVelocity(true, PDSphere(pVec(0, 0, 0), 0.01));

		pBounce(-0.05, 0.35, 0, PDDisc(pVec(0, 0, 0), pVec(0, 0, 1), 5));

		pSink(false, PDPlane(pVec(0,0,-3), pVec(0,0,1)));

		pMove();
	} else {
		std::cerr<<"Only call with action list!\n";
	}

	if(FirstTime)
		pEndActionList();
#endif
#if 0
	//Vortex
	pSize(1.0);
	pVelocityD(PDPoint(pVec(0,0,0));
	pColorD(1.0, PDLine(pVec(.8,.8,.8), pVec(1, 1, 1));
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Tornado";
		pNewActionList(action_handle);
	}

	pSource(10, PDCylinder(pVec(0,0,0), pVec(0,0,9), 6));

	pDamping(.95,.95,.95);
	pGravity(GravityVec);
	pVortex(0,0,-5, 0,0,13,    0.18, 1.5, 0.001, 0.5, 6);

	pKillOld(1000);
	pSink(false, PDPlane(pVec(0,0,-1), pVec(0,0,1)));

	pMove();

	if(FirstTime)
		pEndActionList();
#endif
}

// Fireflies bobbing around
void ParticleEffects::Fireflies(bool FirstTime)
{
	pSize(1.0);
	pVelocityD(PDPoint(pVec(0,0,0)));
	pColorD(PDLine(pVec(.1, .5, 0), pVec(.9, .9, .1)));
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Fireflies";
		pNewActionList(action_handle);
	}

	pSource(1, PDBlob(pVec(0, 0, 2), 2));

	pRandomAccel(PDSphere(pVec(0, 0, 0.00001), 0.002));

	pKillOld(600);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// Rocket-style fireworks
void ParticleEffects::Fireworks(bool FirstTime)
{
	if(FirstTime) {
		EffectName = "Fireworks";
	}

#define MAX_ROCKETS 25
	static int RocketSystem = -1;

	if(RocketSystem < 0)
		RocketSystem = pGenParticleGroups(1, MAX_ROCKETS);

	// Move the rockets.
	pCurrentGroup(RocketSystem);
	pVelocityD(PDCylinder(pVec(0,0,0.25), pVec(0,0,0.35), 0.11, 0.07));
	pColorD(PDBox(pVec(0,0.5,0), pVec(1,1,1)));
	pStartingAge(0);

	pSource(0.02, PDDisc(pVec(0,0,0), pVec(0,0,1), 6));
	pSink(false, PDPlane(pVec(0,0,-1), pVec(0,0,1)));
	pGravity(GravityVec);
	pMove();

	// Read back the position of the rockets.
	float rocketp[MAX_ROCKETS][3], rocketc[MAX_ROCKETS][3], rocketv[MAX_ROCKETS][3];
	int rcount = (int)pGetParticles(0, MAX_ROCKETS, (float *)rocketp, (float *)rocketc, (float *)rocketv);

	pCurrentGroup(particle_handle);

	pSize(1.0);
	pStartingAge(0, 6);

	for(int i=0; i<rcount; i++)
	{
		pVec rv(rocketv[i][0], rocketv[i][1], rocketv[i][2]);
		rv.normalize();
		rv *= -0.026;
		//cerr << i << " " <<rocketp[i][0]<<" "<<rocketp[i][1]<<" "<<rocketp[i][2]<<"\n";
		//cerr << "c " <<rocketc[i][0]<<" "<<rocketc[i][1]<<" "<<rocketc[i][2]<<"\n";

		pColorD(PDLine(pVec(rocketc[i][0], rocketc[i][1], rocketc[i][2]), pVec(1,.5,.5)));
		pVelocityD(PDBlob(rv, 0.006));
		pSource(40, PDPoint(pVec(rocketp[i][0], rocketp[i][1], rocketp[i][2])));
	}

	pGravity(GravityVec);
	//pDamping(pVec(0.999, 0.999, 0.999));
	pTargetColor(pVec(0,0,0), 0, 0.02);
	pKillOld(90);

	pMove();
}

// It's like a flame thrower spinning around
void ParticleEffects::FlameThrower(bool FirstTime)
{
	static double dirAng = 0;

	if(FirstTime) {
		EffectName = "Flame Thrower";
		double Ang = pRandf() * 2.0 * M_PI;
		pNewActionList(action_handle);
	}

	dirAng += 0.02;

	pColorD(PDLine(pVec(0.8,0,0), pVec(1,1,0.3)));
	pVelocityD(PDBlob(pVec(sin(dirAng)*.8,cos(dirAng)*.8,0), 0.03));
	pStartingAge(0);
	pSize(1);

	int LifeTime = 100;
	pKillOld(LifeTime);
	pSource(maxParticles / LifeTime, PDDisc(pVec(0, 0, 2), pVec(0, 0, 1), 0.5));
	pGravity(pVec(0, 0, .01));
	pDamping(pVec(0.9, 0.97, 0.9));

	float BOX = .017;
	pRandomAccel(PDBox(pVec(-BOX, -BOX, -BOX), pVec(BOX, BOX, BOX)));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A fountain spraying up in the middle of the screen
void ParticleEffects::Fountain(bool FirstTime)
{
	pVelocityD(PDCylinder(pVec(0.0, -0.01, 0.35), pVec(0.0, -0.01, 0.37), 0.021, 0.019));
	pColorD(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Fountain";
		pNewActionList(action_handle);
	}

	pSource(maxParticles * 0.01f, PDLine(pVec(0.0, 0.0, 0.0), pVec(0.0, 0.0, 0.405)));

	pGravity(GravityVec);

	pBounce(-0.05, 0.35, 0, PDDisc(pVec(0, 0, 0), pVec(0, 0, 1), 5));

	pMove();

	pSink(false, PDPlane(pVec(0,0,-3), pVec(0,0,1)));

	pSinkVelocity(true, PDSphere(pVec(0, 0, 0), 0.01));

	if(FirstTime)
		pEndActionList();
}

// A bunch of particles in the shape of a photo
void ParticleEffects::PhotoShape(bool FirstTime)
{
    if(Img == NULL) return;

    if(FirstTime) {
        EffectName = "PhotoShape";

        pVelocityD(PDBlob(pVec(0, 0, 0), 0.001));
        pStartingAge(0);
        pKillOld(0);
        pKillOld(1, true);
        int d = sqrtf(maxParticles);

        float sx = Img->w() / float(d);
        float sy = Img->h() / float(d);
        float sc = 1.0f;
        float fy = 0.0f;
        for(int y=0; y<d; y++, fy+=sy) {
            float fx = 0.0f;
            for(int x=0; x<d; x++, fx+=sx) {
                f3Pixel p;
                sample2(p, *Img, fx, fy);
                pColor(p.r()*sc, p.g()*sc, p.b()*sc);
                pVec v(fx, 0, fy);
                v /= float(Img->w());

                pVertex(v*6.0f - 3.0f);
            }
        }

        pNewActionList(action_handle);
        pEndActionList();
    }
}

// A bunch of particles in a grid shape
void ParticleEffects::GridShape(bool FirstTime)
{
	if(FirstTime) {
		EffectName = "GridShape";

		pVelocityD(PDBlob(pVec(0, 0, 0), 0.001));
		pKillOld(-100000);
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
					pColor(j / float(dim), k / float(dim), l / float(dim));
					pStartingAge(0);
					pVertex(pVec(x, y, z));
				}
			}
		}

		pNewActionList(action_handle);
		pEndActionList();
	}
}

// It's like a fan cruising around under a floor, blowing up on some ping pong balls
void ParticleEffects::JetSpray(bool FirstTime)
{
	pVelocityD(PDBlob(pVec(0, 0, 0), 0.01));
	pSize(1.5);

	static pVec jet, dj;

	if(FirstTime) {
		EffectName = "JetSpray";
		pNewActionList(action_handle);
		jet = pVec(0);
        dj = pRandVec() * 0.5f;
        dj.z() = 0.0f;
	}

	pColorD(PDSphere(pVec(.8, .4, .1), .1));
	pSource(1, PDRectangle(pVec(-1, -1, 0.1), pVec(2, 0, 0), pVec(0, 2, 0)));

	pColorD(PDSphere(pVec(.5, .4, .1), .1));
	pSource(300, PDRectangle(pVec(-10, -10, 0.1), pVec(20, 0, 0), pVec(0, 20, 0)));

	pGravity(GravityVec);

	jet += dj;
	if(jet.x() > 10 || jet.x() < -10) {dj.x() *= -1.0f; dj.y() += pRandf() * 0.005;}
	if(jet.y() > 10 || jet.y() < -10) {dj.y() *= -1.0f; dj.x() += pRandf() * 0.005;}

	pJet(PDSphere(jet, 1.5), PDBlob(pVec(0,0,.05), 0.01));

	pBounce(0.1, 0.3, 0.1, PDRectangle(pVec(-10, -10, 0.0), pVec(20, 0, 0), pVec(0, 20, 0)));

	pSink(false, PDPlane(pVec(0,0,-20), pVec(0,0,1)));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A sprayer with particles that orbit two points
void ParticleEffects::Orbit2(bool FirstTime)
{
	if(FirstTime) {
		EffectName = "Orbit2";
		pNewActionList(action_handle);
	}

	pVelocityD(PDBlob(pVec(0.02, -0.2, 0), 0.015));
	pSize(1.0);

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

	pKillOld(LifeTime);

	pColorD(PDSphere(pVec(0.4+fabs(jetx*0.1), 0.4+fabs(jety*0.1), 0.4+fabs(jetz*0.1)), 0.1));
	pSource(maxParticles / LifeTime, PDPoint(pVec(jetx, jety, jetz)));

	pOrbitPoint(pVec(2, 0, 3), 0.1, 0.5);

	pOrbitPoint(pVec(-2, 0, -3), 0.1, 0.5);

	pDamping(0.994, 0.994, 0.994);

	//pSink(false, PDSphere, 0, 0, 0, 15);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// It kinda looks like rain hitting a parking lot
void ParticleEffects::Rain(bool FirstTime)
{
	pVelocity(pVec(0));
	pColorD(PDSphere(pVec(0.4, 0.4, 0.9), .1));
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Rain";
		pNewActionList(action_handle);
	}

	pSource(100, PDRectangle(pVec(-11, -10, 12), pVec(20, 0, 0), pVec(0, 20, 0)));

	pRandomAccel(PDBlob(pVec(0.002, 0, -0.01), 0.003));

	pBounce(0.3, 0.3, 0, PDPlane(pVec(0,0,0), pVec(0,0,1)));

	pKillOld(100);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// Restore particles to their positionB.
void ParticleEffects::Restore(bool FirstTime)
{
	static float i = 0;
	if(FirstTime) {
		EffectName = "Restore";
		pNewActionList(action_handle);
		i = 200;
	}

	pGravity(GravityVec);

	pRestore(i -= (1.0f / float(numSteps)));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A sheet of particles falling down, avoiding various-shaped obstacles
void ParticleEffects::Shower(bool FirstTime, int SteerShape)
{
	static float jetx=0, jety=0, jetz=0;
	static float djx, djy;

	if(FirstTime) {
		EffectName = "Shower";
		jetx = 1;
		jety = 1;
		djx = pRandf() * 0.1;
		djy = pRandf() * 0.1;
		pNewActionList(action_handle);
	}

	jetx += djx;
	jety += djy;

	if(jetx > 1 || jetx < 0) {djx = -djx; djy += pRandf() * 0.0005;}
	if(jety > 2 || jety < -2) {djy = -djy; djx += pRandf() * 0.0005;}

	pVelocity(pVec(0));
	pSize(1.5);
	pStartingAge(0);
	pColor(jetx, jety, 1);

	pSource(120, PDLine(pVec(-5,jety,8), pVec(5,jety,8)));

	pGravity(GravityVec);

	glColor3f(1,1,0);
	if(SteerShape == 0) {
		pAvoid(0.2, 1.0, 20, PDSphere(pVec(0,0,0), 1.1));

		glutSolidSphere(1, 16, 8);
	} else if(SteerShape == 1) {
		pAvoid(2, 1.0, 20, PDTriangle(pVec(0,-1,0), pVec(2,0,0), pVec(0,2,0)));

		glBegin(GL_TRIANGLES);
		glVertex3f(0,-1,0);
		glVertex3f(2,0,0);
		glVertex3f(0,2,0);
		glEnd();
	} else if(SteerShape == 2) {
		pAvoid(2, 1.0, 20, PDRectangle(pVec(0,-1,0), pVec(2,1,0), pVec(0,2,0)));

		glBegin(GL_QUADS);
		glVertex3f(0,-1,0);
		glVertex3f(2,0,0);
		glVertex3f(2,2,0);
		glVertex3f(0,1,0);
		glEnd();
	} else if(SteerShape == 3) {
		pAvoid(2, 1.0, 20, PDPlane(pVec(0,0,0), pVec(0,0,1)));

		glBegin(GL_QUADS);
		glVertex3f(-2,-2,0);
		glVertex3f(2,-2,0);
		glVertex3f(2,2,0);
		glVertex3f(-2,2,0);
		glEnd();
	}

	pKillOld(130);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A bunch of particles in a line that are attracted to the guy ahead of them in line
void ParticleEffects::Snake(bool FirstTime)
{
	if(FirstTime) {
		EffectName = "Snake";

		pVelocity(pVec(0));
		pSize(1.0);
		pStartingAge(0);

		pKillOld(-300);
		pColorD(PDSphere(pVec(0.93, 0.93, 0), 0.05));
		for(float x=-10.0; x<2.0; x+=0.05)
			pVertex(pVec(x, 0, 0));
		pNewActionList(action_handle);
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

	pKillOld(-1.0, true); // Kill the lead particle
	pStartingAge(-10.0); // Tag the new lead particle as having a negative age
	pColor(1, 0, 0);
	pVelocity(pVec(0.001, 0, 0)); // This makes it able to compute a binormal.
	pVertex(pVec(jetx, jety, jetz));
	pStartingAge(0);

	// Either of these gives an interesting effect.
	pFollow(0.01, 1.0);
	// pGravitate(0.01, 1.0);

	pDamping(0.9, 0.9, 0.9);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A bunch of particles inside a sphere
void ParticleEffects::Sphere(bool FirstTime)
{
	static double dirAng = 0;

	if(FirstTime) {
		EffectName = "Sphere";
		pNewActionList(action_handle);
	}

	dirAng += 0.02;

	pColorD(PDLine(pVec(0,1,0), pVec(0,0,1)));
	pVelocityD(PDBlob(pVec(sin(dirAng)*.1,cos(dirAng)*.1, 0.1), 0.01));
	pStartingAge(0);
	pSize(1);

	pKillOld(600);
	pSource(0.8, PDPoint(pVec(1, 1, 6)));
	pGravity(GravityVec);
	pBounce(0, 0.55, 0, PDSphere(pVec(0, 0, 4), 6));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A sprayer with particles orbiting a line
void ParticleEffects::Swirl(bool FirstTime)
{
	pVelocityD(PDBlob(pVec(0.02, -0.2, 0), 0.015));
	pSize(1.0);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Swirl";
		pNewActionList(action_handle);
	}

	static pVec jet(-4, 0, -2.4);
	static pVec djet = pRandVec() * 0.05;

	jet += djet;

	if(jet.x() > 10 || jet.x() < -10) djet.x() = -djet.x();
	if(jet.y() > 10 || jet.y() < -10) djet.y() = -djet.y();
	if(jet.z() > 10 || jet.z() < -10) djet.z() = -djet.z();

	int LifeTime = 300;

	pKillOld(LifeTime);

	pVec tjet = Abs(jet) * 0.1 + pVec(0.4, 0.4, 0.4);
	pColorD(PDSphere(tjet, 0.1));
	pSource(maxParticles / LifeTime, PDPoint(jet));

	pOrbitLine(pVec(0, 0, 1), pVec(1, 0.1, 0), 0.5, 0.1);

	pDamping(pVec(1, 0.994, 0.994));

	pSink(false, PDSphere(pVec(0, 0, 0), 15));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A waterfall bouncing off invisible rocks
void ParticleEffects::Waterfall1(bool FirstTime)
{
	pVelocityD(PDBlob(pVec(0.03, -0.001, 0.01), 0.002));
	pColorD(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
	pSize(1.5);

	if(FirstTime) {
		EffectName = "Waterfall1";
		pNewActionList(action_handle);
	}

	pSource(100, PDLine(pVec(-5, -1, 8), pVec(-5, -3, 8)));
	pGravity(GravityVec);
	pKillOld(300);
	pBounce(0, 0.35, 0, PDRectangle(pVec(-7, -4, 7), pVec(3, 0, 0), pVec(0, 3, 0)));
	pBounce(0, 0.5, 0, PDSphere(pVec(-4, -2, 4), 0.2));
	pBounce(0, 0.5, 0, PDSphere(pVec(-3.5, 0, 2), 2));
	pBounce(0, 0.5, 0, PDSphere(pVec(3.8, 0, 0), 2));
	pBounce(-0.01, 0.35, 0, PDPlane(pVec(0,0,0), pVec(0,0,1)));
	pSink(false, PDSphere(pVec(0,0,0), 20));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A waterfall bouncing off invisible rocks
void ParticleEffects::Waterfall2(bool FirstTime)
{
	pVelocityD(PDBlob(pVec(0.1, 0, 0.1), 0.004));
	pColorD(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Waterfall2";
		pNewActionList(action_handle);
	}

	pSource(100, PDPoint(pVec(-4, 0, 6)));
	pGravity(GravityVec);
	pKillOld(250);
	pBounce(0, 0.01, 0, PDSphere(pVec(-1, 0, 4), 1));
	pBounce(0, 0.01, 0, PDSphere(pVec(-2.5, 0, 2), 1));
	pBounce(0, 0.01, 0, PDSphere(pVec(0.7, -0.5, 2), 1));
	pBounce(-0.01, 0.35, 0, PDPlane(pVec(0,0,0), pVec(0,0,1)));
	pMove();

	if(FirstTime)
		pEndActionList();
}
