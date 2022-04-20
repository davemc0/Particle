/// PInternalsIface.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// A public interface to the private library internals
/// Library users need not access this file.

#ifndef PInternalsIface_h
#define PInternalsIface_h

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

    float dt;
    Particle_t* ibegin;
    Particle_t* iend;
};

class PInternalState_t; // The API-internal struct containing the context's state. Don't try to use it.

void StartParticleLoop(std::shared_ptr<PInternalState_t> PS, PInternalShadow_t& PSh);
void EndParticleLoop(std::shared_ptr<PInternalState_t> PS, PInternalShadow_t& PSh);
} // namespace PAPI

#endif
