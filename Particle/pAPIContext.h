/// PAPIClasses.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// Include this file in all applications that use the Particle System API.

#ifndef particle_api_classes_h
#define particle_api_classes_h

#include "Particle/pDeclarations.h"
#include "Particle/pParticle.h"
#include "Particle/pSourceState.h"

#include <string>

namespace PAPI {

class PInternalState_t; // The API-internal struct containing the context's state. Don't try to use it.
struct Particle_t;

/// Action List Calls
///
/// These calls create and operate on action lists, which are scripts of many actions
/// to be applied together as a block to the current particle group. An empty action
/// list is first created using GenActionLists(), and is then filled or compiled by calling
/// NewActionList(), then calling Actions, then calling EndActionList(). Once the action
/// list is filled, it is run via CallActionList(). Thus, an action list is sort of a
/// higher-level action. Complex behaviors can be stored in an action list and then
/// called later, even as part of another action list. Action lists cannot be edited.
/// They can only be created or destroyed. To destroy an action list, call DeleteActionLists().
///
/// When in immediate mode (not creating or calling an action list), particles are created
/// with attributes from the current state. However, when particles are created within a
/// NewActionList() / EndActionList() block, they will receive attributes from the state that was
/// current when the action list was created (unlike OpenGL).
///
/// The time step length, dt, uses the value that is current when CallActionList() is executed, not the
/// value of dt when the action list was created. This allows dt to be modified without recompiling action
/// lists. Maybe this isn't a good idea. If it should be the other way in the future, let me know.
class PContextActionList_t {
public:
    /// Set the random number seed.
    ///
    /// The Particle API uses a pseudo-random number generator. The returned number is a function of the numbers already returned. If you start
    /// two threads, each with a ParticleContext_t they will both generate the same particles if given the same commands. If this is not desired,
    /// call Seed() on both of them with different seed values.
    /// The API currently uses the C standard library random number generator, whose state is per-thread, so all contexts in the thread share
    /// the same random number seed.
    void Seed(const unsigned int seed);

    /// Specify the time step length.
    ///
    /// The Particle System API uses a discrete time approximation to all actions. This means that actions are applied to the particles at a
    /// particular instant in time as if the action's effect accumulated over a small time interval, dt, with the world being constant over the
    /// interval. The clock is then "ticked" by the length of the interval and the actions can then be reapplied with the particles having their
    /// updated values. This is the standard method of doing almost all time-varying simulations in computer science.
    ///
    /// How does the time step, dt, relate to the application's frame rate? The easiest method is to apply the actions once per frame. If the
    /// application prefers to keep time in terms of seconds, dt can be set to (1 / frames_per_second). But more often, it is easier for a time
    /// unit to be one frame instead of one second. In this case, dt should be 1.0, which is the default.
    ///
    /// For higher quality, the application can apply particle actions more than once per frame. This provides smoother, more realistic results
    /// in many subtle ways. Suppose the application wanted to compute three animation steps for each rendered frame. Set dt to 1/3 its previous
    /// value using TimeStep(), then loop three times over all the action code that gets executed per frame, including the calls to Move. If using
    /// action lists, this can be simply a loop over the CallActionList() call. The run-time results should be about the same, but with fewer
    /// discrete approximation artifacts. Depending on how much non-particle work is done per frame, increasing the number of time steps per frame
    /// may or may not affect the frame rate very much.
    ///
    /// In terms of numerical integration, particle actions can be thought of as the first derivative of unknown functions dictating the particle
    /// attributes (such as position) over time. In order to compute the particle attributes these derivative functions must be integrated. Since
    /// closed form integration doesn't make sense for most actions, Euler's method is used instead. Euler's method is simply the method just
    /// described - the evaluation of the derivative functions at a particular time and then incrementing the current particle values by these
    /// derivative results times dt. In Euler's method, the smaller the dt, the more accurate the results.
    ///
    /// Unlike with other state setting calls, action lists execute using the current dt value set by TimeStep(), rather than the time step value
    /// that was current when the action list was created. Making action lists independent of time step size allows the time step to be changed
    /// without recompiling the action list.
    ///
    /// In general, it is folly to call TimeStep() in between other actions of a simulation frame. For example, calling Bounce(); TimeStep(); Move();
    /// can cause particles to pass through the bounce domain instead of bouncing off it.
    void TimeStep(const float new_dt);

