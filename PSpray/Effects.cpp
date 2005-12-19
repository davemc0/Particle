#include "Effects.h"
#include <particle/papi.h>
#include <particle/pVector.h>

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
	case  4: JetSpray(FirstTime); break;
	case  5: Fountain(FirstTime); break;
	case  6: FlameThrower(FirstTime); break;
	case  7: Rain(FirstTime); break;
	case  8: ShaftShape(FirstTime); break;
	case  9: Explosion(FirstTime); break;
	case 10: GridShape(FirstTime); break;
	case 11: Orbit2(FirstTime); break;
	case 12: Restore(FirstTime); break;
	case 13: Shower(FirstTime, PDSphere); break;
	case 14: Shower(FirstTime, PDTriangle); break;
	case 15: Shower(FirstTime, PDRectangle); break;
	case 16: Swirl(FirstTime); break;
	case 17: Snake(FirstTime); break;
	case 18: Waterfall1(FirstTime); break;
	case 19: Waterfall2(FirstTime); break;
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
	pVelocityD(PDSphere, 0, 0, 0, 0.2);
	pColorD(1.0, PDBox, 0, 0, 0, 1.0, 0.5, 0.5);
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Atom";
		pNewActionList(action_handle);
	}

	pCopyVertexB(false, true);

	pSource(maxParticles / 100, PDSphere, 0, 0, 0, 6);

	// Orbit about the origin.
	pOrbitPoint(0, 0, 0, 0.05, 0.5);

	// Keep orbits from being too eccentric.
	pSink(true, PDSphere, 0.0, 0.0, 0.0, 1.0);
	pSink(false, PDSphere, 0.0, 0.0, 0.0, 8.0);

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

	pCopyVertexB(false, true);

	float x=0, y=0, z=-1;

	int nPar = (int)pGetGroupCount();
	float qty = (maxParticles - nPar) / 6.0f;    // Because of 6 colors
	qty /= 100.0f;
	float BBOX = 1.7;

	pStartingAge(0, 5);
	pVelocity(0,0,0);
	pColor(1,0,0); // These attributes don't get stored in the particle list.
	pSource(qty, PDBox, x-BBOX, y-BBOX, z-BBOX, x+BBOX, y+BBOX, z+BBOX);

	pColor(1,1,0);
	pSource(qty, PDBox, x-BBOX, y-BBOX, z-BBOX, x+BBOX, y+BBOX, z+BBOX);

	pColor(0,1,0);
	pSource(qty, PDBox, x-BBOX, y-BBOX, z-BBOX, x+BBOX, y+BBOX, z+BBOX);

	pColor(0,1,1);
	pSource(qty, PDBox, x-BBOX, y-BBOX, z-BBOX, x+BBOX, y+BBOX, z+BBOX);

	pColor(0,0,1);
	pSource(qty, PDBox, x-BBOX, y-BBOX, z-BBOX, x+BBOX, y+BBOX, z+BBOX);

	pColor(1,0,1);
	pSource(qty, PDBox, x-BBOX, y-BBOX, z-BBOX, x+BBOX, y+BBOX, z+BBOX);

	pGravity(.0005, .005, .0005);

	pDamping(0.9, 0.67, 0.9);

	float BOX = .005;

	pRandomAccel(PDBox, -BOX, -BOX, -BOX, BOX, BOX, BOX);
	pKillOld(500);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// An explosion from the center of the universe, followed by gravity
