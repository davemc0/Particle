/// PInternalState.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// Defines these classes: PInternalState_t

#ifndef PInternalState_h
#define PInternalState_h

#include "Actions.h"
#include "LibHelpers.h"
#include "Particle/pAPIContext.h"
#include "ParticleGroup.h"

#include <string>
#include <vector>

namespace PAPI {

class ActionList : public std::vector<std::shared_ptr<PActionBase>> {
public:
    ActionList() {}
};

// This is the per-thread state of the API.
// All API calls get their data from here.
// In the non-multithreaded case there is one global instance of this class.
class PInternalState_t {
public:
    float dt;
    bool in_call_list;
    bool in_new_list;
    bool in_particle_loop;

    std::vector<ParticleGroup> PGroups;
    int pgroup_id;

    std::vector<ActionList> ALists;
    int alist_id;

    // How many particles will fit in cache? You can set this if you don't like the default value.
    int PWorkingSetSize;

    PInternalState_t();

    int GeneratePGroups(int pgroups_requested);
    int GenerateALists(int alists_requested);

    // Action API entry points call this to either store the action in a list or execute and delete it.
    void SendAction(std::shared_ptr<PActionBase> S);

    // Execute an action list
    void ExecuteActionList(ActionList& AList);
};
}; // namespace PAPI

#endif
