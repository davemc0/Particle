// PSpray.cpp
//
// Copyright 1998 by David K. McAllister
//
// This application demonstrates particle systems for interactive graphics
// using OpenGL and GLUT.

#include <Util/Timer.h>
#include <Util/Utils.h>

#include <particle/papi.h>

// This just lets us pass vectors into the DrawGroupTex function
// and take their cross product.
#include <particle/p_vector.h>
#include <Math/Vector.h>

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

#ifdef WIN32
#pragma warning (disable:4305) /* disable bogus conversion warnings */
#define drand48() (((float) rand())/((float) RAND_MAX))
#define lrand48() ((rand() << 16) ^ rand())
#define srand48(x) srand(x)
#endif

#ifndef GL_ASSERT
#define GL_ASSERT() {GLenum sci_err; while ((sci_err = glGetError()) != GL_NO_ERROR) \
			cerr << "OpenGL error: " << (char *)gluErrorString(sci_err) << " at " << __FILE__ <<":" << __LINE__ << endl;}
#endif

static Timer Clock;

static bool doubleBuffer = true, MotionBlur = false, FreezeParticles = false, AntiAlias = true;
static bool drawGround = false, DoMotion = false, FullScreen = false;
static bool ShowText = true, ConstColor = false, DoDraw = true, DepthTest = true;

static int particle_handle, action_handle=-1, maxParticles = 20000;
static int demoNum = 10, numSteps = 1, prim = GL_LINES, listID = -1, SpotTexID = -1;
static float BlurRate = 0.09;
static int SteerShape = PDSphere;

// Symmetric gaussian centered at origin.
// No covariance matrix. Give it X and Y.
inline float Gaussian2(float x, float y, float sigma)
{
// The sqrt of 2 pi.
#define SQRT2PI 2.506628274631000502415765284811045253006
	return exp(-0.5 * (x*x + y*y) / (sigma*sigma)) / (SQRT2PI * sigma);
}

void MakeGaussianSpotTexture()
{
#define DIM 32
#define DIM2 (DIM>>1)
#define TEX_SCALE 7.0

	glGenTextures(1, (GLuint *)&SpotTexID);
	glBindTexture(GL_TEXTURE_2D, SpotTexID);

	float *img = new float[DIM*DIM];

	for(int y=0; y<DIM; y++)
	{
		for(int x=0; x<DIM; x++)
		{
			if(x==0 || x==DIM-1 || y==0 || y==DIM-1)
				img[y*DIM+x] = 0;
			else
			{
				img[y*DIM+x] = TEX_SCALE * Gaussian2(x-DIM2, y-DIM2, (DIM*0.15));
			}
		}
	}
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	float col[4] = {1.f, 1.f, 1.f, 1.f};
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	GL_ASSERT();

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA16, DIM, DIM, GL_ALPHA, GL_FLOAT, img);
}

// Draw each particle as a screen-aligned triangle with texture.
// Doesn't make the texture current. Just emits texcoords, if specified.
// If size_scale is 1 and const_size is true then the textured square
// will be 2x2 in world space (making the triangle sides be 4x4).
// view and up must be normalized and unequal.