void ParticleEffects::Explosion(bool FirstTime)
{
	pVelocityD(PDSphere, 0,0,0,0.01,0.01);
	pColorD(1.0, PDSphere, 0.5, 0.7, 0.5, .3);
	pSize(1.0);
	pStartingAge(0);

	static float i=0;
	if(FirstTime) {
		EffectName = "Explosion";
		pNewActionList(action_handle);
		i = 0;
	}

	pCopyVertexB(false, true);

	pDamping(0.999, 0.999, 0.999);

	pOrbitPoint(0, 0, 0, .02, 0.5);

	pExplosion(0, 0, 0, 1, 2, 3, 0.1, i+= (1.0f / float(numSteps)));

	pSink(false, PDSphere, 0, 0, 0, 30);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// Whatever junk I want to throw in here
void ParticleEffects::Experimental(bool FirstTime)
{
#if 0
	pVelocityD(PDCylinder, 0.0, -0.01, 0.35, 0.0, -0.01, 0.37, 0.021, 0.019);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "TestFountain";
		pNewActionList(action_handle);
	}

	if(FirstTime) {
		std::cerr << "Doing compiled fountain.\n";
		pSource(maxParticles * 0.01f, PDLine, 0.0, 0.0, 0.401, 0.0, 0.0, 0.405);

		pFountain();

		pCopyVertexB(false, true);

		pGravity(0.0, 0.0, -0.01);

		pSinkVelocity(true, PDSphere, 0, 0, 0, 0.01);

		pBounce(-0.05, 0.35, 0, PDDisc, 0, 0, 0,  0, 0, 1,  5);

		pSink(false, PDPlane, 0,0,-3, 0,0,1);

		pMove();
	} else {
		std::cerr<<"Only call with action list!\n";
	}

	if(FirstTime)
		pEndActionList();
#else
	//Vortex
	pSize(1.0);
	pVelocityD(PDPoint, 0,0,0);
	pColorD(1.0, PDLine, .8,.8,.8, 1, 1, 1);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Tornado";
		pNewActionList(action_handle);
	}

	pCopyVertexB(false, true);

	pSource(10, PDCylinder, 0,0,0, 0,0,9, 6);

	pDamping(.95,.95,.95);
	pGravity(0, 0, -.01);
	pVortex(0,0,-5, 0,0,13,    0.18, 1.5, 0.001, 0.5, 6);

	pKillOld(1000);
	pSink(false, PDPlane, 0,0,-1, 0,0,1);

	pMove();

	if(FirstTime)
		pEndActionList();
#endif
}

// Fireflies bobbing around
void ParticleEffects::Fireflies(bool FirstTime)
{
	pSize(1.0);
	pVelocityD(PDPoint, 0,0,0);
	pColorD(1.0, PDLine, .1, .5, 0, .9, .9, .1);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Fireflies";
		pNewActionList(action_handle);
	}

	pCopyVertexB(false, true);

	pSource(1, PDBlob, 0, 0, 2, 2);

	pRandomAccel(PDSphere, 0, 0, 0.00001, 0.002);

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
	pVelocityD(PDCylinder, 0,0,0.15, 0,0,0.2, 0.11, 0.07);
	pColorD(1, PDBox, 0,0.5,0, 1,1,1);
	pStartingAge(0);

	pSource(0.02, PDDisc, 0,0,0, 0,0,1, 6);
	pSink(false, PDPlane, 0,0,-1, 0,0,1);
	pGravity(0,0,-0.003);
	pMove();

	// Read back the position of the rockets.
	float rocketp[MAX_ROCKETS][3], rocketc[MAX_ROCKETS][3], rocketv[MAX_ROCKETS][3];
	int rcount = (int)pGetParticles(0, MAX_ROCKETS, (float *)rocketp, (float *)rocketc, (float *)rocketv);

	pCurrentGroup(particle_handle);

	pSize(1.0);
	pStartingAge(0, 6);

	pCopyVertexB(false, true);

	for(int i=0; i<rcount; i++)
	{
		pVector rv(rocketv[i][0], rocketv[i][1], rocketv[i][2]);
		rv.normalize();
		rv *= -0.026;
		//cerr << i << " " <<rocketp[i][0]<<" "<<rocketp[i][1]<<" "<<rocketp[i][2]<<"\n";
		//cerr << "c " <<rocketc[i][0]<<" "<<rocketc[i][1]<<" "<<rocketc[i][2]<<"\n";

		pColorD(1.0, PDLine, rocketc[i][0], rocketc[i][1], rocketc[i][2], 1,.5,.5);
		pVelocityD(PDBlob, rv.x, rv.y, rv.z, 0.006);
		pSource(40, PDPoint, rocketp[i][0], rocketp[i][1], rocketp[i][2]);
	}

	pGravity(0,0,-0.001);
	//pDamping(0.999, 0.999, 0.999);
	pTargetColor(0,0,0,0, 0.02);
	pKillOld(90);

	pMove();
}

