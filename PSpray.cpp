// PSpray.cpp
//
// Copyright 1998 by David K. McAllister
//
// This application demonstrates particle systems for interactive graphics
// using OpenGL and GLUT.

#include <particle/papi.h>

#include <math.h>
#include <string.h>
#include <time.h>

#include <GL/glut.h>

#ifdef __sgi
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif

#ifdef WIN32
#pragma warning (disable:4305) /* disable bogus conversion warnings */
#define drand48() (((float) rand())/((float) RAND_MAX))
#define srand48(x) srand(x)
#endif

bool doubleBuffer = true, MotionBlur = false, FreezeParticles = false;
bool immediate = true, drawGround = false, DoMotion = true, FullScreen = false;

int particle_handle, action_handle, maxParticles = 10000;
int demoNum = 10, numSteps = 1, prim = GL_LINES, listID = -1;
float BlurRate = 0.09;

// A fountain spraying up in the middle of the screen
void Fountain(bool do_list = true)
{
	if(do_list)
		action_handle = pGenActionLists(1);
	
	pVelocityD(PDCylinder, 0.0, -0.01, 0.35, 0.0, -0.01, 0.37, 0.021, 0.019);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);
	
	if(do_list)
		pNewActionList(action_handle);
		
	pCopyVertexB(false, true);

	pSource(150, PDLine, 0.0, 0.0, 0.401, 0.0, 0.0, 0.405);
	
	pGravity(0.0, 0.0, -0.01);
	
	pKillSlow(0.01);
	
	pBounce(-0.05, 0.35, 0, PDPlane, -5, -5, 0.0, 10, 0, 0, 0, 10, 0);
	
	pSink(false, PDPlane, 0, 0, -3, 1, 0, 0, 0, 1, 0);
	
	pMove();
	
	if(do_list)
		pEndActionList();
}

// A waterfall pouring down from above
void Waterfall(bool do_list = true)
{
	if(do_list)
		action_handle = pGenActionLists(1);
	
	pVelocityD(PDBlob, 0.03, -0.001, 0.01, 0.002);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);
	
	if(do_list)
		pNewActionList(action_handle);
		
	pCopyVertexB(false, true);

	pSource(50, PDLine, -5, -1, 8, -5, -3, 8);
	
	pGravity(0.0, 0.0, -0.01);
	
	pKillOld(300);
	
	pBounce(0, 0.35, 0, PDPlane, -7, -4, 7, 3, 0, 0, 0, 3, 0);
	
	pBounce(0, 0.5, 0, PDSphere, -4, -2, 4, 0.2);
	
	pBounce(0, 0.5, 0, PDSphere, -3.5, 0, 2, 2);
	
	pBounce(0, 0.5, 0, PDSphere, 3.8, 0, 0, 2);
	pBounce(-0.01, 0.35, 0, PDPlane, -25, -25, 0.0, 50, 0, 0, 0, 50, 0);
	
	pSink(false, PDSphere, 0,0,0,20);
	
	pMove();
	
	if(do_list)
		pEndActionList();
}

// Restore particles to their positionB,
// which is usually set by Shape().
void Restore(bool do_list = true)
{
	static float i = 0;
	if(do_list)
	{
		action_handle = pGenActionLists(1);
		pNewActionList(action_handle);
		i = 200;
	}
		
	pCopyVertexB(false, true);

	pGravity(0.0, 0.0, -0.001);

	pRestore(i-= (1.0f / float(numSteps)));

	pMove();
	
	if(do_list)
		pEndActionList();
}

