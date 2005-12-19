#pragma once

#include "papi.h"
#include "pVector.h"

//////////////////////////////////////////////////////////////////////
// Type codes for all actions
enum PActionEnum
{
	PAHeaderID,			// The first action in each list.
	PAAvoidID,			// Avoid entering the domain of space.
	PABounceID,			// Bounce particles off a domain of space.
	PACallActionListID,	// 
	PACopyVertexBID,	// Set the secondary position and velocity from current.
	PADampingID,		// Dampen particle velocities.
	PARotDampingID,		// Dampen particle velocities.
	PAExplosionID,		// An Explosion.
	PAFollowID,			// Accelerate toward the previous particle in the group.
	PAFountainID,		// EXPERIMENTAL: Precompiled fountain effect.
	PAGravitateID,		// Accelerate each particle toward each other particle.
	PAGravityID,		// Acceleration in the given direction.
	PAJetID,			// 
	PAKillOldID,		// 
	PAMatchVelocityID,	// 
	PAMatchRotVelocityID,	// 
	PAMoveID,			// 
	PAOrbitLineID,		// 
	PAOrbitPointID,		// 
	PARandomAccelID,	// 
	PARandomDisplaceID,	// 
	PARandomVelocityID,	// 
	PARandomRotVelocityID,	// 
	PARestoreID,		// 
	PASinkID,			// 
	PASinkVelocityID,	// 
	PASortID,			// 
	PASourceID,			// 
	PASpeedLimitID,		// 
	PATargetColorID,	// 
	PATargetSizeID,		// 
	PATargetVelocityID,	// 
	PATargetRotVelocityID,	// 
	PAVortexID			// 
};

class ParticleGroup;

// This method actually does the particle's action.
#define ExecMethod	void Execute(ParticleGroup *pg);

struct PABaseClass
{
	static float dt;	// This is copied to here from global state.
	PActionEnum type;	// Type field
};

///////////////////////////////////////////////////////////////////////////
// Data types derived from Action.

struct PAAvoid : public PABaseClass
{
	pDomain position;	// Avoid region
	float look_ahead;	// how many time units ahead to look
	float magnitude;	// what percent of the way to go each time
	float epsilon;		// add to r^2 for softening

	ExecMethod

	void ExecTriangle(ParticleGroup *group);
	void ExecDisc(ParticleGroup *group);
	void ExecPlane(ParticleGroup *group);
	void ExecRectangle(ParticleGroup *group);
	void ExecSphere(ParticleGroup *group);
};

struct PABounce : public PABaseClass
{
	pDomain position;	// Bounce region
	float oneMinusFriction;	// Friction tangent to surface
	float resilience;	// Resilence perpendicular to surface
	float cutoffSqr;	// cutoff velocity; friction applies iff v > cutoff

	ExecMethod

	void ExecTriangle(ParticleGroup *group);
	void ExecDisc(ParticleGroup *group);
	void ExecPlane(ParticleGroup *group);
	void ExecRectangle(ParticleGroup *group);
	void ExecSphere(ParticleGroup *group);
};

struct PACallActionList : public PABaseClass
{
	int action_list_num;	// The action list number to call

	ExecMethod
};

struct PACopyVertexB : public PABaseClass
{
	bool copy_pos;		// True to copy pos to posB.
	bool copy_vel;		// True to copy vel to velB.

	ExecMethod
};

struct PADamping : public PABaseClass
{
	pVector damping;	// Damping constant applied to velocity
	float vlowSqr;		// Low and high cutoff velocities
	float vhighSqr;

	ExecMethod
};

struct PARotDamping : public PABaseClass
{
	pVector damping;	// Damping constant applied to velocity
	float vlowSqr;		// Low and high cutoff velocities
	float vhighSqr;

	ExecMethod
};

struct PAExplosion : public PABaseClass
{
	pVector center;		// The center of the explosion
	float velocity;		// Of shock wave
	float magnitude;	// At unit radius
	float stdev;		// Sharpness or width of shock wave
	float age;			// How long it's been going on
	float epsilon;		// Softening parameter

	ExecMethod
};

struct PAFollow : public PABaseClass
{
	float magnitude;	// The grav of each particle
	float epsilon;		// Softening parameter
	float max_radius;	// Only influence particles within max_radius

	ExecMethod
};

struct PAHeader;
struct PAFountain : public PABaseClass
{
	PAHeader *AL; // A pointer to the data for all the actions.

	ExecMethod
};

struct PAGravitate : public PABaseClass
{
	float magnitude;	// The grav of each particle
	float epsilon;		// Softening parameter
	float max_radius;	// Only influence particles within max_radius

	ExecMethod
};

struct PAGravity : public PABaseClass
{
	pVector direction;	// Amount to increment velocity

	ExecMethod
};

struct PAJet : public PABaseClass
{
	pVector	center;		// Center of the fan
	pDomain acc;		// Acceleration vector domain
	float magnitude;	// Scales acceleration
	float epsilon;		// Softening parameter
	float max_radius;	// Only influence particles within max_radius