// It's like a flame thrower spinning around
void ParticleEffects::FlameThrower(bool FirstTime)
{
	static pVector Jet;
	static pVector dJet;
	static double dirAng = 0;

	if(FirstTime) {
		EffectName = "Flame Thrower";
		double Ang = pRandf() * 2.0 * M_PI;
		dJet = pVector(cos(Ang), sin(Ang), 0);
		dJet *= 0.01;
	}

	Jet += dJet;
	dirAng += 0.02;
#if 0
	if(Jet.x > 10 || Jet.x < -10) {dJet.x = -dJet.x; dJet.y += pRandf() * 0.005;}
	if(Jet.y > 10 || Jet.y < -10) {dJet.y = -dJet.y; dJet.x += pRandf() * 0.005;}
#else
	Jet = pVector(0,0,2);
#endif

	pColorD(1.0, PDLine, 0.8,0,0, 1,1,0.3);
	pVelocityD(PDBlob, sin(dirAng)*.8,cos(dirAng)*.8,0, 0.03);
	pStartingAge(0);
	pSize(1);

	pCopyVertexB(false, true);

	int LifeTime = 100;
	pKillOld(LifeTime);
	pSource(maxParticles / LifeTime, PDDisc, Jet.x, Jet.y, Jet.z, 0, 0, 1, 0.5);
	pGravity(0, 0, .01);
	pDamping(0.9, 0.97, 0.9);

	float BOX = .017;
	pRandomAccel(PDBox, -BOX, -BOX, -BOX, BOX, BOX, BOX);

	pMove();
}

// A fountain spraying up in the middle of the screen
void ParticleEffects::Fountain(bool FirstTime)
{
	pVelocityD(PDCylinder, 0.0, -0.01, 0.35, 0.0, -0.01, 0.37, 0.021, 0.019);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Fountain";
		pNewActionList(action_handle);
	}

	pSource(maxParticles * 0.01f, PDLine, 0.0, 0.0, 0.0, 0.0, 0.0, 0.405);

	pCopyVertexB(false, true);

	pGravity(0.0, 0.0, -0.01);

	pSinkVelocity(true, PDSphere, 0, 0, 0, 0.01);

	pBounce(-0.05, 0.35, 0, PDDisc, 0, 0, 0,  0, 0, 1,  5);

	pSink(false, PDPlane, 0,0,-3, 0,0,1);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A bunch of particles in a grid shape