// Make a bunch of particles in a particular shape.
void Shape(bool do_list = true)
{
	if(do_list)
	{
		pKillOld(0);
		int dim = int(pow(float(maxParticles), 0.33333333));
#define XX 8
#define YY 12
#define ZZ 8
		
		float dx = 2*XX / float(dim);
		float dy = 2*YY / float(dim);
		float dz = 2*ZZ / float(dim);
		
		float z = -ZZ;
		for(int k=0; k<dim; k++, z += dz)
		{
			float y = -YY;
			for(int l=0; l<dim; l++, y += dy)
			{
				float x = -XX;
				for(int j=0; j<dim; j++, x += dx)
				{
					// Make the particles.
					pVelocityD(PDBlob, 0, 0, 0, 0.001);
					pColor(0.5 + z*0.05, 0.5 + x*0.05, 0.5 + x*0.05);
					pVertex(x, y, z);
					
				}
			}
		}
		
		action_handle = pGenActionLists(1);
		pNewActionList(action_handle);
		pEndActionList();
	}
}

// A fountain spraying up in the middle of the screen
void Atom(bool do_list = true)
{
	if(do_list)
		action_handle = pGenActionLists(1);
	
	pVelocityD(PDSphere, 0, 0, 0, 0.2);
	pColorD(1.0, PDBox, 0, 0, 0, 1.0, 0.5, 0.5);
	pSize(1.5);
	
	if(do_list)
		pNewActionList(action_handle);
		
	pCopyVertexB(false, true);

	pSource(300, PDSphere, 0, 0, 0, 6);
	
	// Orbit about the origin.
	pOrbitPoint(0, 0, 0, 0.05);
	
	// Keep orbits from being too eccentric.
	pSink(true, PDSphere, 0.0, 0.0, 0.0, 1.0);
	pSink(false, PDSphere, 0.0, 0.0, 0.0, 8.0);
	
	pMove();
	
	if(do_list)
		pEndActionList();
}

void JetSpray(bool do_list = true)
{
	if(do_list)
		action_handle = pGenActionLists(1);
	
	pVelocity(0, 0, 0);
	pSize(1.5);
	
	if(do_list)
		pNewActionList(action_handle);
		
	pCopyVertexB(false, true);

	pColorD(1.0, PDSphere, .8, .4, .1, .1);
	pSource(1, PDPlane, -1, -1, 0.1, 2, 0, 0, 0, 2, 0);
	
	pColorD(1.0, PDSphere, .5, .4, .1, .1);
	pSource(300, PDPlane, -10, -10, 0.1, 20, 0, 0, 0, 20, 0);
	
	pGravity(0, 0, -0.01);
	
	static float jetx=0, jety=0, jetz=0;
	static float djx = drand48() * 0.5;
	static float djy = drand48() * 0.5;
	
	if(do_list)
	{
		jetx = 0;
		jety = 0;
		djx = drand48() * 0.5;
		djy = drand48() * 0.5;
	}
	
	jetx += djx;
	jety += djy;
	
	if(jetx > 10 || jetx < -10) {djx = -djx; djy += drand48() * 0.005;}
	if(jety > 10 || jety < -10) {djy = -djy; djx += drand48() * 0.005;}
	
	pVelocityD(PDBlob, 0,0,.05, 0.01);
	pJet(jetx, jety, jetz, 1, 0.01, 1.5);
	
	pBounce(0.3, 0.3, 0, PDPlane, -10, -10, 0.0, 20, 0, 0, 0, 20, 0);
	
	pSink(false, PDPlane, -10, -10, -20.0, 20, 0, 0, 0, 20, 0);
	
	pMove();
	
	if(do_list)
		pEndActionList();
}

void Explosion(bool do_list = true)
{
	if(do_list)
		action_handle = pGenActionLists(1);
	
	pVelocityD(PDSphere, 0,0,0,0.01,0.01);
	pColorD(1.0, PDSphere, 0.5, 0.7, 0.5, .3);
	pSize(1.0);
	
	if(do_list)
		pNewActionList(action_handle);
	
	pCopyVertexB(false, true);
	
	pDamping(0.999, 0.999, 0.999);
	
	static float i=0;
	
	if(do_list)
		i = 0;
	
	pOrbitPoint(0, 0, 0, .02, 0.1);
	
	pExplosion(0, 0, 0, 1, 2, 3, 0.1, i+= (1.0f / float(numSteps)));
	
	pSink(false, PDSphere, 0, 0, 0, 30);
	
	pMove();
	
	if(do_list)
		pEndActionList();
}