// Draw as a splat texture on a quad.
void DrawGroupTriSplat(const pVector &view, const pVector &up,
					float size_scale = 1.0f, bool draw_tex=false,
					bool const_size=false, bool const_color=false)
{
	int cnt = pGetGroupCount();
	
	if(cnt < 1)
		return;
	
	pVector *ppos = new pVector[cnt];
	float *color = const_color ? NULL : new float[cnt * 4];
	pVector *size = const_size ? NULL : new pVector[cnt];
	
	pGetParticles(0, cnt, (float *)ppos, color, NULL, (float *)size);
	
	// Compute the vectors from the particle to the corners of its tri.
	// 2
	// |\ The particle is at the center of the x.
	// |-\ V0, V1, and V2 go from there to the vertices.
	// |x|\ The texcoords are (0,0), (2,0), and (0,2) respectively.
	// 0-+-1 We clamp the texture so the rest is transparent.
	
	pVector right = view ^ up;
	right.normalize();
	pVector nup = right ^ view;
	right *= size_scale;
	nup *= size_scale;

	pVector V0 = -(right + nup);
	pVector V1 = V0 + right * 4;
	pVector V2 = V0 + nup * 4;

	//cerr << "x " << view.x << " " << view.y << " " << view.z << endl;
	//cerr << "x " << nup.x << " " << nup.y << " " << nup.z << endl;
	//cerr << "x " << right.x << " " << right.y << " " << right.z << endl;
	//cerr << "x " << V0.x << " " << V0.y << " " << V0.z << endl;

	glBegin(GL_TRIANGLES);
	
	for(int i = 0; i < cnt; i++)
	{
		pVector &p = ppos[i];
		//cerr << p.x << " " << p.y << " " << p.z << endl;
		// cerr << color[i*4+3] << endl;
		
		if(!const_color)
			glColor4fv((GLfloat *)&color[i*4]);

		pVector sV0 = V0;
		pVector sV1 = V1;
		pVector sV2 = V2;

		if(!const_size)
		 {
		 sV0 *= size[i].x;
		 sV1 *= size[i].x;
		 sV2 *= size[i].x;
		 }

		if(draw_tex) glTexCoord2f(0,0);
		pVector ver = p + sV0;
		glVertex3fv((GLfloat *)&ver);

		if(draw_tex) glTexCoord2f(2,0);
		ver = p + sV1;
		glVertex3fv((GLfloat *)&ver);

		if(draw_tex) glTexCoord2f(0,2);
		ver = p + sV2;
		glVertex3fv((GLfloat *)&ver);
	}
	
	glEnd();

	delete [] ppos;
	if(color) delete [] color;
	if(size) delete [] size;
}

// Draw as a splat texture on a quad.
void DrawGroupQuadSplat(const pVector &view, const pVector &up,
					float size_scale = 1.0f, bool draw_tex=false,
					bool const_size=false, bool const_color=false)
{
	int cnt = pGetGroupCount();
	
	if(cnt < 1)
		return;
	
	pVector *ppos = new pVector[cnt];
	float *color = const_color ? NULL : new float[cnt * 4];
	pVector *size = const_size ? NULL : new pVector[cnt];
	
	pGetParticles(0, cnt, (float *)ppos, color, NULL, (float *)size);
	
	// Compute the vectors from the particle to the corners of its quad.
	// The particle is at the center of the x.
	// 3-2 V0, V1, V2 and V3 go from there to the vertices.
	// |x| The texcoords are (0,0), (1,0), (1,1), and (0,1) respectively.
	// 0-1 We clamp the texture so the rest is transparent.
	
	pVector right = view ^ up;
	right.normalize();
	pVector nup = right ^ view;
	right *= size_scale;
	nup *= size_scale;

	pVector V0 = -(right + nup);
	pVector V1 = right - nup;
	pVector V2 = right + nup;
	pVector V3 = nup - right;

	//cerr << "x " << view.x << " " << view.y << " " << view.z << endl;
	//cerr << "x " << nup.x << " " << nup.y << " " << nup.z << endl;
	//cerr << "x " << right.x << " " << right.y << " " << right.z << endl;
	//cerr << "x " << V0.x << " " << V0.y << " " << V0.z << endl;

	glBegin(GL_QUADS);
	
	for(int i = 0; i < cnt; i++)
	{
		pVector &p = ppos[i];
		//cerr << p.x << " " << p.y << " " << p.z << endl;
		// cerr << color[i*4+3] << endl;
		
		if(!const_color)
			glColor4fv((GLfloat *)&color[i*4]);

		pVector sV0 = V0;
		pVector sV1 = V1;
		pVector sV2 = V2;
		pVector sV3 = V3;

		if(!const_size)
		 {
		 sV0 *= size[i].x;
		 sV1 *= size[i].x;
		 sV2 *= size[i].x;
		 sV3 *= size[i].x;
		 }

		if(draw_tex) glTexCoord2f(0,0);
		pVector ver = p + sV0;
		glVertex3fv((GLfloat *)&ver);

		if(draw_tex) glTexCoord2f(1,0);
		ver = p + sV1;
		glVertex3fv((GLfloat *)&ver);

		if(draw_tex) glTexCoord2f(1,1);
		ver = p + sV2;
		glVertex3fv((GLfloat *)&ver);

		if(draw_tex) glTexCoord2f(0,1);
		ver = p + sV3;
		glVertex3fv((GLfloat *)&ver);
	}
	
	glEnd();

	delete [] ppos;
	if(color) delete [] color;
	if(size) delete [] size;
}