    /// Return the current time step, as set with TimeStep()
    float GetTimeStep() const;

    /// Execute the specified action list on the current particle group.
    ///
    /// Call the action functions as specified when this action list was created with NewActionList(). The actions are executed with the state
    /// values in effect when the action list was created, except the context's current value of dt is used, not the value of dt when the list
    /// was created.
    ///
    /// CallActionList() is the only function other than actions that can be stored in an action list. This allows action lists to become atomic
    /// operations in more complex action lists.
    ///
    /// When calling CallActionList() during the creation of a new action list, action_list_num does not need to indicate an existing action list.
    /// At all other times it is an error for action_list_num to not indicate an existing (generated) action list.
    void CallActionList(const int action_list_num);

    /// Delete one or more consecutive action lists.
    ///
    /// Deletes action_list_count action lists, with action_list_num being the list number of the first one. The lists must be numbered
    /// sequentially, and must all exist. This removes the specified action lists from existence.
    void DeleteActionLists(const int action_list_num,      ///< The handle of the first action list to delete
                           const int action_list_count = 1 ///< How many action lists to delete
    );

    /// End the creation of a new action list.
    ///
    /// Obviously, it is an error to call EndActionList() without a corresponding call to NewActionList().
    void EndActionList();

    /// Generate a block of empty action lists.
    ///
    /// Returns the action list number of the first allocated list. All list numbers are in sequential order starting with the first list.
    /// Valid action list numbers are non-negative.
    int GenActionLists(const int action_list_count = 1 ///< How many action lists to create
    );

    /// Begin the creation of the specified action list.
    ///
    /// The action_list_num must have already been generated using GenActionLists. Most calls other than actions and state setting calls
    /// cannot be made between a call to NewActionList() and the corresponding call to EndActionList().
    /// If called on an action list that has previously been defined, the previous contents of the action list are destroyed and the action
    /// list will be created anew. This is as with glNewActionList() in OpenGL.
    void NewActionList(const int action_list_num);

    /// Return the source code string of the action list kernel
    ///
    /// Returns a string representing the C code that implements the action list.
    /// This string is suitable for saving as the complete contents of a .cu or .cpp file.
    /// NOT IMPLEMENTED! When I (or someone) actually implements emitted action lists, get it working completely with host code before attempting CUDA.
    void EmitActionList(const int action_list_num,     ///< the action list number returned from a call to GenActionLists().
                        std::string& Kernel,           ///< a reference to the string to store the kernel source code in
                        const std::string& KernelName, ///< your chosen name for this kernel. This can be the base of the filename where the contents are stored.
                        const EmitCodeParams_e Params  ///< Tells what kind of code to emit; currently host CPU C++ code or GPU CUDA code
    );

