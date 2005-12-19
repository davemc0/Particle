// system.cpp
//
// Copyright 1998-2005 by David K. McAllister.
//
// This file implements most of the under-the-hood stuff.

// To optimize action lists, at execute time check whether the first action can be combined with
// the next action. If so, combine them. Then try again. When the current one can't be combined
// with the next one anymore, execute it.

// Doing this at run time instead of compile time should make it easier to store the state of
// compound actions.

#include "general.h"

#include <iostream>

// This is global just as a cheezy way to change dt for all actions easily.
float PABaseClass::dt;

std::vector<ParticleGroup> _ParticleState::PGroups;
std::vector<ActionList> _ParticleState::ALists;

// This is the global state. It is used for single-threaded apps.
// Multithreaded apps should get a different _ParticleState per thread from a hash table.
_ParticleState __ps;

_ParticleState::_ParticleState()
{
	in_call_list = false;
	in_new_list = false;
	vertexB_tracks = true;

	dt = 1.0f;

	pgroup_id = -1;
	alist_id = -1;
	tid = 0; // This will be filled in above if we're MT.

	Up = pDomain(PDPoint, 0.0f, 1.0f, 0.0f);
	Vel = pDomain(PDPoint, 0.0f, 0.0f, 0.0f);
	RotVel = pDomain(PDPoint, 0.0f, 0.0f, 0.0f);
	VertexB = pDomain(PDPoint, 0.0f, 0.0f, 0.0f);
	Size = pDomain(PDPoint, 1.0f, 1.0f, 1.0f);
	Color = pDomain(PDPoint, 1.0f, 1.0f, 1.0f);
	Alpha = pDomain(PDPoint, 1.0f, 1.0f, 1.0f);
	Age = 0.0f;
	AgeSigma = 0.0f;
	Mass = 1.0f;
}

ParticleGroup &_ParticleState::GetPGroup(int p_group_num)
{
	if(p_group_num < 0) {
		std::cerr << "ERROR: Negative particle group number\n";
		abort(); // IERROR
	}

	if(p_group_num >= (int)PGroups.size()) {
		std::cerr << "ERROR: Bad particle group number\n";
		abort(); // IERROR
	}

	return PGroups[p_group_num];
}

ActionList &_ParticleState::GetAList(int a_list_num)
{
	if(a_list_num < 0) {
		std::cerr << "ERROR: Negative action list number\n";
		abort(); // IERROR
	}

	if(a_list_num >= (int)ALists.size()) {
		std::cerr << "ERROR: Bad action list number\n";
		abort(); // IERROR
	}

	return ALists[a_list_num];
}

// Return an index into the list of particle groups where
// p_group_count groups can be added.
int _ParticleState::GeneratePGroups(int pgroups_requested)
{
	int old_size = (int)PGroups.size();
	PGroups.resize(old_size + pgroups_requested);

	return old_size;
}

// Return an index into the list of action lists where
// alists_requested lists can be added.
int _ParticleState::GenerateALists(int alists_requested)
{
	int old_size = (int)ALists.size();
	ALists.resize(old_size + alists_requested);

	return old_size;
}

////////////////////////////////////////////////////////
// Auxiliary calls

