/// PInternalShadow.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// A shadow copy of some of the private library internals
/// Library users need not access this file.

#ifndef PInternalShadow_h
#define PInternalShadow_h

namespace PAPI {
struct Particle_t;

// Shadow copy of some information from PInternalState_t that is used by the inline actions API
// It is owned by pContextActions_t.
// It is updated by StartParticleLoop(), called from pContextActions_t::ParticleLoop().
class PInternalShadow_t {
public:
    float get_dt() { return dt; }
    const Particle_t* get_const_pgroup_begin() { return ibegin; } // Iterator to beginning of current particle group
    const Particle_t* get_const_pgroup_end() { return iend; }     // Iterator to end of current particle group
    Particle_t* get_pgroup_begin() { return ibegin; }             // Iterator to beginning of current particle group
    Particle_t* get_pgroup_end() { return iend; }                 // Iterator to end of current particle group
    bool get_in_new_list() const { return in_new_list; }
    bool get_in_particle_loop() const { return in_particle_loop; }

    float dt;
    Particle_t* ibegin;
    Particle_t* iend;

    bool in_new_list;
    bool in_particle_loop;
};

class PInternalState_t; // The API-internal struct containing the context's state. Don't try to use it.

void StartParticleLoop(std::shared_ptr<PInternalState_t> PS, PInternalShadow_t& PSh);
void EndParticleLoop(std::shared_ptr<PInternalState_t> PS, PInternalShadow_t& PSh);
} // namespace PAPI

#endif