    /// Give a function pointer for executing the emitted and compiled action list kernel
    ///
    /// When an action list has been emitted and compiled by your emitter program it is then linked into your main application.
    /// To call this compiled action list function you must bind it to an action list. If the emitter program passed P_VARYING_* as an argument
    /// to any action in the emitted action list you must provide the actual desired values for those arguments at run time. To do this,
    /// create an action list identical to the compiled one, with the same actions in the same sequence, passing the same kinds of domains
    /// as arguments. The value of all parameters to all actions are ignored, except the ones whose value while being generated was P_VARYING_*.
    /// The varying values of those arguments are taken from these calls.
    ///
    /// Each time you want to change the value of any P_VARYING_* argument you must recreate the entire action list.
    /// You do not need to call BindEmittedActionList() again if you use the same action list number each time.
    /// If the emitted action list does not contain any P_VARYING_* arguments then its function can be bound to any action list,
    /// including an empty one created with just NewActionList(action_list_num); EndActionList().
    ///
    /// Once the action list is created you must provide the API the function pointer of the function that was emitted then linked to your app.
    /// The function's name is what you provided in the KernelName argument of EmitActionList(). Declare this function extern like so:
    ///     extern P_PARTICLE_EMITTED_ACTION_LIST YourKernelName;
    /// Then provide it to the API like so:
    ///     BindEmittedActionList(action_list_num, YourKernelName, Params);
    ///
    /// Setting ALFunc to NULL and Params to P_INTERNAL_CODE unbinds the compiled action list so future calls to CallActionList()
    /// are executed internally in the traditional way.
    void BindEmittedActionList(const int action_list_num,             ///< the action list number returned from a call to GenActionLists().
                               P_PARTICLE_EMITTED_ACTION_LIST ALFunc, ///< Pointer to the function that executes the compiled action list
                               const EmitCodeParams_e Params          ///< Tells what kind of code this is; currently host CPU C++ code or GPU CUDA code
    );

protected:
    std::shared_ptr<PInternalState_t> PS;                       // The internal API data for this context is stored here.
    void InternalSetup(std::shared_ptr<PInternalState_t> Sr);   // Calls this after construction to set up the PS pointer
    std::shared_ptr<PInternalState_t> getInternalState() const; // Return a pointer to the internal state
};

/// This class contains the API calls that operate on particle groups.
///
/// A particle group is first created using GenParticleGroups(), which will create a sequentially-numbered
/// set of particle groups and return the identifying number of the first generated
/// particle group. You specify which group is current using CurrentGroup(). Unless otherwise
/// stated, all other commands operate on the current particle group. The maximum number
/// of particles in the group is specified using SetMaxParticles(). The particle group
/// is then acted upon using the Actions.
///
/// After the actions have been applied, the particles are rendered. This is done at
/// the same stage of the application's execution as drawing other geometry. To draw
/// a particle group in OpenGL, the application calls GetParticles() or GetParticlePointer()
/// functions to get the vertex data, then sends it to OpenGL. When a particle group
/// is no longer needed, it is deleted using DeleteParticleGroups().
class PContextParticleGroup_t {
public:
    /// Copy particles from the specified group into the current group.
    ///
    /// Copy particles from the specified particle group, p_src_group_num, to the current particle group. Only copy_count particles, starting
    /// with number index, are copied. Of course, the number of particles actually copied is bounded by the available space in the current
    /// particle group, and the number of particles actually in the source particle group. The particles are added, in sequential order, to the
    /// end of the current group. The current group's BirthCallback(), if any, is called for each particle added to the list.
    void CopyGroup(const int p_src_group_num, ///< group number of the source particle group
                   const size_t index,        ///< index of the first particle in the source list to copy
                   const size_t copy_count    ///< copy at most this many particles
    );

    /// Change which group is current.
    ///
    /// Makes p_group_num be the current particle group to which all actions and commands apply.
    void CurrentGroup(const int p_group_num);

    /// Delete one or more consecutive particle groups.
    ///
    /// Deletes p_group_count particle groups, with p_group_num being the particle group number of the first one. The groups must be numbered
    /// sequentially, and must all exist. This removes the specified particle groups from existence (and all their particles). It does not merely
    /// change the number of existing particles or the maximum size of the group.
    ///
    /// The DeathCallback is NOT called for the particles in the deleted groups. Should I change this?
    void DeleteParticleGroups(const int p_group_num,      ///< handle of the first particle group to delete
                              const int p_group_count = 1 ///< delete this many groups
    );

