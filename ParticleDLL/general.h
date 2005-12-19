// general.h
//
// Copyright 1998-2005 by David K. McAllister.
//
// This file contains everything for the implementation of the API.
// It is only included by API implementation files, not by applications.
//
// Defines these classes: Particle, ParticleGroup, _ParticleState

#ifndef general_h
#define general_h

#include "papi.h"
#include "pVector.h"
#include "pDomain.h"
#include "actions.h"

#include <vector>

#ifdef _WIN32
// #pragma warning (disable:4244)
#endif

// A single particle
struct Particle
{
	pVector pos;
	pVector posB;
	pVector up;
	pVector upB;
	pVector vel;
	pVector velB;	// Used to compute binormal, normal, etc.
	pVector rvel;
	pVector rvelB;
	pVector size;
	pVector color;	// Color must be next to alpha so glColor4fv works.
	float alpha;	// This is both cunning and scary.
	float age;
	float mass;
	long data;		// arbitrary data for user
	float tmp0;		// These temporaries are used as padding and for sorting.

	inline Particle(const pVector &pos, const pVector &posB,
		const pVector &up, const pVector &upB,
		const pVector &vel, const pVector &velB, 
		const pVector &rvel, const pVector &rvelB, 
		const pVector &size, const pVector &color,
		float alpha, float age, float mass, long data, float tmp0) :
	pos(pos), posB(posB),
		up(up), upB(upB),
		vel(vel), velB(velB), 
		rvel(rvel), rvelB(rvelB), 
		size(size), color(color), 
		alpha(alpha), age(age), mass(mass), data(data), tmp0(0)
	{
	}

	inline Particle(const Particle &rhs) :
	pos(rhs.pos), posB(rhs.posB),
		up(rhs.up), upB(rhs.upB),
		vel(rhs.vel), velB(rhs.velB), 
		rvel(rhs.rvel), rvelB(rhs.rvelB), 
		size(rhs.size), color(rhs.color),
		alpha(rhs.alpha), age(rhs.age), mass(rhs.mass), data(rhs.data), tmp0(rhs.tmp0)
	{
	}

	Particle() : data(0)
	{
	}

	// For sorting.
	bool operator<(const Particle &P)
	{
		return tmp0 < P.tmp0;
	}
};

typedef std::vector<Particle> ParticleList;

// A group of particles - Info and an array of Particles
class ParticleGroup
{
	ParticleList list;

	size_t max_particles;	// Max particles allowed in group.
	P_PARTICLE_CALLBACK cb_birth;
	P_PARTICLE_CALLBACK cb_death;
	void *cb_birth_data;
	void *cb_death_data;

public:
	ParticleGroup()
	{
		max_particles = 0;
		cb_birth = NULL;
		cb_death = NULL;
		cb_birth_data = NULL;
		cb_death_data = NULL;
	}

	ParticleGroup(size_t maxp) : max_particles(maxp)
	{
		cb_birth = NULL;
		cb_death = NULL;
		cb_birth_data = NULL;
		cb_death_data = NULL;
		list.reserve(max_particles);
	}
	ParticleGroup(const ParticleGroup &rhs) : list(rhs.list)
	{
		cb_birth = rhs.cb_birth;
		cb_death = rhs.cb_death;
		cb_birth_data = rhs.cb_birth_data;
		cb_death_data = rhs.cb_death_data;
		max_particles = rhs.max_particles;
	}
	~ParticleGroup()
	{
		if (cb_death) {
			ParticleList::iterator it;
			for (it = list.begin(); it != list.end(); ++it)
				(*cb_death)((*it), cb_death_data);
		}
	}
	ParticleGroup &operator=(const ParticleGroup &rhs)
	{
		if (this != &rhs) {
			if (cb_death) {
				ParticleList::iterator it;
				for (it = list.begin(); it != list.end(); ++it)
					(*cb_death)((*it), cb_death_data);
			}
			list = rhs.list;
			cb_birth = rhs.cb_birth;
			cb_death = rhs.cb_death;
			cb_birth_data = rhs.cb_birth_data;
			cb_death_data = rhs.cb_death_data;
			max_particles = rhs.max_particles;
		}
		return *this;
	}

