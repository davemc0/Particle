#ifndef _Effects_h
#define _Effects_h

class ParticleEffects
{
private:
	char *EffectName;

public:
	static const int NumEffects = 22;

	int maxParticles; // The number of particles the app wants in this demo
	int numSteps; // The number of simulation time steps per rendered frame
	int particle_handle; // The handle of the particle group
	int action_handle; // The handle of the action list being created or used

	char *GetCurEffectName() {return EffectName;}

	ParticleEffects(int mp = 100);

	// Call this to get a demo by number.
	// Returns the DemoNum % num_demos.
	int CallDemo(int DemoNum, bool FirstTime, bool immediate);

	// Particles orbiting a center
	void Atom(bool FirstTime);

	// A bunch of balloons
	void Balloons(bool FirstTime);

	// Whatever junk I want to throw in here
	void Experimental(bool FirstTime);

	// An explosion from the center of the universe, followed by gravity
	void Explosion(bool FirstTime);

	// Fireflies bobbing around
	void Fireflies(bool FirstTime);

	// Rocket-style fireworks
	void Fireworks(bool FirstTime);

	// It's like a flame thrower spinning around
	void FlameThrower(bool FirstTime);

	// A fountain spraying up in the middle of the screen
	void Fountain(bool FirstTime);

	// A bunch of particles in a grid shape
	void GridShape(bool FirstTime);

	// It's like a fan cruising around under a floor, blowing up on some ping pong balls
	void JetSpray(bool FirstTime);

	// A sprayer with particles that orbit two points
	void Orbit2(bool FirstTime);

	// It kinda looks like rain hitting a parking lot
	void Rain(bool FirstTime);

	// Restore particles to their positionB.
	void Restore(bool FirstTime);

	// A bunch of particles in a shaft shape
	void ShaftShape(bool FirstTime);

	// A sheet of particles falling down, avoiding various-shaped obstacles
	// SteerShape is one of PDSphere, PDTriangle, PDRectangle
	void Shower(bool FirstTime, int SteerShape);

	// A bunch of particles in a line that are attracted to the guy ahead of them in line
	void Snake(bool FirstTime);

	// A bunch of particles inside a sphere
	void Sphere(bool FirstTime);

	// A sprayer with particles orbiting a line
	void Swirl(bool FirstTime);

	// A waterfall bouncing off invisible rocks
	void Waterfall1(bool FirstTime);

	// A waterfall bouncing off invisible rocks
	void Waterfall2(bool FirstTime);
};

#endif
