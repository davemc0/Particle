// MeltPhoto.cpp
//
// Copyright 1999 by David K. McAllister
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
#include <Image/Image.h>

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

// #define DEPTH_TEST

#ifndef GL_ASSERT
// This makes it easy to check for OpenGL errors.
#define GL_ASSERT() {GLenum sci_err; while ((sci_err = glGetError()) != GL_NO_ERROR) \
			cerr << "OpenGL error: " << (char *)gluErrorString(sci_err) << " at " << __FILE__ <<":" << __LINE__ << endl;}
#endif

static Timer Clock;
Image *Im;

static bool doubleBuffer = true, MotionBlur = false, FreezeParticles = false, AntiAlias = true;
static bool immediate = true, drawGround = false, DoMotion = false, FullScreen = false;
static bool ShowText = true, ConstColor = false, DoDraw = true;

static int particle_handle, action_handle=-1, maxParticles = 20000;
static int demoNum = 10, numSteps = 1, prim = GL_POINTS, listID = -1, SpotTexID = -1;
static float BlurRate = 0.09, RestoreTime = 200.0;
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

void BounceBox(bool first_time = true)
{
	pBounce(0, 1.02, 0, PDPlane, 4,0,0, 1,0,0);
	pBounce(0, 1.02, 0, PDPlane, -4,0,0, 1,0,0);
	pBounce(0, 1.02, 0, PDPlane, 0,1,0, 0,1,0);
	pBounce(0, 1.02, 0, PDPlane, 0,-4,0, 0,1,0);
	pBounce(0, 1.02, 0, PDPlane, 0,0,4, 0,0,1);
	pBounce(0, 1.02, 0, PDPlane, 0,0,0, 0,0,1);

	pMove();
}

// XXX New A waterfall pouring down from above
void Waterfall(bool first_time = true)
{
	pGravity(0.0, 0.0, -0.01);
	
	pBounce(0, 0.01, 0, PDSphere, -1, 0, 4, 1);
	
	pBounce(0, 0.01, 0, PDSphere, -2.5, 0, 2, 1);

	pBounce(0, 0.01, 0, PDSphere, 0.7, -0.5, 2, 1);

	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);
	
	pMove();
}

// Restore particles to their positionB,
// which is usually set by Shape().
void Restore(bool first_time = true)
{
	if(RestoreTime < -2)
	  return;

	// pCopyVertexB(false, true);

	pRestore(RestoreTime -= (1.0f / float(numSteps)));

	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);
	
	pMove();
}

// Make a bunch of particles in a particular shape.
void Shape(bool first_time = true)
{
	pKillOld(-1000000000);
	pVelocity(0,0,0);
	pSize(1);
	pStartingAge(-10000);
#define ST 0.03
	
	unsigned char *Pix = Im->Pix;
	int i=0;
	float sc = 1./255.;

	for(int y=0; y<Im->hgt; y++)
	{
		for(int x=0; x<Im->wid; x++)
		{
			pColor(Pix[i]*sc, Pix[i+1]*sc, Pix[i+2]*sc);
			i+=3;
			pVertex((x-Im->wid/2)*ST, 1, (y)*ST+0.1);			
		}
	}
}

// A BounceBox spraying up in the middle of the screen
void Chaos(bool first_time = true)
{
	pOrbitPoint(2, 0, 3, 0.1, 0.1);
	
	pOrbitPoint(-2, 1, 2, 0.1, 0.1);
	
	// pDamping(0.994, 0.994, 0.994);
	
	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);

	pMove();
}

void Disperse(bool first_time = true)
{	
	pDamping(0.9, 0.97, 0.97);
	
	float BOX = .017;
	
	pRandomAccel(PDBox, -BOX, -BOX, -BOX, BOX, BOX, BOX);
	
	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);
	
	pMove();
}

void Jet(bool first_time = true)
{
	pVelocityD(PDDisc, 0.02,0,0, 1,0,0, 0.004);

	pJet(-1.7,0,3, 0.1, 0.1);
	
	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);
	
	pMove();
}

// Vortex
void Vortex(bool first_time = true)
{
	static float jetx=-4, jety=0, jetz=-2.4;
	
	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;
	
	jetx += djx;
	jety += djy;
	jetz += djz;
	
	if(jetx > 4 || jetx < -4) djx = -djx;
	if(jety > 4 || jety < -4) djy = -djy;
	if(jetz > 10 || jetz < -10) djz = -djz;

	pVortex(jetx,jety,1, 0,0,1, 1);
	
	pMove();
}