    /// Create particle groups, each with a maximum of max_particles.
    ///
    /// Generates p_group_count new particle groups and returns the particle group number of the first one. The groups are numbered sequentially,
    /// beginning with the number returned. Each particle group is set to have at most max_particles particles. Call SetMaxParticles() to change this.
    /// Particle group numbers of groups that have been deleted (using DeleteParticleGroups()) might be reused by GenParticleGroups().
    int GenParticleGroups(const int p_group_count = 1,   ///< generate this many groups
                          const size_t max_particles = 0 ///< each created group can have this many particles
    );

    /// Returns the number of particles existing in the current group.
    ///
    /// The number returned is less than or equal to the group's max_particles.
    size_t GetGroupCount();

    /// Return the maximum number of particles allowed in the current group.
    ///
    /// This can be changed with SetMaxParticles().
    size_t GetMaxParticles();

    /// Copy particles from the current group to application memory.
    ///
    /// Copies at most count particles beginning with the index-th particle in the current particle group into memory already allocated by the
    /// application. Three floats are returned for the position of each particle, representing its x,y,z location. Four floats are returned for
    /// the color of each particle, representing its R,G,B,A color. Three floats are returned for the velocity of each particle, representing
    /// its dx,dy,dz direction vector. Three floats are returned for the size of each particle, representing whatever the application wants them
    /// to. One float is returned for the age of each particle.
    ///
    /// GetParticles() returns the number of particles copied to application memory. Of course, the number of particles actually returned is
    /// bounded by count and by the number of particles actually in the particle group minus index.
    /// If verts, color, vel, size or age is NULL then the respective field will not be returned. index and count must be at least 0 and less
    /// than the number of particles. index + count must be less than the number of particles.
    /// As with all arrays in C, the index of the first particle is zero.
    ///
    /// The following code gets the position of all particles:
    ///
    /// int cnt = GetGroupCount();
    /// float *ppos = new float[cnt * 3];
    /// int num_ret = GetParticles(0, cnt, ppos);
    size_t GetParticles(const size_t index,  ///< index of the first particle to return
                        const size_t count,  ///< max number of particles to return
                        float* position,     ///< location to store 3 floats per particle for position
                        const bool getAlpha, ///< true to get a float4 color with alpha; false to get a float3 color
                        float* color = NULL, ///< location to store 3 or 4 floats per particle for color and optionally alpha
                        float* vel = NULL,   ///< location to store 3 floats per particle for velocity
                        float* size = NULL,  ///< location to store 3 floats per particle for size
                        float* age = NULL    ///< location to store 1 float per particle for age
    );

    /// Return a pointer to particle data stored in API memory.
    ///
    /// This function exposes the internal storage of the particle data to the application. It provides a much higher performance way to render
    /// particles because it avoids copying. In fact, the returned pointers can typically be passed directly to OpenGL or D3D without the
    /// application ever owning a copy of the data.
    ///
    /// Writing to the returned memory is obviously unsafe. There may be auxiliary data that depend on the current values of the particle data.
    /// You can try it if you want to, but your code may break against future API versions.
    size_t GetParticlePointer(const float*& ptr, ///< the returned pointer to the particle data
                              size_t& stride,    ///< the number of floats from one particle's value to the next particle's value
                              size_t& pos3Ofs,   ///< the number of floats from returned ptr to the first particle's position parameter
                              size_t& posB3Ofs,  ///< the number of floats from returned ptr to the first particle's positionB parameter
                              size_t& size3Ofs,  ///< the number of floats from returned ptr to the first particle's size parameter
                              size_t& vel3Ofs,   ///< the number of floats from returned ptr to the first particle's velocity parameter
                              size_t& velB3Ofs,  ///< the number of floats from returned ptr to the first particle's velocityB parameter
                              size_t& color3Ofs, ///< the number of floats from returned ptr to the first particle's color parameter
                              size_t& alpha1Ofs, ///< the number of floats from returned ptr to the first particle's alpha parameter
                              size_t& age1Ofs,   ///< the number of floats from returned ptr to the first particle's age parameter
                              size_t& up3Ofs,    ///< the number of floats from returned ptr to the first particle's up parameter
                              size_t& rvel3Ofs,  ///< the number of floats from returned ptr to the first particle's rvel parameter
                              size_t& upB3Ofs,   ///< the number of floats from returned ptr to the first particle's upB parameter
                              size_t& mass1Ofs,  ///< the number of floats from returned ptr to the first particle's mass parameter
                              size_t& data1Ofs   ///< the number of floats from returned ptr to the first particle's data parameter, which is a 32-bit integer
    );

