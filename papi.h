// papi.h
//
// Copyright 1997-1998 by David K. McAllister
// http://www.cs.unc.edu/~davemc/Particle
//
// Include this file in all applications that use the Particle System API.


#ifndef _particle_api_h
#define _particle_api_h

#include <stdlib.h>

// This is the major and minor version number of this release of the API.
#define P_VERSION 111

// Actually this must be < sqrt(MAXFLOAT) since we store this value squared.
#define P_MAXFLOAT 1.0e16f

#ifdef MAXINT
#define P_MAXINT MAXINT
#else
#define P_MAXINT 0x7fffffff
#endif

#define P_EPS 1e-3f

//////////////////////////////////////////////////////////////////////
// Type codes for domains
enum PDomainEnum
{
	PDPoint = 0, // Single point
	PDLine = 1, // Line segment
	PDTriangle = 2, // Triangle
	PDPlane = 3, // Arbitrarily-oriented plane
	PDBox = 4, // Axis-aligned box
	PDSphere = 5, // Sphere
	PDCylinder = 6, // Cylinder
	PDCone = 7, // Cone
	PDBlob = 8 // Gaussian blob
};

// State setting calls

void pColor(float red, float green, float blue, float alpha = 1.0f);

void pColorD(float alpha, PDomainEnum dtype,
			 float a0=0.0f, float a1=0.0f, float a2=0.0f,
			 float a3=0.0f, float a4=0.0f, float a5=0.0f,
			 float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pSize(float s1, float s2 = -1.0);

void pStartingAge(float age);

void pTimeStep(float newDT);

void pVelocity(float x, float y, float z);

void pVelocityD(PDomainEnum dtype,
				float a0=0.0f, float a1=0.0f, float a2=0.0f,
				float a3=0.0f, float a4=0.0f, float a5=0.0f,
				float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pVertexB(float x, float y, float z);

void pVertexBD(PDomainEnum dtype,
			   float a0=0.0f, float a1=0.0f, float a2=0.0f,
			   float a3=0.0f, float a4=0.0f, float a5=0.0f,
			   float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pVertexBTracks(bool trackVertex = true);


// Action List Calls

void pCallActionList(int action_list_num);

void pDeleteActionLists(int action_list_num, int action_list_count = 1);

void pEndActionList();

int pGenActionLists(int action_list_count = 1);

void pNewActionList(int action_list_num);


// Particle Group Calls

void pCopyGroup(int p_group_num, int index = 0, int copy_count = P_MAXINT);

void pCurrentGroup(int p_group_num);

void pDeleteParticleGroups(int p_group_num, int p_group_count = 1);

void pDrawGroupl(int dlist, bool const_size = false,
				 bool const_color = false, bool const_rotation = false);

void pDrawGroupp(int primitive, bool const_size = false,
				 bool const_color = false);

int pGenParticleGroups(int p_group_count = 1, int max_particles = 0);

int pGetGroupCount();

void pGetParticles(int index, int count, float *verts, float *color = NULL,
				   float *vel = NULL, float *size = NULL);

int pSetMaxParticles(int max_count);


// Actions

void pBounce(float friction, float resilience, float cutoff,
			 PDomainEnum dtype,
			 float a0=0.0f, float a1=0.0f, float a2=0.0f,
			 float a3=0.0f, float a4=0.0f, float a5=0.0f,
			 float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pCopyVertexB(bool copy_pos = true, bool copy_vel = false);

void pDamping(float damping_x, float damping_y, float damping_z,
			  float vlow = 0.0f, float vhigh = P_MAXFLOAT);

void pExplosion(float center_x, float center_y, float center_z,
				float velocity, float magnitude, float lifetime,
				float epsilon = P_EPS, float age = 0.0f);

void pFollow(float grav = 1.0f, float epsilon = P_EPS);

void pGravitate(float grav = 1.0f, float epsilon = P_EPS);

void pGravity(float dir_x, float dir_y, float dir_z);

void pGrowSize(float destSize, float scale);

void pJet(float center_x, float center_y, float center_z, float grav = 1.0f,
		  float epsilon = P_EPS, float maxRadius = P_MAXFLOAT);

void pKillOld(float ageLimit, bool kill_less_than = false);

void pKillSlow(float speedLimit, bool kill_less_than = true);

void pMove();

void pOrbitLine(float p_x, float p_y, float p_z,
				float axis_x, float axis_y, float axis_z, float grav = 1.0f,
				float epsilon = P_EPS, float maxRadius = P_MAXFLOAT);

void pOrbitPoint(float center_x, float center_y, float center_z,
				 float grav = 1.0f, float epsilon = P_EPS,
				 float maxRadius = P_MAXFLOAT);

void pRandomAccel(PDomainEnum dtype,
				  float a0=0.0f, float a1=0.0f, float a2=0.0f,
				  float a3=0.0f, float a4=0.0f, float a5=0.0f,
				  float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pRandomDisplace(PDomainEnum dtype,
					 float a0=0.0f, float a1=0.0f, float a2=0.0f,
					 float a3=0.0f, float a4=0.0f, float a5=0.0f,
					 float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pRandomVelocity(PDomainEnum dtype,
					 float a0=0.0f, float a1=0.0f, float a2=0.0f,
					 float a3=0.0f, float a4=0.0f, float a5=0.0f,
					 float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pRestore(float time);

void pShade(float color_x, float color_y, float color_z,
			float alpha, float scale);

void pSink(bool kill_inside, PDomainEnum dtype,
		   float a0=0.0f, float a1=0.0f, float a2=0.0f,
		   float a3=0.0f, float a4=0.0f, float a5=0.0f,
		   float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pSinkVelocity(bool kill_inside, PDomainEnum dtype,
				   float a0=0.0f, float a1=0.0f, float a2=0.0f,
				   float a3=0.0f, float a4=0.0f, float a5=0.0f,
				   float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pSource(float particleRate, PDomainEnum dtype,
			 float a0=0.0f, float a1=0.0f, float a2=0.0f,
			 float a3=0.0f, float a4=0.0f, float a5=0.0f,
			 float a6=0.0f, float a7=0.0f, float a8=0.0f);

void pTargetColor(float color_x, float color_y, float color_z,
				  float alpha, float scale);

void pTargetSize(float destSize, float scale);

void pVertex(float x, float y, float z);

void pVortex(float center_x, float center_y, float center_z,
			 float axis_x, float axis_y, float axis_z,
			 float magnitude, float tightness=1.0f, float maxRadius = P_MAXFLOAT);

#endif
