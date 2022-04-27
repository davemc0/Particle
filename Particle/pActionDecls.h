/// Steer particles away from a domain of space.
///
/// Particles are tested to see whether they will pass from being outside the specified domain to being inside it within look_ahead time
/// units from now if the next Move() action were to occur now. The specific direction and amount of turn is dependent on the kind of
/// domain being avoided.
///
/// At present the only domains for which Avoid() is implemented are PDSphere, PDRectangle, PDTriangle, PDDisc and PDPlane.
void Avoid(PARG const float magnitude, ///< how drastically the particle velocities are modified to avoid the obstacle at each time step.
           const float epsilon,        ///< added to distance to dampen acceleration
           const float look_ahead,     ///< how far forward along the velocity vector to look for the obstacle
           const pDomain& dom          ///< the space to avoid
);

/// Bounce particles off an object defined by a domain.
///
/// Particles are tested to see whether they will pass from being outside the specified domain to being inside it if the next Move()
/// action were to occur now. If they would pass through the surface of the domain, they are instead bounced off it. That is, their
/// velocity vector is decomposed into components normal to the surface and tangent to the surface. The direction of the normal component
/// is reversed, and friction, resilience and fric_min_vel are applied to the components. They are then recomposed into a new velocity heading
/// away from the surface.
///
/// Since particles are tested to see whether they would pass through the domain if Move() were called now, it is best to have Bounce()
/// be the last action that modifies a particle's velocity before calling Move().
/// Also, actions such as RandomDisplace() that modify a particle's position directly, rather than modifying its velocity vector, may yield
/// unsatisfying results when used with Bounce().
///
/// At present Bounce() is not implemented for all domains. Feel free to implement others. For spheres, the particle bounces off either the inside or the
/// outside of the sphere. For planes, triangles and discs, the particles bounce off either side of the surface. For rectangles, particles bounce off either
/// side of the diamond-shaped patch whose corners are o, o+u, o+u+v, and o+v. See the documentation on domains for further explanation.
///
/// Bounce() doesn't work correctly with small time step sizes for particles sliding along a surface, despite fric_min_vel. The friction and resilience
/// parameters should not be scaled by dt, since a bounce happens instantaneously. On the other hand, they should be scaled by dt because particles sliding
/// along a surface will hit more often if dt is smaller. Adjust these parameters manually when you change dt.
void Bounce(PARG const float friction, ///< tangential component of the outgoing velocity vector is scaled by (1 - friction)
            const float resilience,    ///< normal component of the outgoing velocity vector is scaled by resilience
            const float fric_min_vel,  ///< only apply friction if tangential velocity is greater than fric_min_vel so particles can glide smoothly
            const pDomain& dom         ///< bounce off the surface of this domain
);

/// Set the secondary position and velocity from current.
void CopyVertexB(PARG const bool copy_pos = true, ///< If true, sets particle's PositionB to the current position of that particle. This makes each
                                                  ///< particle remember this position so it can later return to it using the Restore() action.
                 const bool copy_vel = false      ///< If true, sets particle's velocityB to the current velocity of that particle. Compute particle orientation
                                                  ///< by copying velocity before other actions. Then velocity X velocityB yields a tangent vector.
);

/// Simulate air by dampening particle velocities.
///
/// If a particle's velocity magnitude is within min_vel and max_vel, then multiply each component of the velocity by the respective damping constant.
/// Typically, the three components of damping will have the same value.
///
/// There are no bounds on the damping constants. Thus, by giving values greater than 1.0 they may be used to speed up particles instead of slow them down.
void Damping(PARG const pVec& damping,        ///< component-wise multiply this vector by the velocity vector
             const float min_vel = 0.0f,      ///< only dampen if velocity magnitude is greater than min_vel
             const float max_vel = P_MAXFLOAT ///< only dampen if velocity magnitude is less than max_vel
);

/// Simulate air by dampening rotational velocities.
///
/// If a particle's rotational velocity magnitude is within min_vel and max_vel, then multiply each component of the rotational velocity by
/// the respective damping constant. Typically, the three components of damping will have the same value.
///
/// There are no bounds on the damping constants. Thus, by giving values greater than 1.0 they may be used to speed up particles instead of slow them down.
void RotDamping(PARG const pVec& damping,        ///< component-wise multiply this vector by the rotational velocity vector
                const float min_vel = 0.0f,      ///< only dampen if velocity magnitude is greater than min_vel
                const float max_vel = P_MAXFLOAT ///< only dampen if velocity magnitude is less than max_vel
);

