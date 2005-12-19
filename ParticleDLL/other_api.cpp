// system.cpp
//
// Copyright 1998-2005 by David K. McAllister.
//
// This file implements the API calls that are not particle actions.

#include "general.h"

#include <iostream>

// For Windows DLL.
#ifdef WIN32
#ifdef PARTICLE_MAKE_DLL
BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif
#endif

////////////////////////////////////////////////////////
// State setting calls

PARTICLEDLL_API void pColor(float red, float green, float blue, float alpha)
{
	_ParticleState &_ps = _GetPState();

	_ps.Alpha = pDomain(PDPoint, alpha, alpha, alpha);
	_ps.Color = pDomain(PDPoint, red, green, blue);
}

PARTICLEDLL_API void pColorD(float alpha, PDomainEnum dtype,
							 float a0, float a1, float a2,
							 float a3, float a4, float a5,
							 float a6, float a7, float a8)
{
	_ParticleState &_ps = _GetPState();

	_ps.Alpha = pDomain(PDPoint, alpha, alpha, alpha);
	_ps.Color = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

PARTICLEDLL_API void pAlphaD(PDomainEnum dtype,
							 float a0, float a1, float a2,
							 float a3, float a4, float a5,
							 float a6, float a7, float a8)
{
	_ParticleState &_ps = _GetPState();
	_ps.Alpha = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

PARTICLEDLL_API void pUpVector(float x, float y, float z)
{
	_ParticleState &_ps = _GetPState();

	_ps.Up = pDomain(PDPoint, x, y, z);
}

PARTICLEDLL_API void pUpVectorD(PDomainEnum dtype,
								float a0, float a1, float a2,
								float a3, float a4, float a5,
								float a6, float a7, float a8)
{
	_ParticleState &_ps = _GetPState();

	_ps.Up = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

PARTICLEDLL_API void pVelocity(float x, float y, float z)
{
	_ParticleState &_ps = _GetPState();

	_ps.Vel = pDomain(PDPoint, x, y, z);
}

PARTICLEDLL_API void pVelocityD(PDomainEnum dtype,
								float a0, float a1, float a2,
								float a3, float a4, float a5,
								float a6, float a7, float a8)
{
	_ParticleState &_ps = _GetPState();

	_ps.Vel = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

PARTICLEDLL_API void pRotVelocity(float x, float y, float z)
{
	_ParticleState &_ps = _GetPState();

	_ps.RotVel = pDomain(PDPoint, x, y, z);
}

PARTICLEDLL_API void pRotVelocityD(PDomainEnum dtype,
								   float a0, float a1, float a2,
								   float a3, float a4, float a5,
								   float a6, float a7, float a8)
{
	_ParticleState &_ps = _GetPState();

	_ps.RotVel = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

PARTICLEDLL_API void pVertexB(float x, float y, float z)
{
	_ParticleState &_ps = _GetPState();

	_ps.VertexB = pDomain(PDPoint, x, y, z);
}

PARTICLEDLL_API void pVertexBD(PDomainEnum dtype,
							   float a0, float a1, float a2,
							   float a3, float a4, float a5,
							   float a6, float a7, float a8)
{
	_ParticleState &_ps = _GetPState();

	_ps.VertexB = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


PARTICLEDLL_API void pVertexBTracks(bool trackVertex)
{
	_ParticleState &_ps = _GetPState();

	_ps.vertexB_tracks = trackVertex;
}

PARTICLEDLL_API void pSize(float size_x, float size_y, float size_z)
{
	_ParticleState &_ps = _GetPState();

	_ps.Size = pDomain(PDPoint, size_x, size_y, size_z);
}

PARTICLEDLL_API void pSizeD(PDomainEnum dtype,
							float a0, float a1, float a2,
							float a3, float a4, float a5,
							float a6, float a7, float a8)
{
	_ParticleState &_ps = _GetPState();

	_ps.Size = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

PARTICLEDLL_API void pMass(float mass)
{
	_ParticleState &_ps = _GetPState();

	_ps.Mass = mass;
}

PARTICLEDLL_API void pStartingAge(float age, float sigma)
{
	_ParticleState &_ps = _GetPState();

	_ps.Age = age;
	_ps.AgeSigma = sigma;
}

PARTICLEDLL_API void pTimeStep(float newDT)
{
	_ParticleState &_ps = _GetPState();

	_ps.dt = newDT;
}

////////////////////////////////////////////////////////
// Action List Calls

PARTICLEDLL_API int pGenActionLists(int action_list_count)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return -1; // ERROR

	_PLock();

	int ind = _ps.GenerateALists(action_list_count);

	_PUnLock();

	return ind;
}

PARTICLEDLL_API void pNewActionList(int action_list_num)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return; // ERROR

	_PLock();

	_ps.alist_id = action_list_num;
	if(_ps.alist_id < 0 || _ps.alist_id >= (int)_ps.ALists.size())
		return; // ERROR

	_ps.in_new_list = true;
	_ps.ALists[_ps.alist_id].resize(0); // Remove any old actions

	_PUnLock();
}

PARTICLEDLL_API void pEndActionList()
{
	_ParticleState &_ps = _GetPState();

	if(!_ps.in_new_list)
		return; // ERROR

	_ps.in_new_list = false;

	_ps.alist_id = -1;
}

PARTICLEDLL_API void pDeleteActionLists(int action_list_num, int action_list_count)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return; // ERROR

	if(action_list_num < 0)
		return; // ERROR

	_PLock();

	if(action_list_num + action_list_count > (int)_ps.ALists.size())
		return; // ERROR

	for(int i = action_list_num; i < action_list_num + action_list_count; i++) {
		_ps.ALists[i].resize(0);
	}

	_PUnLock();
}

PARTICLEDLL_API void pCallActionList(int action_list_num)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list) {
		// Add this call as an action to the current list.
		PACallActionList S;
		S.action_list_num = action_list_num;

		_pSendAction(&S, PACallActionListID, sizeof(PACallActionList));
	} else {
		// Execute the specified action list.
		_PLock();

		if(action_list_num < 0 || action_list_num >= (int)_ps.ALists.size())
			return; // ERROR

		ActionList &AList = _ps.ALists[action_list_num];

		// Not sure it's safe to unlock here since AList will be accessed by another thread while
		// we're executing it, but we can't stay locked while doing all the actions or it's not parallel.
		_PUnLock();

		_ps.in_call_list = true;
		for(ActionList::iterator it = AList.begin(); it != AList.end(); ) {
			it->dt = _ps.dt; // This is a hack to provide local access to dt.
			int cnt = _pExecuteAction((PAHeader *)&*it, &_ps.PGroups[_ps.pgroup_id]);
			it += cnt;
		}
		_ps.in_call_list = false;
	}
}

////////////////////////////////////////////////////////
// Particle Group Calls

// Create particle groups, each with max_particles allocated.
PARTICLEDLL_API int pGenParticleGroups(int p_group_count, size_t max_particles)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return -1; // ERROR

	_PLock();

	int ind = _ps.GeneratePGroups(p_group_count);

	for(int i = ind; i < ind + p_group_count; i++) {
		_ps.PGroups[i].SetMaxParticles(max_particles);
	}

	_PUnLock();

	return ind;
}

PARTICLEDLL_API void pDeleteParticleGroups(int p_group_num, int p_group_count)
{
	_ParticleState &_ps = _GetPState();

	if(p_group_num < 0)
		return; // ERROR

	_PLock();

	if(p_group_num + p_group_count > (int)_ps.ALists.size())
		return; // ERROR

	for(int i = p_group_num; i < p_group_num + p_group_count; i++) {
		_ps.PGroups[i].SetMaxParticles(0);
		_ps.PGroups[i].GetList().resize(0);
	}

	_PUnLock();
}

// Change which group is current.
PARTICLEDLL_API void pCurrentGroup(int p_group_num)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return; // ERROR

	_PLock();

	if(p_group_num < 0 || p_group_num >= (int)_ps.PGroups.size())
		return; // ERROR

	_PUnLock();

	_ps.pgroup_id = p_group_num;
}