	ExecMethod
};

struct PAKillOld : public PABaseClass
{
	float age_limit;		// Exact age at which to kill particles.
	bool kill_less_than;	// True to kill particles less than limit.

	ExecMethod
};

struct PAMatchVelocity : public PABaseClass
{
	float magnitude;	// The grav of each particle
	float epsilon;		// Softening parameter
	float max_radius;	// Only influence particles within max_radius

	ExecMethod
};

struct PAMatchRotVelocity : public PABaseClass
{
	float magnitude;	// The grav of each particle
	float epsilon;		// Softening parameter
	float max_radius;	// Only influence particles within max_radius

	ExecMethod
};

struct PAMove : public PABaseClass
{

	ExecMethod
};

struct PAOrbitLine : public PABaseClass
{
	pVector p, axis;	// Endpoints of line to which particles are attracted
	float magnitude;	// Scales acceleration
	float epsilon;		// Softening parameter
	float max_radius;	// Only influence particles within max_radius

	ExecMethod
};

struct PAOrbitPoint : public PABaseClass
{
	pVector center;		// Point to which particles are attracted
	float magnitude;	// Scales acceleration
	float epsilon;		// Softening parameter
	float max_radius;	// Only influence particles within max_radius

	ExecMethod
};

struct PARandomAccel : public PABaseClass
{
	pDomain gen_acc;	// The domain of random accelerations.

	ExecMethod
};

struct PARandomDisplace : public PABaseClass
{
	pDomain gen_disp;	// The domain of random displacements.

	ExecMethod
};

struct PARandomVelocity : public PABaseClass
{
	pDomain gen_vel;	// The domain of random velocities.

	ExecMethod
};

struct PARandomRotVelocity : public PABaseClass
{
	pDomain gen_vel;	// The domain of random velocities.

	ExecMethod
};

struct PARestore : public PABaseClass
{
	float time_left;		// Time remaining until they should be in position.
	bool restore_velocity;
	bool restore_rvelocity;

	ExecMethod
};

struct PASink : public PABaseClass
{
	bool kill_inside;	// True to dispose of particles *inside* domain
	pDomain position;	// Disposal region

	ExecMethod
};

struct PASinkVelocity : public PABaseClass
{
	bool kill_inside;	// True to dispose of particles with vel *inside* domain
	pDomain velocity;	// Disposal region

	ExecMethod
};

struct PASort : public PABaseClass
{
	pVector Eye;		// A point on the line to project onto
	pVector Look;		// The direction for which to sort particles

	ExecMethod
};

struct PASource : public PABaseClass
{
	pDomain position;	// Choose a position in this domain.
	pDomain positionB;	// Choose a positionB in this domain.
	pDomain upVector;	// Choose an up vector in this domain
	pDomain velocity;	// Choose a velocity in this domain.
	pDomain rvelocity;	// Choose a rotation velocity in this domain.
	pDomain size;		// Choose a size in this domain.
	pDomain color;		// Choose a color in this domain.
	pDomain alpha;		// Choose an alpha in this domain.
	float particle_rate;	// Particles to generate per unit time
	float age;			// Initial age of the particles
	float age_sigma;	// St. dev. of initial age of the particles
	bool vertexB_tracks;	// True to get positionB from position.

	ExecMethod
};

struct PASpeedLimit : public PABaseClass
{
	float min_speed;		// Clamp speed to this minimum.
	float max_speed;		// Clamp speed to this maximum.

	ExecMethod
};

struct PATargetColor : public PABaseClass
{
	pVector color;		// Color to shift towards
	float alpha;		// Alpha value to shift towards
	float scale;		// Amount to shift by (1 == all the way)

	ExecMethod
};

struct PATargetSize : public PABaseClass
{
	pVector size;		// Size to shift towards
	pVector scale;		// Amount to shift by per frame (1 == all the way)

	ExecMethod
};

struct PATargetVelocity : public PABaseClass
{
	pVector velocity;	// Velocity to shift towards
	float scale;		// Amount to shift by (1 == all the way)

	ExecMethod
};

struct PATargetRotVelocity : public PABaseClass
{
	pVector velocity;	// Velocity to shift towards
	float scale;		// Amount to shift by (1 == all the way)

	ExecMethod
};

struct PAVortex : public PABaseClass
{
	pVector tip;		// Tip of vortex
	pVector axis;		// Axis around which vortex is applied
	float magnitude;	// Scale for rotation around axis
	float tightnessExponent; // Raise to this power to create vortex-like silhouette
	float rotSpeed;     // How fast to rotate around
	float epsilon;		// Softening parameter
	float max_radius;	// Only influence particles within max_radius

	ExecMethod
};

struct PAHeader : public PABaseClass
{
	unsigned char padding[sizeof(PASource)+16];	// This must be the largest action.

	ExecMethod
};