/// Exert force on each particle away from explosion center.
///
/// Causes an explosion by accelerating all particles away from the center. Particles are accelerated away from the center by an amount proportional to
/// magnitude. The shock wave of the explosion has a gaussian magnitude. The peak of the wave front travels spherically outward from the center at the
/// specified velocity. So at a given time step, particles at a distance (velocity * age) from center will receive the most acceleration, and particles not
/// at the peak of the shock wave will receive a lesser outward acceleration.
///
/// radius is the current radius of the explosion wave's peak. It is up to the application to increment the radius for each call to
/// Explosion(). For Explosion() calls in action lists, this means you will need to recreate the action list each time step.
///
/// You can set up a standing wave by not incrementing the radius.
void Explosion(PARG const pVec& center,    ///< center point of shock wave
               const float radius,         ///< current radius of wave peak
               const float magnitude,      ///< scales the acceleration applied to particles
               const float sigma,          ///< standard deviation of the gaussian; the sharpness or broadness of the strength of the wave.
               const float epsilon = P_EPS ///< added to distance to dampen acceleration
);

/// Accelerate particles in the given direction.
///
/// The gravity acceleration vector is simply added to the velocity vector of each particle at each time step. The magnitude of the
/// gravity vector is the acceleration due to gravity.
void Gravity(PARG const pVec& dir ///< acceleration vector
);

/// For particles in the domain of influence, accelerate them with a domain.
///
/// For each particle within the jet's domain of influence, dom, Jet() chooses an acceleration vector from the domain acc and applies
/// it to the particle's velocity.
void Jet(PARG const pDomain& dom, ///< apply jet to particles in this domain
         const pDomain& acc       ///< acceleration vector comes from this domain
);

/// Apply the particles' velocities to their positions, and age the particles.
///
/// This action actually updates the particle positions by adding the current velocity to the current position and the current rotational
/// velocity to the current up vector. This is typically the last particle action performed in an iteration of a particle simulation, and
/// typically only occurs once per iteration.
///
/// The velocity is multiplied by the time step length, dt, before being added to the position. This implements Euler's method of numerical
/// integration with a constant but specifiable step size. See TimeStep() for more on varying the time step size.
void Move(PARG const bool move_velocity = true,      ///< apply velocity to position.
          const bool move_rotational_velocity = true ///< apply rotational velocity to Up vector. This is an optimization.
);

/// Accelerate particles toward the closest point on the given line.
///
/// For each particle, this action computes the vector to the closest point on the line, and accelerates the particle in that direction.
void OrbitLine(PARG const pVec& p,                 ///< a point on the line
               const pVec& axis,                   ///< any vector parallel to the line
               const float magnitude = 1.0f,       ///< scales each particle's acceleration
               const float epsilon = P_EPS,        ///< added to distance to dampen acceleration
               const float max_radius = P_MAXFLOAT ///< no particle further than max_radius from the line is affected
);

/// Accelerate particles toward the given center point.
///
/// For each particle, this action computes the vector to the center point, and accelerates the particle in the vector direction.
void OrbitPoint(PARG const pVec& center,            ///< accelerate toward this point
                const float magnitude = 1.0f,       ///< scales each particle's acceleration
                const float epsilon = P_EPS,        ///< added to distance to dampen acceleration
                const float max_radius = P_MAXFLOAT ///< no particle further than max_radius from the center is affected
);

/// Accelerate particles in random directions.
///
/// For each particle, chooses an acceleration vector from the specified domain and adds it to the particle's velocity.
/// Reducing the time step, dt, will make a higher probability of being near the original velocity after unit time. Smaller dt approach
/// a normal distribution of velocity vectors instead of a square wave distribution.
void RandomAccel(PARG const pDomain& dom ///< choose acceleration from this domain
);

/// Immediately displace position by a random amount.
///
/// Chooses a displacement vector from the specified domain and adds it to the particle's position.
/// Reducing the time step, dt, will make a higher probability of being near the original position after unit time. Smaller dt approach a
/// normal distribution of particle positions instead of a square wave distribution.
///
/// Since this action changes particle positions, rather than changing their velocities and depending on the Move() action to change the
/// positions, unsatisfying results may occur when used with the Avoid() or Bounce() actions.
/// In particular, particles may be displaced to the opposite side of the surface without bouncing off it.
void RandomDisplace(PARG const pDomain& dom ///< choose position offset from this domain
);

/// Replace particle velocity with a random velocity.
///
/// Sets each particle's velocity vector to a random vector in the specified domain.
/// This function is not affected by dt.
void RandomVelocity(PARG const pDomain& dom ///< choose velocity from this domain
);

/// Immediately assign a random rotational velocity.
///
/// Sets each particle's rotational velocity vector to a random vector in the specified domain.
/// This function is not affected by dt.
void RandomRotVelocity(PARG const pDomain& dom ///< choose rotational velocity from this domain
);