    /// Change the maximum number of particles in the current group.
    ///
    /// If necessary, this will delete particles from the end of the particle group, but no other particles will be deleted.
    /// The DeathCallback() of deleted particles WILL be called.
    /// Call SetMaxParticles(0) to empty the group.
    void SetMaxParticles(const size_t max_count);

    /// Specify a particle creation callback.
    ///
    /// Specify a callback function within your code that should be called every time a particle is created. The callback is associated only
    /// with the particle group that is current at the time you make the BirthCallback() call. You can optionally pass a 32-bit value
    /// that could be a handle to arbitrary data of your own, which is returned to your callback.
    ///
    /// The API's internal Particle_t struct is passed back to your callback function, so your application will have to include
    /// "ParticleLib/Particle.h", which it normally should not do.
    void BirthCallback(P_PARTICLE_CALLBACK callback, ///< Pointer to function of yours to call
                       pdata_t group_data = 0        ///< Arbitrary per-group data of yours to pass into your function
    );

    /// Specify a particle death callback.
    ///
    /// Specify a callback function within your code that should be called every time a particle is killed. The callback is associated only
    /// with the particle group that is current at the time you make the DeathCallback() call. You can optionally pass a pointer to arbitrary
    /// data of your own, which is returned to your callback.
    ///
    /// The API's internal Particle_t struct is passed back to your callback function, so your application will have to include
    /// "ParticleLib/Particle.h", which it normally should not do.
    void DeathCallback(P_PARTICLE_CALLBACK callback, ///< Pointer to function of yours to call
                       pdata_t group_data = 0        ///< Arbitrary per-group data of yours to pass into your function
    );

    /// Set the number of particles that fit in the CPU's cache
    ///
    /// You probably don't need to call this function. It is the number of bytes in the working set. Most action lists apply several actions to
    /// the working set of particles, then load the next working set of particles and apply the same actions to them. This allows particles to
    /// stay resident in the CPU's cache for a longer period of time, potentially increasing performance dramatically.
    ///
    /// You specify the working set size in bytes.
    void SetWorkingSetSize(const int set_size_bytes);

protected:
    std::shared_ptr<PInternalState_t> PS;                     // The internal API data for this context is stored here.
    void InternalSetup(std::shared_ptr<PInternalState_t> Sr); // Calls this after construction to set up the PS pointer
};

/// The inline actions API allows an action to be applied to a particle using a function call that will be inlined.
/// This enables a new programming model with a normally parallel for_each loop around a per-particle list of actions.
/// It also enables the inline actions functions to be called from CUDA threads, or equivalent, allowing GPU acceleration.
class PContextInlineActions_t {
public:
#define PARG Particle_t &m,
#include "Particle/pActionDecls.h"
#undef PARG

