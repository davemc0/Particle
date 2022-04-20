/// PInternalState.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// A public interface to the private library internals
/// Library users need not access this file.

#ifndef PInternalState_h
#define PInternalState_h

#include "Particle/pAPIContext.h"
#include "ParticleGroup.h"

#include <string>
#include <vector>

namespace PAPI {
struct PActionBase;

class ActionList : public std::vector<std::shared_ptr<PActionBase>> {
public:
    ActionList() {}
};

// This is the per-thread state of the API. All API calls get their data from here.
// In the non-multithreaded case there is one global instance of this class.
class PInternalState_t {
public:
    PInternalState_t();

    bool get_in_call_list() const { return in_call_list; }
    bool get_in_new_list() const { return in_new_list; }
    bool get_in_particle_loop() const { return in_particle_loop; }
    float get_dt() const { return dt; }
    int get_alist_id() const { return alist_id; }
    int get_pgroup_id() const { return pgroup_id; }
    int get_working_set_size() const { return working_set_size; }

    void set_alist_id(const int alist_id_) { alist_id = alist_id_; }
    void set_dt(const float dt_) { dt = dt_; }
    void set_in_call_list(const int in_call_list_) { in_call_list = in_call_list_; }
    void set_in_new_list(const int in_new_list_) { in_new_list = in_new_list_; }
    void set_in_particle_loop(const int in_particle_loop_) { in_particle_loop = in_particle_loop_; }
    void set_pgroup_id(const int pgroup_id_) { pgroup_id = pgroup_id_; }
    void set_working_set_size(const int working_set_size_) { working_set_size = working_set_size_; }

    int GenerateALists(int alists_requested);
    int GeneratePGroups(int pgroups_requested);
    void ExecuteActionList(ActionList& AList);       // Execute an action list
    void SendAction(std::shared_ptr<PActionBase> S); // Action API entry points call this to either store the action in a list or execute and delete it.

    std::vector<ActionList>& getALists() { return ALists; }
    std::vector<ParticleGroup>& getPGroups() { return PGroups; }

private:
    bool in_call_list;
    bool in_new_list;
    bool in_particle_loop;
    float dt;
    int alist_id;
    int pgroup_id;
    int working_set_size; // How many particles will fit in cache

    std::vector<ActionList> ALists;
    std::vector<ParticleGroup> PGroups;
};
}; // namespace PAPI

#endif