void Swirl(bool do_list = true)
{
	if(do_list)
		action_handle = pGenActionLists(1);
	
	pVelocityD(PDBlob, 0.02, -0.2, 0, 0.015);
	pSize(1.0);
	
	if(do_list)
		pNewActionList(action_handle);
	
	static float jetx=-4, jety=0, jetz=-2.4;
	
	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;
	
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
	
	pOrbitLine(0, 0, 1, 1, 0, 0.0, 0.1, 0.1, 99);
	
	pDamping(1, 0.994, 0.994);
	
	pSink(false, PDSphere, 0, 0, 0, 15);
	
	pMove();
	
	if(do_list)
		pEndActionList();
}

void Chaos(bool do_list = true)
{
	if(do_list)
		action_handle = pGenActionLists(1);
	
	pVelocityD(PDBlob, 0.02, -0.2, 0, 0.015);
	pSize(1.0);
	
	if(do_list)
		pNewActionList(action_handle);
	
	static float jetx=-4, jety=0, jetz=-2.4;
	
	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;
	
	jetx += djx;
	jety += djy;
	jetz += djz;
	
	if(jetx > 10 || jetx < -10) djx = -djx;
	if(jety > 10 || jety < -10) djy = -djy;
	if(jetz > 10 || jetz < -10) djz = -djz;
	
	pCopyVertexB(false, true);
	
	int LifeTime = 350;

	pKillOld(LifeTime);
	
	pColorD(1.0, PDSphere, 0.4+fabs(jetx*0.1), 0.4+fabs(jety*0.1), 0.4+fabs(jetz*0.1), 0.1);
	pSource(maxParticles / LifeTime, PDPoint, jetx, jety, jetz);
	
	pOrbitPoint(2, 0, 3, 0.1, 0.1, 99);
	
	pOrbitPoint(-2, 0, -3, 0.1, 0.1, 99);
	
	pDamping(0.994, 0.994, 0.994);
	
	//pSink(false, PDSphere, 0, 0, 0, 15);
	
	pMove();
	
	if(do_list)
		pEndActionList();
}