/// Over time, restore particles to their target positionB and upB.
///
/// If vel is true, computes a new velocity for each particle that will make the particle arrive at its positionB at the specified amount
/// of time in the future. If rvel is true, computes a new rotational velocity that moves up toward upB.
///
/// The curved path that the particles take is a parametric quadratic. Once the specified amount of time has passed, Restore() instead sets
/// position and Up to equal positionB and upB and sets velocity and rotational velocity to 0 to freeze them in place.
///
/// It is the application's responsibility to decrease time_left by dt on each call. When in an action list, this means you need to
/// recreate the action list each time step.
///
/// The positionB attribute of each particle is typically the particle's position when it was created, or it can be specified within a
/// domain. This is controlled by VertexBTracks(), and VertexB(). The positionB can be set at any time to the particle's current position
/// using the CopyVertexB() action.
///
/// Restore(0) is the opposite of CopyVertexB(); it sets each particle's position to be equal to its positionB. However, this has the side
/// effect of setting each particle's velocity to 0.
void Restore(PARG const float time, ///< how long more until particles should arrive at target position and orientation
             const bool vel = true, ///< restore positions
             const bool rvel = true ///< restore up vectors
);

/// Clamp particle velocities to the given range.
///
/// Computes each particle's speed (the magnitude of its velocity vector) and if it is less than min_speed or greater than max_speed the
/// velocity is scaled to within those bounds, while preserving the velocity vector's direction.
///
/// The vector [0,0,0] is an exception because it has no direction. Such vectors are not modified by SpeedClamp().
void SpeedClamp(PARG const float min_speed, ///< set velocity vectors below min_speed to min_speed
                const float max_speed       ///< set velocity vectors above max_speed to max_speed
);

/// Change color of all particles toward the specified color.
///
/// Modifies the color and alpha of each particle to be scale percent of the way closer to the specified color and alpha. scale is
/// multiplied by dt before scaling the sizes. Thus, using smaller dt causes a slightly faster approach to the target color.
///
/// This action makes all colors tend toward the specified, uniform color.
/// The value of scale will usually be very small (less than 0.01) to yield a gradual transition.
void TargetColor(PARG const pVec& color, ///< target color
                 const float alpha,      ///< target alpha value
                 const float scale       ///< what percent of the way from the current color to the target color to transition in unit time
);

/// Change sizes of all particles toward the specified size.
///
/// Modifies the size of each particle to be scale percent of the way closer to the specified size triple. This makes sizes grow
/// asymptotically closer to the given size. scale is multiplied by dt before scaling the sizes. Thus, using smaller dt causes a slightly
/// faster approach to the target size. The separate scales for each component allow only selected components to be scaled.
///
/// This action makes all sizes tend toward the specified, uniform size. Future versions will have more actions that modify size.
/// Please send me suggestions (perhaps with sample implementations).
///
/// The value of scale will usually be very small (less than 0.01) to yield a gradual transition.
void TargetSize(PARG const pVec& size, ///< target size
                const pVec& scale      ///< what percent of the way from the current size to the target size to transition in unit time
);

/// Change velocity of all particles toward the specified velocity.
///
/// Modifies the velocity of each particle to be scale percent of the way closer to the specified velocity. This makes velocities
/// grow asymptotically closer to the given velocity. scale is multiplied by dt before scaling the velocities. Thus, using smaller
/// dt causes a slightly faster approach to the target velocity.
///
/// This action makes all velocities tend toward the specified, uniform velocity.
/// The value of scale will usually be very small (less than 0.01) to yield a gradual transition.
void TargetVelocity(PARG const pVec& vel, ///< target velocity
                    const float scale     ///< percent of the way from the current velocity to the target velocity to transition in unit time
);

/// Change rotational velocity of all particles toward the specified rotational velocity.
///
/// Modifies the rotational velocity of each particle to be scale percent of the way closer to the specified rotational velocity.
/// This makes rotational velocities grow asymptotically closer to the given rotational velocity. scale is multiplied by dt before
/// scaling the velocities. Thus, using smaller dt causes a slightly faster approach to the target rotational velocity.
///
/// This action makes all rotational velocities tend toward the specified, uniform rotational velocity.
/// The value of scale will usually be very small (less than 0.01) to yield a gradual transition.
void TargetRotVelocity(PARG const pVec& rvel, ///< rotational velocity
                       const float scale      ///< percent of the way from the current to the target rotational velocity to transition in unit time
);

/// Accelerate particles in a vortex-like way.
///
/// The vortex is a complicated action to use, but when done correctly it makes particles fly around like in a tornado.
void Vortex(PARG const pVec& tip,          ///< tip of the vortex
            const pVec& axis,              ///< the ray along the center of the vortex
            const float tightnessExponent, ///< exponent that curves the vortex silhouette; 1.0 is a cone; greater curves inward
            const float max_radius,        ///< no particle further than max_radius from the axis is affected
            const float inSpeed,           ///< inward acceleration of particles OUTSIDE the vortex
            const float upSpeed,           ///< vertical acceleration of particles INSIDE the vortex. Can be negative to apply gravity.
            const float aroundSpeed        ///< acceleration around vortex of particles INSIDE the vortex.
);