// Change the maximum number of particles in the current group.
PARTICLEDLL_API size_t pSetMaxParticles(size_t max_count)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return 0; // ERROR

	if(max_count < 0)
		return 0; // ERROR

	// This can kill them and call their death callback.
	_ps.GetPGroup(_ps.pgroup_id).SetMaxParticles(max_count);

	return max_count;
}

// Copy from the specified group to the current group.
PARTICLEDLL_API void pCopyGroup(int p_src_group_num, size_t index, size_t copy_count)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return; // ERROR

	_PLock();

	if(p_src_group_num < 0 || p_src_group_num >= (int)_ps.PGroups.size())
		return; // ERROR

	ParticleGroup &srcgrp = _ps.GetPGroup(p_src_group_num);

	ParticleGroup &destgrp = _ps.GetPGroup(_ps.pgroup_id);

	// Find out exactly how many to copy.
	size_t ccount = copy_count;
	if(ccount > srcgrp.size() - index)
		ccount = srcgrp.size() - index;
	if(ccount > destgrp.GetMaxParticles() - destgrp.size())
		ccount = destgrp.GetMaxParticles() - destgrp.size();
	if(ccount<0)
		ccount = 0;

	// Directly copy the particles to the current list.
	for(size_t i=0; i<ccount; i++) {
		// Is it bad to call a birth callback while locked?
		destgrp.Add(srcgrp.GetList()[index+i]);
	}

	_PUnLock();
}