void Snake(bool do_list = true)
{
	if(do_list)
	{
		action_handle = pGenActionLists(1);
		
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
	
	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;
	
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
	
	if(do_list)
		pEndActionList();
}

// A fountain spraying up in the middle of the screen
void FireFlies(bool do_list = true)
{
	if(do_list)
		action_handle = pGenActionLists(1);
	
	pSize(1.0);
	pVelocityD(PDPoint, 0,0,0);
	pColorD(1.0, PDSphere, .5, .4, .1, .1);
	
	if(do_list)
		pNewActionList(action_handle);
		
	pCopyVertexB(false, true);

	pSource(1, PDBlob, 0, 0, 2, 2);
	
	pRandomAccel(PDSphere, 0, 0, 0, 0.02);
	
	pKillOld(20);
	
	pMove();
	
	if(do_list)
		pEndActionList();
}

void CallDemo(bool initial)
{
	if(!immediate && !initial)
	{
		pCallActionList(action_handle);
		return;
	}
	
	switch(demoNum)
	{
	case 0:
		Fountain(initial);
		break;
	case 1:
		Atom(initial);
		break;
	case 2:
		JetSpray(initial);
		break;
	case 3:
		Explosion(initial);
		break;
	case 4:
		Swirl(initial);
		break;
	case 5:
		FireFlies(initial);
		break;
	case 6:
		Waterfall(initial);
		break;
	case 7:
		Restore(initial);
		break;
	case 8:
		Shape(initial);
		break;
	case 9:
		Snake(initial);
		break;
	case 10:
		Chaos(initial);
		break;
	default:
		cerr << "Bad demo number!\n";
		break;
	}
}

void Draw()
{
	glLoadIdentity();
	
	if(MotionBlur)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		
		glDrawBuffer(GL_FRONT);
		
		// glEnable(GL_BLEND);
		
		glColor4f(0, 0, 0, BlurRate);
		glRectf(-1, -1, 1, 1);
#ifdef DEPTH_TEST
		glClear(GL_DEPTH_BUFFER_BIT);
#endif
		// glDisable(GL_BLEND);
		
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	else
	{
#ifdef DEPTH_TEST
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
		glClear(GL_COLOR_BUFFER_BIT);
#endif
	}
	
	static double t = -23.0;
	static double dt = -0.05;
	if(DoMotion)
		t += dt;
	if(t < -25.0 || t > -10.0) dt = -dt;
	gluLookAt(0, t, 10, 0, 0, 0, 0, 0, 1);
	
	if(drawGround)
	{
		glBegin(GL_TRIANGLE_STRIP);
		glColor3ub(128, 64, 128);
		glVertex3f(-5,-5,0);
		glColor3ub(0, 64, 128);
		glVertex3f(-5,5,0);
		glColor3ub(128, 64, 0);
		glVertex3f(5,-5,0);
		glColor3ub(64, 128, 0);
		glVertex3f(5,5,0);
		glEnd();
	}
	
	if(!FreezeParticles)
	{
		for(int step = 0; step < numSteps; step++)
		{
			CallDemo(false);
		}
	}
	
	//int count = pGetGroupCount();
	//cerr << count << endl;
	
	if(prim < 0)
		pDrawGroupl(listID);
	else
		pDrawGroupp(prim);
	
	if(doubleBuffer && !MotionBlur)
		glutSwapBuffers();
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, w / double(h), 1, 100);
	glMatrixMode(GL_MODELVIEW);
	
	// Useful for motion blur so BG doesn't get ugly.
#ifdef DEPTH_TEST
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
		glClear(GL_COLOR_BUFFER_BIT);
#endif
}