void ParticleEffects::GridShape(bool FirstTime)
{
	if(FirstTime) {
		EffectName = "GridShape";

		pKillOld(-1000);
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
					pVelocityD(PDBlob, 0, 0, 0, 0.001);
					///pColor(0.5 + z*0.05, 0.5 + x*0.05, 0.5 + x*0.05);
					pColor(j / float(dim), k / float(dim), l / float(dim));
					pStartingAge(0);
					pVertex(x, y, z);
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
	pVelocityD(PDBlob, 0, 0, 0, 0.01);
	pSize(1.5);

	static float jetx=0, jety=0, jetz=0;
	static float djx = pRandf() * 0.5;
	static float djy = pRandf() * 0.5;

	if(FirstTime) {
		EffectName = "JetSpray";
		pNewActionList(action_handle);
		jetx = 0;
		jety = 0;
		djx = pRandf() * 0.5;
		djy = pRandf() * 0.5;
	}

	pCopyVertexB(false, true);

	pColorD(1.0, PDSphere, .8, .4, .1, .1);
	pSource(1, PDRectangle, -1, -1, 0.1, 2, 0, 0, 0, 2, 0);

	pColorD(1.0, PDSphere, .5, .4, .1, .1);
	pSource(300, PDRectangle, -10, -10, 0.1, 20, 0, 0, 0, 20, 0);

	pGravity(0, 0, -0.01);

	jetx += djx;
	jety += djy;
	if(jetx > 10 || jetx < -10) {djx = -djx; djy += pRandf() * 0.005;}
	if(jety > 10 || jety < -10) {djy = -djy; djx += pRandf() * 0.005;}

	pVelocityD(PDBlob, 0,0,.05, 0.01);
	pJet(jetx, jety, jetz, 1, 0.01, 1.5);

	pBounce(0.1, 0.3, 0.1, PDRectangle, -10, -10, 0.0, 20, 0, 0, 0, 20, 0);

	pSink(false, PDPlane, 0,0,-20, 0,0,1);

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

	pVelocityD(PDBlob, 0.02, -0.2, 0, 0.015);
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

	pCopyVertexB(false, true);

	int LifeTime = 350;

	pKillOld(LifeTime);

	pColorD(1, PDSphere, 0.4+fabs(jetx*0.1), 0.4+fabs(jety*0.1), 0.4+fabs(jetz*0.1), 0.1);
	pSource(maxParticles / LifeTime, PDPoint, jetx, jety, jetz);

	pOrbitPoint(2, 0, 3, 0.1, 0.5);

	pOrbitPoint(-2, 0, -3, 0.1, 0.5);

	pDamping(0.994, 0.994, 0.994);

	//pSink(false, PDSphere, 0, 0, 0, 15);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// It kinda looks like rain hitting a parking lot
void ParticleEffects::Rain(bool FirstTime)
{
	pVelocity(0, 0, 0);
	pColorD(1.0, PDSphere, 0.4, 0.4, 0.9, .1);
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Rain";
		pNewActionList(action_handle);
	}

	pCopyVertexB(false, true);

	pSource(100, PDRectangle, -11, -10, 12, 20, 0, 0, 0, 20, 0);

	pRandomAccel(PDBlob, 0.002, 0, -0.01, 0.003);

	pBounce(0.3, 0.3, 0, PDPlane, 0,0,0, 0,0,1);

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

	pCopyVertexB(false, true);

	pGravity(0.0, 0.0, -0.001);

	pRestore(i-= (1.0f / float(numSteps)));

	pMove();

	if(FirstTime)
		pEndActionList();
}

// Add a bunch of particles in a shaft shape
void ParticleEffects::ShaftShape(bool FirstTime)
{
	if(FirstTime) {
		EffectName = "ShaftShape";
	}

	pVelocity(0,0,0);
	pStartingAge(0);
	pSource(100000, PDCylinder, 5,0,-10,5,0,10,1);
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

	pVelocity(0, 0, 0);
	pSize(1.5);
	pStartingAge(0);
	pColor(jetx, jety, 1);

	pCopyVertexB(false, true);

	pSource(120, PDLine, -5,jety,8, 5,jety,8);

	pGravity(0.0, 0.0, -0.004);

	glColor3f(1,1,0);
	if(SteerShape == PDSphere) {
		pAvoid(0.2, 1.0, 20, PDSphere, 0,0,0, 1.1);

		glutSolidSphere(1, 16, 8);
	} else if(SteerShape == PDTriangle) {
		pAvoid(2, 1.0, 20, PDTriangle, 0,-1,0, 2,0,0, 0,2,0);

		glBegin(GL_TRIANGLES);
		glVertex3f(0,-1,0);
		glVertex3f(2,0,0);
		glVertex3f(0,2,0);
		glEnd();
	} else if(SteerShape == PDRectangle) {
		pAvoid(2, 1.0, 20, PDRectangle, 0,-1,0, 2,1,0, 0,2,0);

		glBegin(GL_QUADS);
		glVertex3f(0,-1,0);
		glVertex3f(2,0,0);
		glVertex3f(2,2,0);
		glVertex3f(0,1,0);
		glEnd();
	} else if(SteerShape == PDPlane) {
		pAvoid(2, 1.0, 20, PDPlane, 0,0,0, 0,0,1);

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

		pVelocity(0, 0, 0);
		pSize(1.0);
		pStartingAge(0);

		pKillOld(-300);
		pColorD(1.0, PDSphere, 0.93, 0.93, 0, 0.05);
		for(float x=-10.0; x<2.0; x+=0.05)
			pVertex(x, 0, 0);
		pNewActionList(action_handle);
	}

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

	pCopyVertexB(false, true);

	pKillOld(-1.0, true);
	pStartingAge(-10.0);
	pColor(1, 0, 0);
	pVertex(jetx, jety, jetz);
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

	pColorD(1.0, PDLine, 0,1,0, 0,0,1);
	pVelocityD(PDBlob, sin(dirAng)*.1,cos(dirAng)*.1, 0.1, 0.01);
	pStartingAge(0);
	pSize(1);

	pCopyVertexB(false, true);
	pKillOld(600);
	pSource(0.8, PDPoint, 1, 1, 6);
	pGravity(0, 0, -.01);
	pBounce(0, 0.55, 0, PDSphere, 0, 0, 4, 6);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A sprayer with particles orbiting a line
void ParticleEffects::Swirl(bool FirstTime)
{
	pVelocityD(PDBlob, 0.02, -0.2, 0, 0.015);
	pSize(1.0);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Swirl";
		pNewActionList(action_handle);
	}

	static float jetx=-4, jety=0, jetz=-2.4;

	static float djx = pRandf() * 0.05;
	static float djy = pRandf() * 0.05;
	static float djz = pRandf() * 0.05;

	jetx += djx;
	jety += djy;
	jetz += djz;

	if(jetx > 10 || jetx < -10) djx = -djx;
	if(jety > 10 || jety < -10) djy = -djy;
	if(jetz > 10 || jetz < -10) djz = -djz;

	pCopyVertexB(false, true);

	int LifeTime = 300;

	pKillOld(LifeTime);

	pColorD(1.0, PDSphere, 0.4+fabs(jetx*0.1), 0.4+fabs(jety*0.1), 0.4+fabs(jetz*0.1), 0.1);
	pSource(maxParticles / LifeTime, PDPoint, jetx, jety, jetz);

	pOrbitLine(0, 0, 1, 1, 0, 0.0, 0.5, 0.1);

	pDamping(1, 0.994, 0.994);

	pSink(false, PDSphere, 0, 0, 0, 15);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A waterfall bouncing off invisible rocks
void ParticleEffects::Waterfall1(bool FirstTime)
{
	pVelocityD(PDBlob, 0.03, -0.001, 0.01, 0.002);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);

	if(FirstTime) {
		EffectName = "Waterfall1";
		pNewActionList(action_handle);
	}

	pCopyVertexB(false, true);
	pSource(100, PDLine, -5, -1, 8, -5, -3, 8);
	pGravity(0.0, 0.0, -0.01);
	pKillOld(300);
	pBounce(0, 0.35, 0, PDRectangle, -7, -4, 7, 3, 0, 0, 0, 3, 0);
	pBounce(0, 0.5, 0, PDSphere, -4, -2, 4, 0.2);
	pBounce(0, 0.5, 0, PDSphere, -3.5, 0, 2, 2);
	pBounce(0, 0.5, 0, PDSphere, 3.8, 0, 0, 2);
	pBounce(-0.01, 0.35, 0, PDPlane, 0,0,0, 0,0,1);
	pSink(false, PDSphere, 0,0,0,20);

	pMove();

	if(FirstTime)
		pEndActionList();
}

// A waterfall bouncing off invisible rocks
void ParticleEffects::Waterfall2(bool FirstTime)
{
	pVelocityD(PDBlob, 0.1, 0, 0.1, 0.004);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);
	pStartingAge(0);

	if(FirstTime) {
		EffectName = "Waterfall2";
		pNewActionList(action_handle);
	}

	pCopyVertexB(false, true);
	pSource(100, PDPoint, -4, 0, 6);
	pGravity(0.0, 0.0, -0.01);
	pKillOld(250);
	pBounce(0, 0.01, 0, PDSphere, -1, 0, 4, 1);
	pBounce(0, 0.01, 0, PDSphere, -2.5, 0, 2, 1);
	pBounce(0, 0.01, 0, PDSphere, 0.7, -0.5, 2, 1);
	pBounce(-0.01, 0.35, 0, PDPlane, 0,0,0, 0,0,1);
	pMove();

	if(FirstTime)
		pEndActionList();
}
