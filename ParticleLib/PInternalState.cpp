/// PInternalState.cpp
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// This file implements the PInternalState_t class and other under-the-hood stuff that is not part of an API call.
///
/// To optimize action lists, at execute time check whether the first action can be combined with
/// the next action. If so, combine them. Then try again. When the current one can't be combined
/// with the next one anymore, execute it.
///
/// Doing this at CallList time instead of list compile time should make it easier to store the state of compound actions.

#include "PInternalState.h"

#include "pAPI.h"

#include <typeinfo>

namespace PAPI {

// Constructor for the app-owned context
ParticleContext_t::ParticleContext_t()
{
    std::shared_ptr<PInternalState_t> PSt(new PInternalState_t);
    PContextActionList_t::InternalSetup(PSt);
    PContextActions_t::InternalSetup(PSt);
    PContextParticleGroup_t::InternalSetup(PSt);
}

void PContextActionList_t::InternalSetup(std::shared_ptr<PInternalState_t> St) { PS = St; }

void PContextActions_t::InternalSetup(std::shared_ptr<PInternalState_t> St) { PS = St; }

void PContextParticleGroup_t::InternalSetup(std::shared_ptr<PInternalState_t> St) { PS = St; }

std::shared_ptr<PInternalState_t> PContextActionList_t::getInternalState() const { return PS; }

// Constructor for the internal state
PInternalState_t::PInternalState_t()
{
    in_call_list = false;
    in_new_list = false;

    dt = 1.0f;

    pgroup_id = -1;
    alist_id = -1;

    PWorkingSetSize = (0x40000 / sizeof(Particle_t)); // Use 256 KB of cache.
}

// Return an index into the list of particle groups where
// p_group_count groups can be added.
int PInternalState_t::GeneratePGroups(int pgroups_requested)
{
    int old_size = (int)PGroups.size();
    PGroups.resize(old_size + pgroups_requested);

    return old_size;
}

// Return an index into the list of action lists where
// alists_requested lists can be added.
int PInternalState_t::GenerateALists(int alists_requested)
{
    int old_size = (int)ALists.size();
    ALists.resize(old_size + alists_requested);

    return old_size;
}

// Action API entry points call this to either store the action in a list or execute it.
void PInternalState_t::SendAction(std::shared_ptr<PActionBase> S)
{
    S->SetPInternalState(this); // Let the actions have access to the PInternalState_t

    if (in_new_list) {
        // Add action S to the end of the current action list.
        ActionList& AList = ALists[alist_id];
        AList.push_back(S);
    } else {
        // Immediate mode. Execute it.
        S->dt = dt; // Provide the action with access to the current dt.
        ParticleGroup& pg = PGroups[pgroup_id];
        S->Execute(pg, pg.begin(), pg.end());
    }
}

// Execute an action list
void PInternalState_t::ExecuteActionList(ActionList& AList)
{
    ParticleGroup& pg = PGroups[pgroup_id];
    in_call_list = true;

    PASSERT((AList.ALFunc == NULL && AList.Params == P_INTERNAL_CODE) || (AList.ALFunc && AList.Params != P_INTERNAL_CODE), "ALFunc and Params mismatch.");

    if (AList.Params & P_CPU_CPP_CODE) {  // Compiled mode
        AList.ALFunc(&AList, &pg, dt, 0); // We have a compiled AL for CPU so call that function
    } else {                              // Internal mode
        ActionList::iterator it = AList.begin();
        while (it != AList.end()) {
            // Make an action segment
            ActionList::iterator abeg = it;
            ActionList::iterator aend = it + 1;

            // If the first one is connectable, try to connect some more.
            if (!(*abeg)->GetKillsParticles() && !(*abeg)->GetDoNotSegment())
                while (aend != AList.end() && !(*aend)->GetKillsParticles() && !(*aend)->GetDoNotSegment()) aend++;

            // Found a sub-list that can be done together. Now do them.
            ParticleList::iterator pbeg = pg.begin();
            ParticleList::iterator pend = ((pg.end() - pbeg) <= PWorkingSetSize) ? pg.end() : (pbeg + PWorkingSetSize);
            bool one_pass = false;
            if (aend - abeg == 1) {
                pend = pg.end(); // If a single action, do the whole thing in one whack.
                one_pass = true;
            }

            ActionList::iterator ait = abeg;
            do {
                // For each chunk of particles, do all the actions in this sub-list
                ait = abeg;
                while (ait < aend) {
                    (*ait)->dt = dt; // Provide the action with access to the current dt.
                    (*ait)->Execute(pg, pbeg, pend);

                    ait++;
                }
                if (!one_pass) { // If we're not one_pass then we know we didn't do any actions that mangle our iterators.
                    pbeg = pend;
                    pend = ((pg.end() - pbeg) <= PWorkingSetSize) ? pg.end() : (pbeg + PWorkingSetSize);
                }
            } while ((!one_pass) && pbeg != pg.end());
            it = ait;
        }
    }

    in_call_list = false;
}
}; // namespace PAPI