// Make a Monarch Butterfly by hand. I can't believe I ever did this.
void monarch(int ListNum)
{
    glNewList(ListNum, GL_COMPILE);
	
	glScalef(4.0, 4.0, 4.0);
	glRotatef(-90.0, 0, 0, 1);

    // Left Wing
    glBegin(GL_TRIANGLE_FAN);
	
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(-0.075, 0.0, 0.0);
	
    glNormal3f(0.707, 0.707, 0.000);
    glVertex3f(-0.015, 0.0, 0.0);
    glNormal3f(0.659, 0.707, 0.255);
    glVertex3f(-0.019, 0.030, 0.0);
    glNormal3f(0.523, 0.707, 0.476);
    glVertex3f(-0.015, 0.060, 0.0);
    glNormal3f(0.315, 0.707, 0.633);
    glVertex3f(-0.045, 0.069, 0.0);
    glNormal3f(0.065, 0.707, 0.704);
    glVertex3f(-0.075, 0.075, 0.0);
    glNormal3f(-0.194, 0.707, 0.680);
    glVertex3f(-0.105, 0.069, 0.0);
    glNormal3f(-0.426, 0.707, 0.564);
    glVertex3f(-0.135, 0.060, 0.0);
    glNormal3f(-0.601, 0.707, 0.372);
    glVertex3f(-0.150, 0.030, 0.0);
    glNormal3f(-0.695, 0.707, 0.130);
    glVertex3f(-0.141, 0.0, 0.0);
    glNormal3f(-0.695, 0.707, -0.130);
    glVertex3f(-0.126, -0.030, 0.0);
    glNormal3f(-0.601, 0.707, -0.372);
    glVertex3f(-0.108, -0.060, 0.0);
    glNormal3f(-0.426, 0.707, -0.564);
    glVertex3f(-0.075, -0.084, 0.0);
    glNormal3f(-0.194, 0.707, -0.680);
    glVertex3f(-0.045, -0.129, 0.0);
    glNormal3f(0.065, 0.707, -0.704);
    glVertex3f(-0.030, -0.120, 0.0);
    glNormal3f(0.315, 0.707, -0.633);
    glVertex3f(-0.009, -0.105, 0.0);
    glNormal3f(0.523, 0.707, -0.476);
    glVertex3f(-0.015, -0.060, 0.0);
    glNormal3f(0.659, 0.707, -0.255);
    glVertex3f(-0.015, -0.030, 0.0);
    glNormal3f(0.707, 0.707, -0.000);
    glVertex3f(-0.015, -0.0, 0.0);
	
    glEnd();
	
    // Right Wing
    glBegin(GL_TRIANGLE_FAN);
	
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(0.075, 0.0, 0.0);
	
    glNormal3f(0.707, 0.707, 0.000);
    glVertex3f(0.015, -0.0, 0.0);
    glNormal3f(0.659, 0.707, 0.255);
    glVertex3f(0.015, -0.030, 0.0);
    glNormal3f(0.523, 0.707, 0.476);
    glVertex3f(0.015, -0.060, 0.0);
    glNormal3f(0.315, 0.707, 0.633);
    glVertex3f(0.009, -0.105, 0.0);
    glNormal3f(0.065, 0.707, 0.704);
    glVertex3f(0.030, -0.120, 0.0);
    glNormal3f(-0.194, 0.707, 0.680);
    glVertex3f(0.045, -0.129, 0.0);
    glNormal3f(-0.426, 0.707, 0.564);
    glVertex3f(0.075, -0.084, 0.0);
    glNormal3f(-0.601, 0.707, 0.372);
    glVertex3f(0.108, -0.060, 0.0);
    glNormal3f(-0.695, 0.707, 0.130);
    glVertex3f(0.126, -0.030, 0.0);
    glNormal3f(-0.695, 0.707, -0.130);
    glVertex3f(0.141, 0.0, 0.0);
    glNormal3f(-0.601, 0.707, -0.372);
    glVertex3f(0.150, 0.030, 0.0);
    glNormal3f(-0.426, 0.707, -0.564);
    glVertex3f(0.135, 0.060, 0.0);
    glNormal3f(-0.194, 0.707, -0.680);
    glVertex3f(0.105, 0.069, 0.0);
    glNormal3f(0.065, 0.707, -0.704);
    glVertex3f(0.075, 0.075, 0.0);
    glNormal3f(0.315, 0.707, -0.633);
    glVertex3f(0.045, 0.069, 0.0);
    glNormal3f(0.523, 0.707, -0.476);
    glVertex3f(0.015, 0.060, 0.0);
    glNormal3f(0.659, 0.707, -0.255);
    glVertex3f(0.019, 0.030, 0.0);
    glNormal3f(0.707, 0.707, -0.000);
    glVertex3f(0.015, 0.0, 0.0);
	
    glEnd();
	
	glColor3f(.5, .5, 1);

    // Body contour
	glBegin(GL_QUAD_STRIP);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.000, 0.081, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.000, 0.081, 0.000);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.007, 0.078, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.005, 0.078, 0.005);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.012, 0.072, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.008, 0.072, 0.008);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.015, 0.060, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.011, 0.060, 0.011);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.019, 0.030, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.014, 0.030, 0.014);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.021, 0.000, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.015, 0.000, 0.015);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.021, -0.030, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.015, -0.030, 0.015);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.015, -0.060, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.011, -0.060, 0.011);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.009, -0.105, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.006, -0.105, 0.006);
    glNormal3f(1.000, 0.000, 0.000);
    glVertex3f(0.000, -0.120, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.000, -0.120, 0.000);
	glEnd();
	
	glBegin(GL_QUAD_STRIP);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.000, 0.081, 0.000);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.081, 0.000);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.005, 0.078, 0.005);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.078, 0.007);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.008, 0.072, 0.008);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.072, 0.012);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.011, 0.060, 0.011);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.060, 0.015);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.014, 0.030, 0.014);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.030, 0.019);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.015, 0.000, 0.015);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.000, 0.021);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.015, -0.030, 0.015);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, -0.030, 0.021);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.011, -0.060, 0.011);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, -0.060, 0.015);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.006, -0.105, 0.006);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, -0.105, 0.009);
    glNormal3f(0.707, 0.000, 0.707);
    glVertex3f(0.000, -0.120, 0.000);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, -0.120, 0.000);
	glEnd();
	
	glBegin(GL_QUAD_STRIP);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.081, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.000, 0.081, 0.000);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.078, 0.007);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.005, 0.078, 0.005);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.072, 0.012);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.008, 0.072, 0.008);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.060, 0.015);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.011, 0.060, 0.011);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.030, 0.019);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.014, 0.030, 0.014);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, 0.000, 0.021);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.015, 0.000, 0.015);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, -0.030, 0.021);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.015, -0.030, 0.015);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, -0.060, 0.015);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.011, -0.060, 0.011);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, -0.105, 0.009);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.006, -0.105, 0.006);
    glNormal3f(0.000, 0.000, 1.000);
    glVertex3f(0.000, -0.120, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.000, -0.120, 0.000);
	glEnd();
	
	glBegin(GL_QUAD_STRIP);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.000, 0.081, 0.000);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.000, 0.081, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.005, 0.078, 0.005);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.007, 0.078, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.008, 0.072, 0.008);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.012, 0.072, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.011, 0.060, 0.011);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.015, 0.060, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.014, 0.030, 0.014);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.019, 0.030, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.015, 0.000, 0.015);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.021, 0.000, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.015, -0.030, 0.015);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.021, -0.030, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.011, -0.060, 0.011);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.015, -0.060, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.006, -0.105, 0.006);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.009, -0.105, 0.000);
    glNormal3f(-0.707, 0.000, 0.707);
    glVertex3f(-0.000, -0.120, 0.000);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.000, -0.120, 0.000);
	glEnd();
	
	glBegin(GL_QUAD_STRIP);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.000, 0.081, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.000, 0.081, -0.000);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.007, 0.078, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.005, 0.078, -0.005);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.012, 0.072, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.008, 0.072, -0.008);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.015, 0.060, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.011, 0.060, -0.011);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.019, 0.030, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.014, 0.030, -0.014);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.021, 0.000, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.015, 0.000, -0.015);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.021, -0.030, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.015, -0.030, -0.015);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.015, -0.060, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.011, -0.060, -0.011);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.009, -0.105, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.006, -0.105, -0.006);
    glNormal3f(-1.000, 0.000, 0.000);
    glVertex3f(-0.000, -0.120, 0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.000, -0.120, -0.000);
	glEnd();
	
	glBegin(GL_QUAD_STRIP);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.000, 0.081, -0.000);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.081, -0.000);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.005, 0.078, -0.005);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.078, -0.007);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.008, 0.072, -0.008);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.072, -0.012);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.011, 0.060, -0.011);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.060, -0.015);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.014, 0.030, -0.014);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.030, -0.019);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.015, 0.000, -0.015);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.000, -0.021);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.015, -0.030, -0.015);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, -0.030, -0.021);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.011, -0.060, -0.011);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, -0.060, -0.015);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.006, -0.105, -0.006);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, -0.105, -0.009);
    glNormal3f(-0.707, 0.000, -0.707);
    glVertex3f(-0.000, -0.120, -0.000);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, -0.120, -0.000);
	glEnd();
	
	glBegin(GL_QUAD_STRIP);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.081, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.000, 0.081, -0.000);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.078, -0.007);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.005, 0.078, -0.005);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.072, -0.012);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.008, 0.072, -0.008);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.060, -0.015);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.011, 0.060, -0.011);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.030, -0.019);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.014, 0.030, -0.014);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, 0.000, -0.021);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.015, 0.000, -0.015);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, -0.030, -0.021);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.015, -0.030, -0.015);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, -0.060, -0.015);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.011, -0.060, -0.011);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, -0.105, -0.009);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.006, -0.105, -0.006);
    glNormal3f(-0.000, 0.000, -1.000);
    glVertex3f(-0.000, -0.120, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.000, -0.120, -0.000);
	glEnd();
	
	glBegin(GL_QUAD_STRIP);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.000, 0.081, -0.000);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.000, 0.081, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.005, 0.078, -0.005);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.007, 0.078, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.008, 0.072, -0.008);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.012, 0.072, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.011, 0.060, -0.011);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.015, 0.060, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.014, 0.030, -0.014);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.019, 0.030, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.015, 0.000, -0.015);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.021, 0.000, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.015, -0.030, -0.015);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.021, -0.030, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.011, -0.060, -0.011);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.015, -0.060, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.006, -0.105, -0.006);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.009, -0.105, -0.000);
    glNormal3f(0.707, 0.000, -0.707);
    glVertex3f(0.000, -0.120, -0.000);
    glNormal3f(1.000, 0.000, -0.000);
    glVertex3f(0.000, -0.120, -0.000);
	glEnd();
	
	
    glEndList();
}

