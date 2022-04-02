// Boids.cpp - Demonstrate the use of the Particle System API for Boids.
//
// This one has a flock of birds flying around.
// It can read in a 3D model file of polygons to steer to avoid.
//
// WARNING: This code never worked well and has not been maintained, as you can see.
//
// Copyright 1999 by David K. McAllister

#include <particle/papi.h>

#include <Util/Timer.h>
#include <Util/Utils.h>
#include <Math/Vector.h>
#include <Model/Model.h>

#include <GL/glut.h>

#include <math.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#ifdef __sgi
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif

// XXX extern void RenderModel(Model &M);

Timer Clock;
Model Mod;

bool FreezeParticles = false, AntiAlias = true, drawGround = true, FullScreen = false;
bool ShowText = true, ConstColor = false, DoMotion = false;
bool BoidGravity = true, BoidDamping = true, BoidFollowPoint = true, BoidCentering = true;
bool BoidMatchVel = true, BoidNgbrCol = true, BoidAvoid = true;

int maxParticles = 100;
int numSteps = 1, prim = -1, listID = -1;
#define BOIDLEN 0.3

void BoidPrep()
{
	pSetMaxParticles(0);
	pSetMaxParticles(maxParticles);
	pColorD(1, PDBox, 0,0,0, 1,1,1);
	pSize(BOIDLEN, BOIDLEN, BOIDLEN);
	pStartingAge(0);
#if 0
	pVelocityD(PDBlob, 0, 0, 0, 0.02);
#else
	pVelocityD(PDPoint, 0.04, 0, -0.08);
#endif
	pSource(100000, PDBox, -7,-7,7, -1,7,11);
}

void AvoidModel(Model &M)
{
	for(int i=0; i<M.Objs.size(); i++)
    {
		Object &Ob = M.Objs[i];
		ASSERT0(Ob.verts.size() % 3 == 0);
		
		for(int j=0; j<Ob.verts.size(); j+=3)
		{
			//	  cerr << "Avoid: " << Ob.verts[j] << Ob.verts[j+1] << Ob.verts[j+2] << endl;
			pAvoid(0.1, 1.0, 100, PDTriangle, Ob.verts[j].x, Ob.verts[j].y, Ob.verts[j].z,
				Ob.verts[j+1].x, Ob.verts[j+1].y, Ob.verts[j+1].z,
				Ob.verts[j+2].x, Ob.verts[j+2].y, Ob.verts[j+2].z);
		}
    }
}

// Boids
void Boids()
{
#define GOAL_SPEED 0.05
	static Vector goal(0,0,4);
	static Vector dgoal(DRand(), DRand(), DRand());
	dgoal.normalize();
	dgoal *= GOAL_SPEED;
	
	goal += dgoal;
	
	if(goal.x > 16 || goal.x < -16) dgoal.x = -dgoal.x;
	if(goal.y > 16 || goal.y < -16) dgoal.y = -dgoal.y;
	if(goal.z > 7 || goal.z < 3) dgoal.z = -dgoal.z;
	
	pCopyVertexB(false, true);
	
	if(BoidGravity) pGravity(0, 0, -0.0001);
	
#define DAMPING 0.996
	if(BoidDamping) pDamping(DAMPING, DAMPING, DAMPING);
	
	if(BoidFollowPoint) pOrbitPoint(goal.x, goal.y, goal.z, 0.001, 0.1); // Follow goal.
	
	if(BoidCentering) pGravitate(BOIDLEN*0.0006, BOIDLEN*7); // Flock centering.
	
	if(BoidMatchVel) pMatchVelocity(BOIDLEN*0.003, BOIDLEN*5); // Velocity matching.
	
	if(BoidNgbrCol) pGravitate(-BOIDLEN*0.0005, BOIDLEN*2); // Neighbor collision avoidance.
	
	if(BoidAvoid)
	{ // Static collision avoidance.
		pAvoid(0.1, 1.0, 100, PDPlane, 0,0,0, 0,0,1);
		AvoidModel(Mod);
		
		// Draw the model.
		// XXX RenderModel(Mod);
	}
	
	//if(BoidAvoid) pBounce(0, 1, 0, PDPlane, 0,0,0, 0,0,1); // Static collision avoidance.
	
	pSpeedLimit(0.03);
	
	pMove();
	
	if(BoidFollowPoint)
	{
		// Draw the point.
		glPushMatrix();
		glColor3f(1,1,0);
		glTranslatef(goal.x, goal.y, goal.z);
		glutSolidSphere(0.4, 16, 8);
		glPopMatrix();
	}
}

