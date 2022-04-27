// PAPI.h
//
// Copyright 1997-2007, 2022 by David K. McAllister
//
// Include this file in all applications that use the Particle System API.

/*!
\mainpage The Particle System API

\section intro Introduction
The Particle System API (Application Programmer Interface) allows C++ programs to simulate the dynamics of particles. It is intended for special effects in
interactive and non-interactive graphics applications, not for scientific simulation, although principles of Newtonian physics have been used to implement the
particle dynamics where applicable.

The API consists of four sets of functions:
- <b>Particle Group Calls</b> - A particle group is a system of particles that are all acted on together. It can be considered a dynamic geometric
object. From the point of view of the graphics system, a particle group is often treated as just another model to be drawn. These calls create and destroy
particle groups, etc. Details are in PAPI::PContextParticleGroup_t.
- <b>Source State Calls</b> - As in OpenGL, some calls actually do something and some calls modify the behavior of the subsequent calls that do something.
The source state setting calls set the attributes of particles that will be created with the PAPI::PContextActions_t::Source or PAPI::PContextActions_t::Vertex
calls. The source state calls are in PAPI::pSourceState.
- <b>Actions</b> - These implement the particle dynamics. Bounce and Gravity are examples of actions. The Actions are in the class PAPI::PContextActions_t.
- <b>Action List Calls</b> - These create and operate on lists of actions. These are not needed by the new \ref inline_actions API.

Following are several basic concepts central to the Particle System API.

\subsection particles Particles

For the purposes of the Particle System API, a particle is not just a very tiny speck. Particles can be drawn as anything - water droplets, birds,
tumbleweeds, boulders, etc.. The Particle System API is generally useful for operating on many similar objects that all move and change according to the same
basic rules, no matter what the objects and rules are.

A particle in the abstract sense used by the API is merely an entity with a small set of
attributes such as position and color that ultimately dictate the particle's behavior and appearance. Particles are declared in pParticle.h or \ref
PAPI::Particle_t. Here are the particle attributes:

<table border="1"><tr><td>Position </td><td>location of the particle center </td><td>3 floats</td></tr><tr><td>Color
</td><td>color (in any color space) </td><td>3 floats </td></tr><tr><td>Velocity </td><td>velocity vector (direction and speed) </td><td>3 floats
</td></tr><tr><td>Up </td><td>up vector (together with velocity vector, yields an orientation frame) </td><td>3 floats </td></tr><tr><td>Rotational Velocity
</td><td>direction of rotation of the up vector </td><td>3 floats </td></tr><tr><td>Size </td><td>how large the particle is for rendering (doesn't affect
particle dynamics or collision) </td><td>3 floats </td></tr><tr><td>PositionB </td><td>initial or target position for returning to (see Restore()) </td><td>3
floats </td></tr><tr><td>UpB </td><td>initial or target orientation for returning to (see Restore()) </td><td>3 floats </td></tr><tr><td>VelocityB </td><td>can
be used as velocity from last frame for computing a side vector </td><td>3 floats </td></tr><tr><td>Mass </td><td>how large the particle is for dynamics
computations (doesn't affect collision location) </td><td>1 float </td></tr><tr><td>Age </td><td>time since the particle's creation </td><td>1 float
</td></tr><tr><td>Alpha </td><td>opacity, or a fourth color channel </td><td>1 float </td></tr><tr><td>Tmp </td><td>for a sorting key or for tagging particles
to kill by setting to P_MAXFLOAT </td><td>1 float </td></tr><tr><td>Data </td><td>user data to be passed back to user callbacks </td><td>1 uint32
</td></tr></table>

\subsection actions Actions

Actions are the core of the Particle System API. They are the functions in the API that directly manipulate particles in particle groups. They perform
effects such as gravity, explosions, bouncing, etc. to all particles in the current particle group. Actions modify the position, color, velocity, size, age, and
other attributes of particles. A program typically creates and initializes one or more particle groups, then during each frame it calls particle actions to
animate the particles then gets the particle data and draws the group of particles onto the screen.

All actions apply to the current particle group, as set by
CurrentGroup. Some actions will add particles to or delete them from the particle group, and others will modify the particles in other ways. Typically, a series
of actions will be applied to each particle group once (or more) per rendered frame.

The Particle System API uses a discrete time approximation to all actions. The amount of effect of an action call depends on the
time step size, dt, as set by \ref PAPI::PContextActionList_t::TimeStep. A smaller time step improves simulation quality.

Some functions have parameters with a default value of P_EPS. P_EPS is a very small floating point constant
that is most often used as the default value of the epsilon parameter to actions whose influence on a particle is relative to the inverse square of its distance
from something. If that distance is very small, the amount of influence approaches infinity. Since all actions are computed using Euler's method, this can cause
unsatisfying results in which particles are accelerated way too much. So this epsilon parameter is added to the distance before taking its inverse square, thus
keeping the acceleration within reasonable limits. By varying epsilon, you specify what is reasonable. Larger epsilon make particles accelerate less.

The Actions are in the class PAPI::PContextActions_t.

\subsection domains Domains
A Domain is a representation of a region of 3D space. For example, the Source action uses a domain to describe the volume in which a particle's random
initial position will be. The Avoid(), Sink() and Bounce() actions use domains to describe a volume in space for particles to steer to avoid, die when they
enter, or to bounce off, respectively. Domains are used as parameters to many functions within the API.

See PAPI::pDomain and pDomain.h for more.

\subsection sourcestate Source State Setting Calls
These calls dictate the properties of particles to be created by Source or Vertex. When particles are created within a NewActionList / EndActionList block,
they will receive attributes from the state that was current when the action list was created. When in immediate mode (not creating or calling an action list),
particles are created with attributes from the current state.

See the Source State Setting Calls in the class PAPI::pSourceState.

\subsection pgroup Particle Group Calls

A particle group is first created using PAPI::PContextParticleGroup_t::GenParticleGroups, which will create a sequentially-numbered set of particle groups and
return the identifying number of the first generated particle group. You specify which group is current using PAPI::PContextParticleGroup_t::CurrentGroup.
Unless otherwise stated, all other commands operate on the current particle group. The maximum number of particles in the group is specified using
PAPI::PContextParticleGroup_t::SetMaxParticles. The particle group is then acted upon using the Actions (PAPI::PContextActions_t).

After the actions have been applied, the particles are rendered. This is done at the same stage of the application's execution as drawing other geometry. To
draw a particle group in a graphics API, the application calls PAPI::PContextParticleGroup_t::GetParticles or PAPI::PContextParticleGroup_t::GetParticlePointer
functions to get the vertex data, then sends it to the graphics API. When a particle group is no longer needed, it is deleted using
PAPI::PContextParticleGroup_t::DeleteParticleGroups.

See the particle group calls in PAPI::PContextParticleGroup_t.

\subsection alists Action Lists
Action lists are blocks of actions that are applied together to a particle group. An action list is conceptually similar to a script or procedure. Action
lists are implemented and expressed similarly to display lists in OpenGL. An action list encapsulates the specifics of a particular effect and allows complex
effects to be treated as primitives like actions. An action list can be called from within another action list.

When using the \ref legacy_actions API, rather than the more performant \ref inline_actions API, action lists also allow effects to be simulated
much more efficiently. This is because the entire set of actions is known at the start, and this knowledge can be used by the implementation of the Particle
System API. For example, several actions can be applied to one set of particles before moving to the next set of particles, yielding better memory access
patterns. Also, in future versions common sequences of actions might be detected and replaced by efficient code that handles all those actions in one pass.

To take advantage of the up to 50% performance increase of action lists, you need to order your actions carefully. The performance advantage is achieved when
two or more actions that can be combined are ordered next to each other in the list. Actions that create (Vertex(), Source()) or delete (Sink*(), and
KillOld()) particles, and actions that reorder particles (Sort()) cannot be combined. All other actions can be combined to yield the performance advantage. See
Effects.cpp for several examples of efficient ordering.

Action List Calls create and operate on action lists. An empty
action list is first created using PAPI::PContextActionList_t::GenActionLists, and is then filled or compiled by calling
PAPI::PContextActionList_t::NewActionList, then calling Actions, then calling PAPI::PContextActionList_t::EndActionList. Once the action list is filled, it is
run via PAPI::PContextActionList_t::CallActionList. Thus, an action list is sort of a higher-level action. Complex behaviors can be stored in an action list and
then called later, even as part of another action list. Action lists cannot be edited. They can only be created or destroyed. To destroy an action list, call
PAPI::PContextActionList_t::DeleteActionLists. When particles are created within a NewActionList / EndActionList block, they will receive attributes from the
state that was current when the action list was created.

When in immediate mode (not creating or calling an action list), particles are created with attributes
from the current state. However, the time step length, dt, uses the value that is current when CallActionList is executed, not the value of dt when the action
list was created. This allows dt to be modified without recompiling action lists.

See the Action List Calls in the class PAPI::PContextActionList_t.

\subsection defparams Default Parameters
Many functions in the Particle System API take arguments with default values. This is a feature of C++. For example, in \code
Color(float r, float g, float b, float alpha = 1.0f) \endcode,
the parameter alpha has a default value of 1.0. This means that Color can be called with either three parameters or four. When
called with three parameters, they are the values of r, g, and b; and alpha receives the default value, 1.0. Only parameters at the end of the parameter list
can have default values. Likewise, when calling functions that have default values for parameters, all values specified will be applied to parameters starting
with the left. This means that there is no way to specify a value for a parameter at the end of the list without specifying values for all parameters to its
left.

\subsection except Exception Handling
See pError.h for the list of exceptions thrown by the API.

\section other Other Stuff
- Change log is <a href="../../ChangeLog.html">[here]</a>.
- Future work is <a href="../../ToDo.html">[here]</a>.
- Project is hosted <a href="https://github.com/davemc0/Particle">[here]</a>.

\subsection acks Acknowledgements
- Code for some of the particle actions and several concepts regarding the structure of the API are thanks to Jonathan P. Leech. See also:
Jonathan Leech and Russell M. Taylor II, "Interactive Modeling Using Particle Systems", Proceedings of the 2nd Conference on Discrete Element Methods,
MIT, spring 1993, pp. 105-116.
- Thanks to Mark B. Allen of NASA Ames Research Center for finding bugs, making suggestions and implementing the particle
length attribute.
- Thanks to Jason Pratt of CMU Stage 3 research group (makers of ALICE) for adding the PDTriangle domain. This is a powerful feature
that should have been there the whole time.
*/

#ifndef particle_api_h
#define particle_api_h

#include "Particle/pAPIContext.h"

// Only application code should include this. It contains the definitions of the inline action API functions.
#include "Particle/pInlineActionsAPI.h"

#endif