    void InternalSetup(std::shared_ptr<PInternalState_t> Sr); // Calls this after construction to set up the PS pointer

protected:
    std::shared_ptr<PInternalState_t> PS; // The internal API data for this context is stored here.
};

/// This class contains the legacy, less performant, but perhaps more elegant Action API.
///
/// Actions modify the position, color, velocity, size, age, and other attributes of
/// particles. All actions apply to the current particle group, as set by CurrentGroup().
/// Some actions will add particles to or delete them from the particle group, and others
/// will modify the particles in other ways. Typically, a series of actions will be
/// applied to each particle group once (or more) per rendered frame.
///
/// Remember that the amount of effect of an action call depends on the time step size,
/// dt, as set by TimeStep. See TimeStep() for an explanation of time steps.
///
/// Some functions have parameters with a default value of the constant P_EPS. P_EPS is a very small
/// floating point constant that is most often used as the default value of the epsilon
/// parameter to actions whose influence on a particle is relative to the inverse square
/// of its distance from something. If that distance is very small, the amount of influence
/// approaches infinity. Since all actions are computed using Euler's method, this can
/// cause unsatisfying results in which particles are accelerated way too much. So this
/// epsilon parameter is added to the distance before taking its inverse square, thus
/// keeping the acceleration within reasonable limits. By varying epsilon, you specify
/// what is reasonable. Larger epsilon make particles accelerate less.
class PContextActions_t {
public:
#define PARG
#include "Particle/pActionDecls.h"
#undef PARG

    /// Add particles with positions in the specified domain.
    ///
    /// Adds new particles to the current particle group. The particle positions are chosen from the given domain. All the other particle
    /// attributes such as color and velocity are chosen according to their current domains.
    ///
    /// When the Source action is called within an action list, the particle attribute domains used are those that were current when the Source
    /// command was called within the NewActionList() / EndActionList() block instead of when CallActionList() was called. Note that this is unlike OpenGL
    /// display lists.
    ///
    /// If particle_rate / dt is not an integer then Source() adjusts the number of particles to add during this time step so that the average
    /// number added per unit time is particle_rate.
    ///
    /// If too few particles seem to be added each frame, it is probably because the particle group is already full. If this is bad, you can
    /// grow the group using SetMaxParticles().
    void Source(const float particle_rate, ///< how many particles to add per unit time
                const pDomain& dom,        ///< particle positions are chosen from this domain
                const pSourceState& SrcSt  ///< all other particle attributes are chosen from this source state
    );

    /// Add a single particle at the specified location.
    ///
    /// This action mostly is a shorthand for Source(1, PDPoint(x, y, z)) but allows different callback data per particle.
    ///
    /// When called in immediate mode, this action uses a slightly faster method to add a single particle to the current particle group.
    /// Also when in immediate mode, exactly one particle will be added per call, instead of an average of 1 / dt particles being added.
    /// Particle attributes are chosen according to their current domains, as with Source.
    ///
    /// The user data attribute is an exception. It always takes the attribute from the optional data parameter to Vertex(), overriding the source state value.
    ///
    /// This call is patterned after the glVertex() calls. It is useful for creating a particle group with exactly specified initial positions.
    /// For example, you can specify a geometrical model using Vertex calls, and then explode or deform it.
    void Vertex(const pVec& v,             ///< position of particle to create
                const pSourceState& SrcSt, ///< all other particle attributes are chosen from this source state
                pdata_t data = 0           ///< application data to be passed to the birth and death callbacks
    );

    template <class UnaryFunction> void ParticleLoop(UnaryFunction f) { std::for_each(PGbegin(), PGend(), f); }
    template <class ExPol, class UnaryFunction> void ParticleLoop(ExPol&& policy, UnaryFunction f) { std::for_each(policy, PGbegin(), PGend(), f); }

protected:
    std::shared_ptr<PInternalState_t> PS;                     // The internal API data for this context is stored here.
    void InternalSetup(std::shared_ptr<PInternalState_t> Sr); // Calls this after construction to set up the PS pointer

    typedef std::vector<Particle_t> ParticleList;
    ParticleList::iterator PGbegin();
    ParticleList::iterator PGend();
};

/// The Particle System API - Your app should have one of these.
///
/// This is a complete instance of the Particle API. All API state is stored in the context.
///
/// See the documentation of the base classes for the description of all the API entry points.
class ParticleContext_t : public PContextParticleGroup_t, public PContextActionList_t, public PContextActions_t /* The legacy actions */ {
public:
    ParticleContext_t(); /// The context's default constructor

    PContextInlineActions_t I; /// The inline actions
};
}; // namespace PAPI

#endif