	inline size_t GetMaxParticles() { return max_particles; }
	inline ParticleList &GetList() { return list; }
	inline void SetBirthCallback(P_PARTICLE_CALLBACK cbb, void *cbb_data)
	{
		cb_birth = cbb;
		cb_birth_data = cbb_data;
	}

	inline void SetDeathCallback(P_PARTICLE_CALLBACK cbd, void *cbd_data)
	{
		cb_death = cbd;
		cb_death_data = cbd_data;
	}

	inline void SetMaxParticles(size_t maxp)
	{
		max_particles = maxp;
		if(list.size() > max_particles) {
			if (cb_death) {
				ParticleList::iterator it;
				for (it = &(list[max_particles]); it != list.end(); ++it)
					(*cb_death)((*it), cb_death_data);
			}
			list.resize(max_particles);
		}
		list.reserve(max_particles);
	}

	inline size_t size() const { return list.size(); }
	inline ParticleList::iterator begin() { return list.begin(); }
	inline ParticleList::iterator end() { return list.end(); }

	inline void Remove(ParticleList::iterator it)
	{
		if (cb_death)
			(*cb_death)((*it), cb_death_data);

		// Copy the one from the end to here.
		if(it != list.end() - 1) {
			*it = *(list.end() - 1);
		}

		// Delete the one at the end
		list.pop_back();
	}

	inline bool Add(const pVector &pos, const pVector &posB,
		const pVector &up, 
		const pVector &vel, const pVector &rvel,
		const pVector &size, const pVector &color,
		const float alpha = 1.0f,
		const float age = 0.0f,
		const float mass = 1.0f,
		const long data = 0)
	{
		if (list.size() >= max_particles)
			return false;
		else {
			list.push_back(Particle(pos, posB, up, up, vel, vel, rvel, rvel, size, color, alpha, age, mass, data, 0.0f));
			Particle &p = list.back();
			if (cb_birth)
				(*cb_birth)(p, cb_birth_data);
			return true;
		}
	}

	inline bool Add(const Particle &P)
	{
		if (list.size() >= max_particles)
			return false;
		else {
			list.push_back(P);
			Particle &p = list.back();
			if (cb_birth)
				(*cb_birth)(p, cb_birth_data);
			return true;
		}
	}
};

typedef std::vector<PAHeader> ActionList;

// This is the per-thread state of the API.
// All API calls get their data from here.
// In the non-multithreaded case there is one global instance of this class.
struct _ParticleState
{
	// Any particles created will get their attributes from here.
	pDomain Up;
	pDomain Vel;
	pDomain RotVel;
	pDomain VertexB;
	pDomain Size;
	pDomain Color;
	pDomain Alpha;
	float Age;
	float AgeSigma;
	float Mass;

	float dt;
	bool in_call_list;
	bool in_new_list;
	bool vertexB_tracks;
	int tid;             // Only used in the MP case, but always define it.

	static std::vector<ParticleGroup> PGroups; // Static since all threads access the same action lists. Lock all accesses.
	int pgroup_id;
	int GeneratePGroups(int pgroups_requested);
	ParticleGroup &GetPGroup(int p_group_num);

	static std::vector<ActionList> ALists; // Static since all threads access the same action lists. Lock all accesses.
	int alist_id;
	int GenerateALists(int alists_requested);
	ActionList &GetAList(int a_list_num);

	_ParticleState();
};

#ifdef PARTICLE_MP
// All entry points call this to get their particle state.
inline _ParticleState &_GetPState()
{
	// Returns a reference to the appropriate particle state.
	extern _ParticleState &_GetPStateWithTID();

	return _GetPStateWithTID();
}

inline void _PLock() { /* Do lock stuff here. */ }
inline void _PUnLock() { /* Do lock stuff here. */ }

#else

// All entry points call this to get their particle state.
// For the non-MP case this is practically a no-op.
inline _ParticleState &_GetPState()
{
	// This is the global state.
	extern _ParticleState __ps;

	return __ps;
}

inline void _PLock() {}
inline void _PUnLock() {}

#endif

////////////////////////////////////////////////////////
// Auxiliary calls

// Execute action pa on particle group pg.
int _pExecuteAction(PAHeader *pa, ParticleGroup *pg);

// Add action S to the end of the current action list.
void _pAddActionToList(PABaseClass *S, int size);

// Action API calls call this to either store the action in a list or execute it.
void _pSendAction(PABaseClass *S, PActionEnum type, int size);

#endif