void Explosion(bool first_time = true)
{	
	pDamping(0.999, 0.999, 0.999);
	
	static float i=0;

	if(first_time) i=0;

	pOrbitPoint(0, 0, 0, .02, 0.1);
	
	pExplosion(0, 0, 0, 1, 8, 3, 0.1, i += (1.0f / float(numSteps)));
	
	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);
	
	pMove();
}

void Swirl(bool first_time = true)
{	
	static float jetx=-4, jety=0, jetz=-2.4;
	
	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;
	
	jetx += djx;
	jety += djy;
	jetz += djz;
	
	if(jetx > 4 || jetx < -4) djx = -djx;
	if(jety > 4 || jety < -4) djy = -djy;
	if(jetz > 10 || jetz < -10) djz = -djz;
		
	pOrbitLine(jetx,jety,0, 0,0,1, 0.1, 0.1);
	
	pDamping(1, 0.994, 0.994);
	
	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);
	
	pMove();
}

void Shower(bool first_time = true)
{
	static float jetx=1, jety=1, jetz=1;
	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;
	
	jetx += djx;
	jety += djy;
	jetz += djz;
	
	if(jetx > -5 || jetx < 5) {djx = -djx; djy += drand48() * 0.0005;}
	if(jety > 5 || jety < -5) {djy = -djy; djx += drand48() * 0.0005;}
	if(jetz > 5 || jetz < 0) {djz = -djz; djz += drand48() * 0.0005;}
	
	pGravity(0.0, 0.0, -0.004);

	if(SteerShape == PDSphere)
	{
		pAvoid(0.2, 1.0, 20, PDSphere, jetx,jety,jetz, 0.6);

		glColor3f(1,1,0);
		glutSolidSphere(0.5, 16, 8);
	}
	else if(SteerShape == PDTriangle)
	{
		pAvoid(2, 1.0, 20, PDTriangle, jetx,jety-1,jetz, jetx+2,jety,jetz, jetx,jety+2,jetz);

		glColor3f(1,1,0);
		glBegin(GL_TRIANGLES);
		glVertex3f(jetx,jety-1,jetz);
		glVertex3f(jetx+2,jety,jetz);
		glVertex3f(jetx,jety+2,jetz);
		glEnd();
	}
	else if(SteerShape == PDRectangle)
	{
		pAvoid(2, 1.0, 20, PDRectangle, jetx,jety-1,jetz, 2,1,0, 0,2,0);

		glColor3f(1,1,0);
		glBegin(GL_QUADS);
		glVertex3f(jetx,jety-1,jetz);
		glVertex3f(jetx+2,jety,jetz);
		glVertex3f(jetx+2,jety+2,jetz);
		glVertex3f(jetx,jety+1,jetz);
		glEnd();
	}
	else if(SteerShape == PDPlane)
	{
		pAvoid(2, 1.0, 20, PDPlane, jetx,jety,jetz, djx,djy,djz);

		glBegin(GL_QUADS);
		glVertex3f(-2,-2,0);
		glVertex3f(2,-2,0);
		glVertex3f(2,2,0);
		glVertex3f(-2,2,0);
		glEnd();
	}
	
	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);
	
	pMove();
}

void FadeColor(bool first_time = true)
{
	pTargetColor(0.5,0.6,0.9,1, 0.01);
	
	pBounce(0.05, 0.3, 0, PDPlane, 0,0,0, 0,0,1);

	pMove();
}