// Flames
void Fountain(bool first_time = true)
{
  pVelocity(0,0,0);
  pStartingAge(0);
  pSource(100000, PDCylinder, 5,0,-10,5,0,10,1);

}

// XXX New A waterfall pouring down from above
void Waterfall(bool first_time = true)
{
	pVelocityD(PDBlob, 0.1, 0, 0.1, 0.004);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);
	pStartingAge(0);
	
	if(first_time)
	{
		action_handle = pGenActionLists(1);
		pNewActionList(action_handle);
	}

	pCopyVertexB(false, true);

	pSource(50, PDPoint, -4, 0, 6);
	
	pGravity(0.0, 0.0, -0.01);
	
	pKillOld(250);
	
	pBounce(0, 0.01, 0, PDSphere, -1, 0, 4, 1);
	
	pBounce(0, 0.01, 0, PDSphere, -2.5, 0, 2, 1);

	pBounce(0, 0.01, 0, PDSphere, 0.7, -0.5, 2, 1);

	pBounce(-0.01, 0.35, 0, PDPlane, 0,0,0, 0,0,1);
	
	pMove();
	
	if(first_time)
		pEndActionList();
}

// Balloons particles to their positionB,
// which is usually set by Shape().
// XXX Balloons
// Make the balloon effect
void Balloons(bool first_time = true)
{
	float x=0, y=0, z=-1;
	float qty = maxParticles / 6.0;	// Because of 6 colors
	
	float BBOX = 0.5;
	
	pStartingAge(0, 5);
	pVelocity(0,0,0);
	pColor(1,0,0);
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
	pKillOld(400);
	
	pMove();
	pStartingAge(0);
}