void
menu(int item)
{
	static int OldWidth, OldHeight;

	switch(item)
	{
	case 'F':
		demoNum = 0;
		CallDemo(true);
		break;
	case 'A':
		demoNum = 1;
		CallDemo(true);
		break;
	case 'J':
		demoNum = 2;
		CallDemo(true);
		break;
	case 'E':
	case ' ':
		demoNum = 3;
		CallDemo(true);
		break;
	case 'T':
		demoNum = 4;
		CallDemo(true);
		break;
	case 'I':
		demoNum = 5;
		CallDemo(true);
		break;
	case 'W':
		demoNum = 6;
		CallDemo(true);
		break;
	case 'R':
		demoNum = 7;
		CallDemo(true);
		break;
	case 'S':
		demoNum = 8;
		CallDemo(true);
		break;
	case 'C':
		demoNum = 10;
		CallDemo(true);
		break;
	case 'N':
		demoNum = 9;
		CallDemo(true);
		break;
	case 'i':
		immediate = !immediate;
		cerr << "Switching to " << (immediate ? "immediate":"action list") << " mode.\n";
		CallDemo(true);
		break;
	case 'm':
		MotionBlur = !MotionBlur;
		if(!MotionBlur)
			glDrawBuffer(GL_BACK);
		break;
	case 'c':
		DoMotion = !DoMotion;
		break;
	case 'd':
		prim = -1;
		if(listID < 0)
		{
			listID = glGenLists(1);
#if 1
			monarch(listID);
#else
			glNewList(listID, GL_COMPILE);
			glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(1, 0, 0);
			glEnd();
			glEndList();
#endif
		}
		break;
	case 'f':
		FullScreen = !FullScreen;
		if(FullScreen)
		{
			OldWidth = glutGet(GLenum(GLUT_WINDOW_WIDTH));
			OldHeight = glutGet(GLenum(GLUT_WINDOW_HEIGHT));
			glutSetCursor(GLUT_CURSOR_NONE);
			glutFullScreen(); 
		}
		else
		{
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			glutReshapeWindow(OldWidth, OldHeight);
		}
		break;
	case 'p':
		prim = GL_POINTS;
		break;
	case 'l':
		prim = GL_LINES;
		break;
	case 'g':
		drawGround = !drawGround;
		break;
	case 'z':
		pKillSlow(0.001);
		break;
	case 'x':
		FreezeParticles = !FreezeParticles;
		break;
	case '+':
		maxParticles += 100;
		pSetMaxParticles(maxParticles);
		cerr << maxParticles << endl;
		break;
	case '-':
		maxParticles -= 100;
		if(maxParticles<0) maxParticles = 0;
		pSetMaxParticles(maxParticles);
		cerr << maxParticles << endl;
		break;
	case '>':
		BlurRate -= 0.01;
		if(BlurRate < 0)
			BlurRate = 0;
		break;
	case '<':
		BlurRate += 0.01;
		if(BlurRate >= 1)
			BlurRate = 1;
		break;
	case 'q':
	case '\033':   /* ESC key: quit */
		exit(0);
		break;
	}
	
	if(item > '0' && item <= '9')
	{
		numSteps = item - '0';
		pTimeStep(1 / float(numSteps));
	}
	
	glutPostRedisplay();
}