void CallDemo(bool initial)
{
	if(!immediate && !initial)
	{
		cerr << action_handle << endl;
		pCallActionList(action_handle);
		return;
	}

	if(action_handle >= 0 && !immediate)
		pDeleteActionLists(action_handle);

	switch(demoNum)
	{
	case 0:
		BounceBox(initial);
		break;
	case 1:
		Chaos(initial);
		break;
	case 2:
		Disperse(initial);
		break;
	case 3:
		Explosion(initial);
		break;
	case 4:
		Swirl(initial);
		break;
	case 5:
		FadeColor(initial);
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
		Shower(initial);
		break;
	case 10:
		Vortex(initial);
		break;
	case 11:
		Jet(initial);
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
	
	static float t = 0.0;
	static float dt = -0.05;
	if(DoMotion)
		t += dt;
	if(t < -15.0 || t > 15.0) dt = -dt;
	gluLookAt(t, -10, 2, 0, 0, 2, 0, 0, 1);
	
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
			pVector view = pVector(0, 0, 2) - pVector(t, -10, 2);
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
#ifdef DEPTH_TEST
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
		glClear(GL_COLOR_BUFFER_BIT);
#endif
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
		if(!immediate) CallDemo(true);
		break;
	case 'C':
		demoNum = 1;
		if(!immediate) CallDemo(true);
		break;
	case 'J':
		demoNum = 2;
		if(!immediate) CallDemo(true);
		break;
	case 'E':
	case ' ':
		demoNum = 3;
		CallDemo(true);
		break;
	case 'T':
		demoNum = 4;
		if(!immediate) CallDemo(true);
		break;
	case 'I':
		demoNum = 5;
		if(!immediate) CallDemo(true);
		break;
	case 'W':
		demoNum = 6;
		if(!immediate) CallDemo(true);
		break;
	case 'R':
		demoNum = 7;
		RestoreTime = 200.0;
		if(!immediate) CallDemo(true);
		break;
	case 'S':
		demoNum = 8;
		if(!immediate) CallDemo(true);
		break;
	case 'N':
		demoNum = 9;
		SteerShape = PDSphere;
		if(!immediate) CallDemo(true);
		break;
	case 'M':
		demoNum = 9;
		SteerShape = PDPlane;
		if(!immediate) CallDemo(true);
		break;
	case 'B':
		demoNum = 9;
		SteerShape = PDTriangle;
		if(!immediate) CallDemo(true);
		break;
	case 'V':
		demoNum = 9;
		SteerShape = PDRectangle;
		if(!immediate) CallDemo(true);
		break;
	case 'A':
		demoNum = 10;
		if(!immediate) CallDemo(true);
		break;
	case 'H':
		demoNum = 11;
		if(!immediate) CallDemo(true);
		break;
	case 'i':
		immediate = !immediate;
		cerr << "Switching to " << (immediate ? "immediate":"action list") << " mode.\n";
		if(!immediate) CallDemo(true);
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
	
	cerr << "Usage: " << program_name << " imagefilename\n";
	exit(1);
}

static void
Args(int argc, char **argv)
{
	char *program = argv[0];
	
	while (--argc)
	{
		argv++;
		
		if (!strcmp("-h", argv[0]) || !strcmp("-help", argv[0]))
			Usage(program, NULL);
		else if (!strcmp("-db", argv[0]) || !strcmp("-double", argv[0]))
			doubleBuffer = true;
		else if (!strcmp("-sb", argv[0]) || !strcmp("-single", argv[0]))
			doubleBuffer = false;
		else //if (!strcmp("-im", argv[0]) || !strcmp("-image", argv[0]))
		{
			Im = new Image(argv[0]);
			Im->VFlip();
			maxParticles = Im->size;
			//argv++; argc--;
		}
	}
	if(Im == NULL)
		Usage(program, "Invalid option!");
}

int main(int argc, char **argv)
{
	srand48( (unsigned)time( NULL ) );

	demoNum = 8;

	glutInit(&argc, argv);
	Args(argc, argv);
	
	int type = GLUT_RGB;
#ifdef DEPTH_TEST
	type |= GLUT_DEPTH;
#endif
	// type |= GLUT_MULTISAMPLE;
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

	glutAddMenuEntry(" : Explosion", ' ');
	glutAddMenuEntry("A: Vortex", 'A');
	glutAddMenuEntry("B: Avoid Triangle", 'B');
	glutAddMenuEntry("C: Chaos", 'C');
	glutAddMenuEntry("F: BounceBox", 'F');
	glutAddMenuEntry("H: Jet", 'H');
	glutAddMenuEntry("I: FadeColor", 'I');
	glutAddMenuEntry("J: Disperse", 'J');
	glutAddMenuEntry("M: Avoid Plane", 'M');
	glutAddMenuEntry("N: Avoid Sphere", 'N');
	glutAddMenuEntry("R: Restore", 'R');
	glutAddMenuEntry("S: Shape", 'S');
	glutAddMenuEntry("T: Twister", 'T');
	glutAddMenuEntry("V: Avoid Rectangle", 'V');
	glutAddMenuEntry("W: Waterfall", 'W');

	glutAddMenuEntry("g: Draw ground", 'g');
	glutAddMenuEntry("t: Show Text", 't');
	glutAddMenuEntry("s: Toggle antialiasing", 's');
	glutAddMenuEntry("p: Use GL_POINTS", 'p');
	glutAddMenuEntry("l: Use GL_LINES", 'l');
	glutAddMenuEntry("o: Use textured quad", 'o');
	glutAddMenuEntry("k: Use textured tri", 'k');
	glutAddMenuEntry("b: Toggle drawing", 'b');
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

	// glEnable(GL_LINE_SMOOTH);
	// glEnable(GL_POINT_SMOOTH);
	glLineWidth(1.0);
	glPointSize(3);
	// glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	// Make a particle group
	particle_handle = pGenParticleGroups(1, maxParticles);
	
	pCurrentGroup(particle_handle);
	
	if(!immediate) CallDemo(true);
	
	glutMainLoop();
	
	return 0;
}