static void showBitmapMessage(GLfloat x, GLfloat y, GLfloat z, char *message)
{
	if(message == NULL)
		return;
	
#ifndef SCI_MACHINE_hp
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
#endif
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glRasterPos2f(x, y);
	while (*message) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *message);
		message++;
	}
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
#ifndef SCI_MACHINE_hp
	glPopAttrib();
#endif
}

// Make a Monarch Butterfly by hand. I can't believe I ever did this.
// This one's for you, Tiffany!
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

void Draw()
{
#define NUM_FRAMES 30
	static double eltime = 1.0;
	static int NumFrames = 0;
	
	glLoadIdentity();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	static float t = -23.0;
	static float dt = -0.05;
	if(DoMotion)
		t += dt;
	if(t < -25.0 || t > -10.0) dt = -dt;
	gluLookAt(0, -25, 3, 0, 0, 3, 0, 0, 1);
	// gluLookAt(0, t, 3, 0, 0, 0, 0, 0, 1);
	
	glRotatef(t*6, 0,0,1);
	
	if(drawGround)
	{
		glBegin(GL_QUADS);
		glColor3ub(0, 115, 0);
		glVertex3f(-35,-35,0);
		glColor3ub(0, 5, 140);
		glVertex3f(-35,35,0);
		glColor3ub(0, 5, 140);
		glVertex3f(35,35,0);
		glColor3ub(0, 115, 0);
		glVertex3f(35,-35,0);
		glEnd();
	}
	
	if(!FreezeParticles)
	{
		for(int step = 0; step < numSteps; step++)
		{
			Boids();
		}
	}
	
	GL_ASSERT();
	
	if(prim < 0)
	{
		if(listID < 0)
		{
			listID = glGenLists(1);
			monarch(listID);
		}
		pDrawGroupl(listID, true, ConstColor);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		pDrawGroupp(prim, true, ConstColor);
	}
	
	GL_ASSERT();
	
	// Draw the text.
	if(ShowText)
	{
		glLoadIdentity();
		glColor3f(1, 0, 0);
		char msg[64];
		sprintf(msg, "%02.3f fps", double(NUM_FRAMES) / eltime);
		showBitmapMessage(-0.9f, 0.85f, 0.0f, msg);
		sprintf(msg, "%c%c%c%c%c%c%c", BoidGravity?'G':' ', BoidDamping?'D':' ',
			BoidFollowPoint?'F':' ', BoidCentering?'C':' ', BoidMatchVel?'M':' ',
			BoidNgbrCol?'N':' ', BoidAvoid?'A':' ');
		showBitmapMessage(-0.4f, 0.85f, 0.0f, msg);
		sprintf(msg, "%d particles drawn", pGetGroupCount());
		showBitmapMessage(0.1f, 0.85f, 0.0f, msg);
	}
	
	NumFrames++;
	if(NumFrames >= NUM_FRAMES)
	{
		double c = Clock.Reset();
		eltime = c;
		Clock.Start();
		NumFrames = 0;
	}
	
	glutSwapBuffers();
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, w / double(h), 1, 100);
	glMatrixMode(GL_MODELVIEW);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