// Copy from the current group to application memory.
PARTICLEDLL_API size_t pGetParticles(size_t index, size_t count, float *verts,
									 float *color, float *vel, float *size, float *age)
{
	_ParticleState &_ps = _GetPState();

	// XXX I should think about whether color means color3, color4, or what.
	// For now, it means color4.

	if(_ps.in_new_list)
		return -1; // ERROR

	_PLock();

	if(_ps.pgroup_id < 0 || _ps.pgroup_id >= (int)_ps.PGroups.size())
		return -2; // ERROR

	if(index < 0 || count < 0)
		return -3; // ERROR

	ParticleGroup &pg = _ps.PGroups[_ps.pgroup_id];

	_PUnLock();

	if(index + count > pg.size()) {
		count = pg.size() - index;
		if(count <= 0)
			return -4; // ERROR index out of bounds.
	}

	int vi = 0, ci = 0, li = 0, si = 0, ai = 0;

	// This should be optimized.
	for(size_t i=index; i<index+count; i++) {
		const Particle &m = pg.GetList()[i];

		if(verts) {
			verts[vi++] = m.pos.x;
			verts[vi++] = m.pos.y;
			verts[vi++] = m.pos.z;
		}

		if(color) {
			color[ci++] = m.color.x;
			color[ci++] = m.color.y;
			color[ci++] = m.color.z;
			color[ci++] = m.alpha;
		}

		if(vel) {
			vel[li++] = m.vel.x;
			vel[li++] = m.vel.y;
			vel[li++] = m.vel.z;
		}

		if(size) {
			size[si++] = m.size.x;
			size[si++] = m.size.y;
			size[si++] = m.size.z;
		}

		if(age) {
			age[ai++] = m.age;
		}
	}

	return count;
}

// Return a pointer to the particle data, together with the stride IN FLOATS
// from one particle to the next and the offset IN FLOATS from the start of the particle
// for each attribute's data. The number in the arg name is how many floats the attribute
// consists of.
//
// WARNING: This function gives the application access to memory allocated and controlled
// by the Particle API. Don't do anything stupid with this power or you will regret it.
PARTICLEDLL_API size_t pGetParticlePointer(float *&ptr, size_t &stride, size_t &pos3Ofs, size_t &posB3Ofs,
										   size_t &size3Ofs, size_t &vel3Ofs, size_t &velB3Ofs,
										   size_t &color3Ofs, size_t &alpha1Ofs, size_t &age1Ofs)
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return -1; // ERROR

	ParticleGroup &pg = _ps.PGroups[_ps.pgroup_id];

	if(pg.size() < 1) {
		return -4; // ERROR index out of bounds.
	}

	ParticleList::iterator it = pg.begin();
	Particle *p0 = &(*it);
	++it;
	Particle *p1 = &(*it);
	float *fp0 = (float *)p0;
	float *fp1 = (float *)p1;

	ptr = (float *)p0;
	stride = fp1 - fp0;
	pos3Ofs = (float *)&(p0->pos.x) - fp0;
	posB3Ofs = (float *)&(p0->posB.x) - fp0;
	size3Ofs = (float *)&(p0->size.x) - fp0;
	vel3Ofs = (float *)&(p0->vel.x) - fp0;
	velB3Ofs = (float *)&(p0->velB.x) - fp0;
	color3Ofs = (float *)&(p0->color.x) - fp0;
	alpha1Ofs = (float *)&(p0->alpha) - fp0;
	age1Ofs = (float *)&(p0->age) - fp0;

	return pg.size();
}

// Returns the number of particles currently in the group.
PARTICLEDLL_API size_t pGetGroupCount()
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return 0; // ERROR

	_ps.GetPGroup(_ps.pgroup_id);
	_PLock();

	if(_ps.pgroup_id < 0 || _ps.pgroup_id >= (int)_ps.PGroups.size())
		return -2; // ERROR

	_PUnLock();

	return _ps.PGroups[_ps.pgroup_id].size();
}

// Returns the maximum number of allowed particles
PARTICLEDLL_API size_t pGetMaxParticles()
{
	_ParticleState &_ps = _GetPState();

	if(_ps.in_new_list)
		return 0; // ERROR

	_PLock();

	if(_ps.pgroup_id < 0 || _ps.pgroup_id >= (int)_ps.PGroups.size())
		return -2; // ERROR

	_PUnLock();

	return _ps.PGroups[_ps.pgroup_id].GetMaxParticles();
}

////////////////////////////////////////////////////////
// Other API Calls

PARTICLEDLL_API void pBirthCallback(P_PARTICLE_CALLBACK callback, void *data)
{
	_ParticleState &_ps = _GetPState();
	if(_ps.in_new_list)
		return; // ERROR

	_ps.GetPGroup(_ps.pgroup_id).SetBirthCallback(callback, data);
}

PARTICLEDLL_API void pDeathCallback(P_PARTICLE_CALLBACK callback, void *data)
{
	_ParticleState &_ps = _GetPState();
	if(_ps.in_new_list)
		return; // ERROR

	_ps.GetPGroup(_ps.pgroup_id).SetDeathCallback(callback, data);
}

PARTICLEDLL_API void pReset()
{
	_ParticleState &_ps = _GetPState();
	if(_ps.in_new_list)
		return; // ERROR
	
	_ps.GetPGroup(_ps.pgroup_id).GetList().clear();
}

PARTICLEDLL_API void pSeed(unsigned int seed)
{
	pSRandf(seed);
}