//////////////////////////////////////////////////////////////////
// Inter-particle actions

/// Accelerate toward the next particle in the list.
///
/// This allows snaky effects where the particles follow each other. Each particle is accelerated toward the next particle in the group.
/// The Follow() action does not affect the last particle in the group. This allows controlled effects where the last particle in the group
/// is killed after each time step and replaced by a new particle at a slightly different position. See KillOld() to learn how to kill
/// the last particle in the group after each step.
void Follow(PARG const float magnitude = 1.0f,  ///< scales each particle's acceleration
            const float epsilon = P_EPS,        ///< added to distance to dampen acceleration
            const float max_radius = P_MAXFLOAT ///< no particle further than max_radius from its predecessor is affected
);

/// Accelerate each particle toward each other particle.
///
/// Each particle is accelerated toward each other particle.
/// This action is more computationally intensive than the others are because each particle is affected by each other particle.
void Gravitate(PARG const float magnitude = 1.0f,  ///< scales each particle's acceleration
               const float epsilon = P_EPS,        ///< added to distance to dampen acceleration
               const float max_radius = P_MAXFLOAT ///< no particle further than max_radius from another particle is affected
);

/// Modify each particle's velocity to be similar to that of its neighbors.
///
/// Each particle is accelerated toward the weighted mean of the velocities of the other particles in the group.
///
/// Using an epsilon similar in size to magnitude can increase the range of influence of nearby particles on this particle.
void MatchVelocity(PARG const float magnitude = 1.0f,  ///< scales each particle's acceleration
                   const float epsilon = P_EPS,        ///< added to distance to dampen acceleration
                   const float max_radius = P_MAXFLOAT ///< no particle further than max_radius from another particle is affected
);

/// Modify each particle's rotational velocity to be similar to that of its neighbors.
///
/// Each particle is accelerated toward the weighted mean of the rotational velocities of the other particles in the group.
///
/// Using an epsilon similar in size to magnitude can increase the range of influence of nearby particles on this particle.
void MatchRotVelocity(PARG const float magnitude = 1.0f,  ///< scales each particle's acceleration
                      const float epsilon = P_EPS,        ///< added to distance to dampen acceleration
                      const float max_radius = P_MAXFLOAT ///< no particle further than max_radius from another particle is affected
);

//////////////////////////////////////////////////////////////////
// Other exceptional actions

/// Call an arbitrary user-provided function on each particle in the group.
///
/// The function will receive both your call data and the full Particle_t struct, which contains per-particle user data.
void Callback(PARG P_PARTICLE_CALLBACK_ACTION callbackFunc, ///< Pointer to function of yours to call.
              const pdata_t call_data = 0                   ///< Arbitrary data of yours to pass into your function
);

/// Get rid of older particles.
///
/// Removes all particles older than age_limit. But if kill_less_than is true, it instead removes all particles newer than age_limit.
/// age_limit is not clamped, so negative values are ok. This can be used in conjunction with StartingAge(-n) to create and then kill a particular set of
/// particles.
///
/// In order to kill a particular particle, set StartingAge() to a number that will never be a typical age for any other particle in the
/// group, for example -1.0. Then emit the particle using Source() or Vertex(). Then do the rest of the particle actions and finally call
/// KillOld(-0.9, true) to kill the special particle because it is the only one with an age less than -0.9.
void KillOld(PARG const float age_limit,       ///< max age of particles
             const bool kill_less_than = false ///< true to kill particles younger than age_limit instead of older
);

/// Kill particles that have positions on wrong side of the specified domain.
///
/// If kill_inside is true, deletes all particles inside the given domain. If kill_inside is false, deletes all particles outside the given domain.
void Sink(PARG const bool kill_inside, ///< true to kill particles inside the domain
          const pDomain& kill_pos_dom  ///< kill particles in this domain
);

/// Kill particles that have velocities on wrong side of the specified domain.
///
/// If kill_inside is true, deletes all particles whose velocity vectors are inside the given domain. If kill_inside is false, deletes all
/// particles whose velocity vectors are outside the given domain.
/// This allows particles to die when they turn around, get too fast or too slow, etc. For example, use a sphere domain centered at the
/// origin with a radius equal to the minimum velocity to kill particles that are too slow.
void SinkVelocity(PARG const bool kill_inside, ///< true to kill particles with velocities inside the domain
                  const pDomain& kill_vel_dom  ///< kill particles with velocities in this domain
);