// Make a bunch of particles in a particular shape.
void Shape(bool first_time = true)
{
	if(first_time)
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
					pVelocityD(PDBlob, 0, 0, 0, 0.01);
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
void Atom(bool first_time = true)
{
	if(first_time && action_handle<0)
		action_handle = pGenActionLists(1);

	pVelocityD(PDBlob, 0.02, -0.2, 0, 0.015);
	pSize(1.0);
	
	if(first_time)
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
	
	pColorD(1, PDSphere, 0.4+fabs(jetx*0.1), 0.4+fabs(jety*0.1), 0.4+fabs(jetz*0.1), 0.1);
	pSource(maxParticles / LifeTime, PDPoint, jetx, jety, jetz);
	
	pOrbitPoint(2, 0, 3, 0.1, 0.1);
	
	pOrbitPoint(-2, 0, -3, 0.1, 0.1);
	
	pDamping(0.994, 0.994, 0.994, 0.2);
	
	//pSink(false, PDSphere, 0, 0, 0, 15);
	
	pMove();
	
	if(first_time)
		pEndActionList();
}

void JetSpray(bool first_time = true)
{
	static Vector Jet;
	static Vector dJet;
	static bool FirstTime = true;
	
	if(FirstTime)
	{
		FirstTime = false;
		double Ang = drand48() * 2.0 * M_PI;
		dJet = Vector(cos(Ang), sin(Ang), 0);
		dJet *= 0.1;
	}
	Jet += dJet;
	if(Jet.x > 10 || Jet.x < -10) {dJet.x = -dJet.x; dJet.y += drand48() * 0.005;}
	if(Jet.y > 10 || Jet.y < -10) {dJet.y = -dJet.y; dJet.x += drand48() * 0.005;}
	
	pColorD(1.0, PDLine, 1,0,0, 1,1,0.3);
	
	pVelocityD(PDBlob, 0,0,0.1, 0.01);
	pStartingAge(0);
	pSize(1);
	
	int LifeTime = 100;
	
	pKillOld(LifeTime);
	
	pSource(maxParticles / LifeTime, PDDisc, Jet.x, Jet.y, Jet.z, 0, 0, 1, 0.5);
	
	pGravity(.000, .01, .000);
	
	pDamping(0.9, 0.97, 0.9);
	
	float BOX = .017;
	
	pRandomAccel(PDBox, -BOX, -BOX, -BOX, BOX, BOX, BOX);
	
	pMove();
}

void Rocket(bool first_time = true)
{
#define MAX_ROCKETS 15
	static int RocketSystem = -1;

	if(RocketSystem < 0)
		RocketSystem = pGenParticleGroups(1, MAX_ROCKETS);

	// Move the rockets.
	pCurrentGroup(RocketSystem);
	pVelocityD(PDCylinder, 0,0,0.15, 0,0,0.2, 0.11, 0.07);
	pColorD(1, PDBox, 0,0.5,0, 1,1,1);

	pSource(0.02, PDDisc, 0,0,0, 0,0,1, 6);
	pSink(false, PDPlane, 0,0,-1, 0,0,1);
	pGravity(0,0,-0.003);
	pMove();

	// Read back the position of the rockets.
	float rocketp[MAX_ROCKETS][3], rocketc[MAX_ROCKETS][3], rocketv[MAX_ROCKETS][3];
	int rcount = pGetParticles(0, MAX_ROCKETS, (float *)rocketp, (float *)rocketc, (float *)rocketv);

	pCurrentGroup(particle_handle);
	
	pSize(1.0);
	pStartingAge(0, 6);
	
	for(int i=0; i<rcount; i++)
	{
		Vector rv(rocketv[i][0], rocketv[i][1], rocketv[i][2]);
		rv.normalize();
		rv *= -0.026;
		//cerr << i << " " <<rocketp[i][0]<<" "<<rocketp[i][1]<<" "<<rocketp[i][2]<<"\n";
		//cerr << "c " <<rocketc[i][0]<<" "<<rocketc[i][1]<<" "<<rocketc[i][2]<<"\n";

		pColorD(1.0, PDLine, rocketc[i][0], rocketc[i][1], rocketc[i][2], 1,.5,.5);
		pVelocityD(PDBlob, rv.x, rv.y, rv.z, 0.006);		
		pSource(30, PDPoint, rocketp[i][0], rocketp[i][1], rocketp[i][2]);
	}
	
	pGravity(0,0,-0.001);
	//pDamping(0.999, 0.999, 0.999);
	pTargetColor(0,0,0,0, 0.02);
	pKillOld(90);

	pMove();
}

// Boids
void Boids(bool first_time = true)
{
	pStartingAge(0);
	pMove();
}

void Explosion(bool first_time = true)
{
	pVelocityD(PDSphere, 0,0,0,0.01,0.01);
	pColorD(1.0, PDSphere, 0.5, 0.7, 0.5, .3);
	pSize(1.0);
	pStartingAge(0);
	
	pCopyVertexB(false, true);
	
	pDamping(0.999, 0.999, 0.999, 0.2);
	
	static float i=0;

	if(first_time) i=0;

	pOrbitPoint(0, 0, 0, .02, 0.1);
	
	pExplosion(0, 0, 0, 1, 8, 3, 0.1, i += (1.0f / float(numSteps)));
	
	pSink(false, PDSphere, 0, 0, 0, 30);
	
	pMove();
}

void Swirl(bool first_time = true)
{
	pVelocityD(PDBlob, 0.02, -0.2, 0, 0.015);
	pSize(1.0);
	pStartingAge(0);
	
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
	
	pOrbitLine(0, 0, 1, 1, 0, 0, 0.1, 0.1);
	
	pDamping(1, 0.994, 0.994);
	
	pSink(false, PDSphere, 0, 0, 0, 15);
	
	pMove();
}

void Shower(bool first_time = true)
{
	static float jetx=0, jety=0, jetz=0;
	static float djx, djy;
	
	if(first_time)
	{
		jetx = 1;
		jety = 1;
		djx = drand48() * 0.1;
		djy = drand48() * 0.1;
	} 
	
	jetx += djx;
	jety += djy;
	
	if(jetx > 1 || jetx < 0) {djx = -djx; djy += drand48() * 0.0005;}
	if(jety > 2 || jety < -2) {djy = -djy; djx += drand48() * 0.0005;}

	pVelocity(0, 0, 0);
	pSize(1.5);
	pStartingAge(0);

	pColor(jetx, jety, 1);

	pSource(120, PDLine, -5,jety,8, 5,jety,8);
	
	pGravity(0.0, 0.0, -0.004);

	glColor3f(1,1,0);
	if(SteerShape == PDSphere)
	{
		pAvoid(0.2, 1.0, 20, PDSphere, 0,0,0, 1.1);

	    glutSolidSphere(1, 16, 8);
	}
	else if(SteerShape == PDTriangle)
	{
		pAvoid(2, 1.0, 20, PDTriangle, 0,-1,0, 2,0,0, 0,2,0);

		glBegin(GL_TRIANGLES);
		glVertex3f(0,-1,0);
		glVertex3f(2,0,0);
		glVertex3f(0,2,0);
		glEnd();
	}
	else if(SteerShape == PDRectangle)
	{
		pAvoid(2, 1.0, 20, PDRectangle, 0,-1,0, 2,1,0, 0,2,0);

		glBegin(GL_QUADS);
		glVertex3f(0,-1,0);
		glVertex3f(2,0,0);
		glVertex3f(2,2,0);
		glVertex3f(0,1,0);
		glEnd();
	}
	else if(SteerShape == PDPlane)
	{
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
}

// A fountain spraying up in the middle of the screen
void FireFlies(bool first_time = true)
{
	pSize(1.0);
	pVelocityD(PDPoint, 0,0,0);
	pColorD(1.0, PDSphere, .5, .4, .1, .1);
	pStartingAge(0);
	
	pCopyVertexB(false, true);

	pSource(1, PDBlob, 0, 0, 2, 2);
	
	pRandomAccel(PDSphere, 0, 0, 0, 0.02);
	
	pKillOld(20);
	
	pMove();
}

void CallDemo(bool initial)
{

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
		Balloons(initial);
		break;
	case 8:
		Shape(initial);
		break;
	case 9:
		Shower(initial);
		break;
	case 10:
		Boids(initial);
		break;
	case 11:
		Rocket(initial);
		break;
	default:
		cerr << "Bad demo number!\n";
		break;
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

void Draw()
{
#define NUM_FRAMES 30
	static double eltime = 1.0;
	static int NumFrames = 0;

	glLoadIdentity();
	
	if(SpotTexID < 0)
		MakeGaussianSpotTexture();

	if(MotionBlur)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		
		glDrawBuffer(GL_FRONT);
		
		// glEnable(GL_BLEND);
		
		glColor4f(0, 0, 0, BlurRate);
		glRectf(-1, -1, 1, 1);
		if(DepthTest)
		  glClear(GL_DEPTH_BUFFER_BIT);

		// glDisable(GL_BLEND);
		
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT | (DepthTest ? GL_DEPTH_BUFFER_BIT:0));
	}
	
	static float t = -23.0;
	static float dt = -0.05;
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
	GL_ASSERT();
	
	glColor3f(1.0, 1.0, 1.0);

	if(DoDraw)
	{
		if(prim < 0)
			pDrawGroupl(listID, true, ConstColor);
		else if(prim < 0x100)
		{
			glDisable(GL_TEXTURE_2D);
			pDrawGroupp(prim, true, ConstColor);
		}
		else
		{
			pVector view = pVector(0, 0, 0) - pVector(0, t, 10);
			view.normalize();
			pVector up(0, 0, 1);
			if(prim == 0x100)
				DrawGroupTriSplat(view, up, 0.16, true, true, ConstColor);
			else
				DrawGroupQuadSplat(view, up, 0.16, true, true, ConstColor);
		}
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
		int cnt = pGetGroupCount();
		sprintf(msg, "%d particles drawn", cnt);
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
	glClear(GL_COLOR_BUFFER_BIT | (DepthTest ? GL_DEPTH_BUFFER_BIT:0));
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
	case 't':
		ShowText = !ShowText;
		break;
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
	case 'M':
		demoNum = 9;
		SteerShape = PDPlane;
		CallDemo(true);
		break;
	case 'N':
		demoNum = 9;
		SteerShape = PDSphere;
		CallDemo(true);
		break;
	case 'B':
		demoNum = 9;
		SteerShape = PDTriangle;
		CallDemo(true);
		break;
	case 'V':
		demoNum = 9;
		SteerShape = PDRectangle;
		CallDemo(true);
		break;
	case 'C':
		demoNum = 10;
		CallDemo(true);
		break;
	case 'H':
		demoNum = 11;
		CallDemo(true);
		break;
	case 'v':
		ConstColor = !ConstColor;
		break;
	case 'm':
		MotionBlur = !MotionBlur;
		if(!MotionBlur)
			glDrawBuffer(GL_BACK);
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
	case 'c':
		DoMotion = !DoMotion;
		break;
	case 'b':
		DoDraw = !DoDraw;
		break;
	case 'd':
		prim = -1;
		glDisable(GL_TEXTURE_2D);

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
	case 'o':
		prim = 0x0100;
		glEnable(GL_TEXTURE_2D);
		break;
	case 'k':
		prim = 0x0101;
		glEnable(GL_TEXTURE_2D);
		break;
	case 'g':
		drawGround = !drawGround;
		break;
	case 'e':
		DepthTest = !DepthTest;
		if(DepthTest)
		  glEnable(GL_DEPTH_TEST);
		else
		  glDisable(GL_DEPTH_TEST);
		break;
	case 'z':
		pSinkVelocity(true, PDSphere, 0, 0, 0, 0.01);
		break;
	case 'x':
		FreezeParticles = !FreezeParticles;
		break;
	case '+':
		maxParticles += 500;
		pSetMaxParticles(maxParticles);
		cerr << maxParticles << endl;
		break;
	case '-':
		maxParticles -= 500;
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

	do {
	 demoNum = lrand48() % 12;
	} while(demoNum == 3 || demoNum == 7);

	glutInit(&argc, argv);
	Args(argc, argv);
	
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
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
	glutAddMenuEntry("C: Boids", 'C');
	glutAddMenuEntry("R: Balloons", 'R');
	glutAddMenuEntry("N: Shower Sphere", 'N');
	glutAddMenuEntry("M: Shower Plane", 'M');
	glutAddMenuEntry("B: Shower Triangle", 'B');
	glutAddMenuEntry("V: Shower Rectangle", 'V');
	glutAddMenuEntry("H: Rocket", 'H');
	glutAddMenuEntry(" : Explosion", ' ');
	glutAddMenuEntry("g: Draw ground", 'g');
	glutAddMenuEntry("t: Show Text", 't');
	glutAddMenuEntry("s: Toggle antialiasing", 's');
	glutAddMenuEntry("p: Use GL_POINTS", 'p');
	glutAddMenuEntry("l: Use GL_LINES", 'l');
	glutAddMenuEntry("b: Toggle drawing", 'd');
	glutAddMenuEntry("e: Toggle depth test", 'e');
	glutAddMenuEntry("o: Use textured quad", 'o');
	glutAddMenuEntry("k: Use textured tri", 'k');
	glutAddMenuEntry("m: Toggle motion blur", 'm');
	glutAddMenuEntry("c: Toggle camera motion", 'c');
	glutAddMenuEntry("+: 100 more particles", '+');
	glutAddMenuEntry("-: 100 less particles", '-');
	glutAddMenuEntry(">: More motion blur", '>');
	glutAddMenuEntry("<: Less motion blur", '<');
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
	particle_handle = pGenParticleGroups(1, maxParticles);
	
	pCurrentGroup(particle_handle);
	
	CallDemo(true);
	
	glutMainLoop();
	
	return 0;
}