// Execute action pa on particle group pg.
int _pExecuteAction(PAHeader *pa, ParticleGroup *pg)
{
	// All these require a particle group, so check for it.
	if(pg == NULL)
		abort();

	// Step through all the actions in the action list.
	switch(pa->type) {
	case PAAvoidID:
		((PAAvoid *)pa)->Execute(pg);
		break;
	case PABounceID:
		((PABounce *)pa)->Execute(pg);
		break;
	case PACallActionListID:
		((PACallActionList *)pa)->Execute(pg);
		break;
	case PACopyVertexBID:
		((PACopyVertexB *)pa)->Execute(pg);
		break;
	case PADampingID:
		((PADamping *)pa)->Execute(pg);
		break;
	case PARotDampingID:
		((PARotDamping *)pa)->Execute(pg);
		break;
	case PAExplosionID:
		((PAExplosion *)pa)->Execute(pg);
		break;
	case PAFollowID:
		((PAFollow *)pa)->Execute(pg);
		break;
	case PAFountainID:
		((PAFountain *)pa)->AL = pa+1;
		((PAFountain *)pa)->Execute(pg);
		return 7; // The hard-coded fountain contains itself and 6 other actions. HACK!!!
		break;
	case PAGravitateID:
		((PAGravitate *)pa)->Execute(pg);
		break;
	case PAGravityID:
		((PAGravity *)pa)->Execute(pg);
		break;
	case PAJetID:
		((PAJet *)pa)->Execute(pg);
		break;
	case PAKillOldID:
		((PAKillOld *)pa)->Execute(pg);
		break;
	case PAMatchVelocityID:
		((PAMatchVelocity *)pa)->Execute(pg);
		break;
	case PAMatchRotVelocityID:
		((PAMatchRotVelocity *)pa)->Execute(pg);
		break;
	case PAMoveID:
		((PAMove *)pa)->Execute(pg);
		break;
	case PAOrbitLineID:
		((PAOrbitLine *)pa)->Execute(pg);
		break;
	case PAOrbitPointID:
		((PAOrbitPoint *)pa)->Execute(pg);
		break;
	case PARandomAccelID:
		((PARandomAccel *)pa)->Execute(pg);
		break;
	case PARandomDisplaceID:
		((PARandomDisplace *)pa)->Execute(pg);
		break;
	case PARandomVelocityID:
		((PARandomVelocity *)pa)->Execute(pg);
		break;
	case PARandomRotVelocityID:
		((PARandomRotVelocity *)pa)->Execute(pg);
		break;
	case PARestoreID:
		((PARestore *)pa)->Execute(pg);
		break;
	case PASinkID:
		((PASink *)pa)->Execute(pg);
		break;
	case PASinkVelocityID:
		((PASinkVelocity *)pa)->Execute(pg);
		break;
	case PASortID:
		((PASort *)pa)->Execute(pg);
		break;
	case PASourceID:
		((PASource *)pa)->Execute(pg);
		break;
	case PASpeedLimitID:
		((PASpeedLimit *)pa)->Execute(pg);
		break;
	case PATargetColorID:
		((PATargetColor *)pa)->Execute(pg);
		break;
	case PATargetSizeID:
		((PATargetSize *)pa)->Execute(pg);
		break;
	case PATargetVelocityID:
		((PATargetVelocity *)pa)->Execute(pg);
		break;
	case PATargetRotVelocityID:
		((PATargetRotVelocity *)pa)->Execute(pg);
		break;
	case PAVortexID:
		((PAVortex *)pa)->Execute(pg);
		break;
	default:
		std::cerr<<"ERROR: Bad action type\n";
		abort();
		break;
	}

	return 1;
}

// Add action S to the end of the current action list.
void _pAddActionToList(PABaseClass *S, int size)
{
	_ParticleState &_ps = _GetPState();

	if(!_ps.in_new_list)
		return; // ERROR

	ActionList &AList = _ps.GetAList(_ps.alist_id);
	// If I knew how, I could use RTTI to avoid this clunky idiom of copying the data.
	PAHeader tmp;
	memcpy(&tmp, S, size);
	AList.push_back(tmp);
}

// Action API calls call this to either store the action in a list or execute it.
void _pSendAction(PABaseClass *S, PActionEnum type, int size)
{
	// If I knew how, I could use RTTI to avoid this clunky idiom of passing the size.
	_ParticleState &_ps = _GetPState();

	S->type = type;

	if(_ps.in_new_list) {
		_pAddActionToList(S, size);
	} else {
		// Immediate mode. Execute it.
		S->dt = _ps.dt; // This is a hack to provide local access to dt.
		_pExecuteAction((PAHeader *)S, &_ps.GetPGroup(_ps.pgroup_id));
	}
}