void
KeyPress(unsigned char key, int x, int y)
{
	menu((int) key);
}

static void
Usage(char *program_name, char *message)
{
	if (message)
		cerr << message << endl;
	
	cerr << "Usage: " << program_name << endl;
	cerr << "-db\t\tdo double buffering\n";
	cerr << "-sb\t\tdo single buffering (default)\n";
	exit(1);
}

static void
Args(int argc, char **argv)
{
	char *program = argv[0];
	
	while (--argc)
	{
		++argv;
		
		if (!strcmp("-h", argv[0]) || !strcmp("-help", argv[0]))
			Usage(program, NULL);
		else if (!strcmp("-db", argv[0]) || !strcmp("-double", argv[0]))
			doubleBuffer = true;
		else if (!strcmp("-sb", argv[0]) || !strcmp("-single", argv[0]))
			doubleBuffer = false;
		else
			Usage(program, "Invalid option!");
	}
}

int main(int argc, char **argv)
{
	srand48( (unsigned)time( NULL ) );

	glutInit(&argc, argv);
	Args(argc, argv);
	
	int type = GLUT_RGB;
#ifdef DEPTH_TEST
	type |= GLUT_DEPTH;
#endif
	type |= GLUT_MULTISAMPLE;
	type |= doubleBuffer ? GLUT_DOUBLE : GLUT_SINGLE;
	glutInitDisplayMode(type);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Particle Test");
	
	glutDisplayFunc(Draw);
	glutIdleFunc(Draw);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(KeyPress);
	
	glutCreateMenu(menu);
	glutAddMenuEntry("h: Help on Interface", 'h');
	glutAddMenuEntry("1: 1 step per frame", '1');
	glutAddMenuEntry("2: 1 step per frame", '2');
	glutAddMenuEntry("3: 1 step per frame", '3');
	glutAddMenuEntry("F: Fountain", 'F');
	glutAddMenuEntry("A: Atom", 'A');
	glutAddMenuEntry("J: JetSpray", 'J');
	glutAddMenuEntry("T: Twister", 'T');
	glutAddMenuEntry("S: Shape", 'S');
	glutAddMenuEntry("I: FireFlies", 'I');
	glutAddMenuEntry("W: Waterfall", 'W');
	glutAddMenuEntry("C: Chaos", 'C');
	glutAddMenuEntry("R: Restore", 'R');
	glutAddMenuEntry("N: Snake", 'N');
	glutAddMenuEntry(" : Explosion", ' ');
	glutAddMenuEntry("g: Draw ground", 'g');
	glutAddMenuEntry("p: Use GL_POINTS", 'p');
	glutAddMenuEntry("l: Use GL_LINES", 'l');
	glutAddMenuEntry("m: Toggle motion blur", 'm');
	glutAddMenuEntry("c: Toggle camera motion", 'c');
	glutAddMenuEntry("i: Toggle immed mode", 'i');
	glutAddMenuEntry("+: 100 more particles", '+');
	glutAddMenuEntry("-: 100 less particles", '-');
	glutAddMenuEntry(">: More motion blur", '>');
	glutAddMenuEntry("<: Less motion blur", '<');
	glutAddMenuEntry("x: Freeze Particles", 'x');
	
	glutAddMenuEntry("<esc> or q: exit program", '\033');
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
#ifdef DEPTH_TEST
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
#endif

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glLineWidth(1.0);
	glPointSize(3.0);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	
	// Make a particle group
	particle_handle = pGenParticleGroups(1, maxParticles);
	
	pCurrentGroup(particle_handle);
	
	CallDemo(true);
	
	glutMainLoop();
	
	return 0;
}