menu(int item)
{
	static int OldWidth, OldHeight;
	
	switch(item)
	{
	case 'G':
		BoidGravity = !BoidGravity;
		break;
	case 'D':
		BoidDamping = !BoidDamping;
		break;
	case 'F':
		BoidFollowPoint = !BoidFollowPoint;
		break;
	case 'C':
		BoidCentering = !BoidCentering;
		break;
	case 'M':
		BoidMatchVel = !BoidMatchVel;
		break;
	case 'N':
		BoidNgbrCol = !BoidNgbrCol;
		break;
	case 'A':
		BoidAvoid = !BoidAvoid;
		break;
	case 't':
		ShowText = !ShowText;
		break;
	case ' ':
		BoidPrep();
		break;
	case 'c':
		DoMotion = !DoMotion;
		break;
	case 'v':
		ConstColor = !ConstColor;
		break;
	case 's':
		AntiAlias = !AntiAlias;
		if(AntiAlias)
		{
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_POINT_SMOOTH);
		}
		else
		{
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_POINT_SMOOTH);
		}
		break;
	case 'd':
		prim = -1;
		glDisable(GL_TEXTURE_2D);
		break;
	case 'w':
		{
			static int kk = 0;
			kk++;
			if(kk>3)kk=0;
			switch(kk)
			{
			case 0:
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				cerr << "REPLACE\n";
				break;
			case 1:
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				cerr << "MODULATE\n";
				break;
			case 2:
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
				cerr << "DECAL\n";
				break;
			case 3:
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
				cerr << "BLEND\n";
				break;
			}
			break;
		}
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
		glDisable(GL_TEXTURE_2D);
		break;
	case 'l':
		prim = GL_LINES;
		glDisable(GL_TEXTURE_2D);
		break;
	case 'g':
		drawGround = !drawGround;
		break;
	case 'z':
		pSinkVelocity(true, PDSphere, 0, 0, 0, 0.01);
		break;
	case 'x':
		FreezeParticles = !FreezeParticles;
		break;
	case '+':
		maxParticles += 50;
		pSetMaxParticles(maxParticles);
		cerr << maxParticles << endl;
		break;
	case '-':
		maxParticles -= 50;
		if(maxParticles<0) maxParticles = 0;
		pSetMaxParticles(maxParticles);
		cerr << maxParticles << endl;
		break;
	case 'q':
	case '\033': /* ESC key: quit */
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
	cerr << "-model <model_name.wrl> | <model_name.obj>\n";
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
		else if (!strcmp("-m", argv[0]) || !strcmp("-model", argv[0]))
		{
			Mod = Model(argv[1]);
			Mod.RebuildBBox();
			
			Mod.Flatten();
			
			cerr << "BBox is " << Mod.Box << endl;
			float len = Mod.Box.MaxDim();
			Vector Ctr = Mod.Box.Center();
			
			Matrix44 Mat;
			Mat.Scale(10./len);
			Mat.Rotate(1.57, Vector(0,0,1));
			Mat.Translate(Vector(-Ctr.x, -Ctr.y, -Mod.Box.MinV.z));
			
			Mod.Transform(Mat);
			
			argv++; argc--;
		}
		else
			Usage(program, "Invalid option!");
	}
}

int main(int argc, char **argv)
{
	SRand();
	
	glutInit(&argc, argv);
	Args(argc, argv);
	
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Particle Boids");
	
	glutDisplayFunc(Draw);
	glutIdleFunc(Draw);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(KeyPress);
	
	glutCreateMenu(menu);
	glutAddMenuEntry("G: BoidGravity", 'G');
	glutAddMenuEntry("D: BoidDamping", 'D');
	glutAddMenuEntry("F: BoidFollowPoint", 'F');
	glutAddMenuEntry("C: BoidCentering", 'C');
	glutAddMenuEntry("M: BoidMatchVel", 'M');
	glutAddMenuEntry("N: BoidNgbrCol", 'N');
	glutAddMenuEntry("A: BoidAvoid", 'A');
	
	glutAddMenuEntry("1: 1 step per frame", '1');
	glutAddMenuEntry("2: 2 steps per frame", '2');
	glutAddMenuEntry("3: 3 steps ...", '3');
	glutAddMenuEntry(" : Reset", ' ');
	glutAddMenuEntry("g: Draw ground", 'g');
	glutAddMenuEntry("t: Show Text", 't');
	glutAddMenuEntry("s: Toggle antialiasing", 's');
	glutAddMenuEntry("p: Use GL_POINTS", 'p');
	glutAddMenuEntry("l: Use GL_LINES", 'l');
	glutAddMenuEntry("c: Toggle camera motion", 'c');
	glutAddMenuEntry("+: 50 more particles", '+');
	glutAddMenuEntry("-: 50 less particles", '-');
	glutAddMenuEntry("x: Freeze Particles", 'x');
	
	glutAddMenuEntry("<esc> or q: exit program", '\033');
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glLineWidth(1.0);
	glPointSize(4.0);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	
	// Make a particle group
	int particle_handle = pGenParticleGroups(1, maxParticles);
	
	pCurrentGroup(particle_handle);
	
	BoidPrep();
	
	glutMainLoop();
	
	return 0;
}