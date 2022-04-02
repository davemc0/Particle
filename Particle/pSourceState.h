/// pSourceState.h
///
/// Copyright 1997-2009 by David K. McAllister
/// http://www.ParticleSystems.org
///
/// Defines these classes: pSourceState

#ifndef pSourceState_h
#define pSourceState_h

#include "pDomain.h"

namespace PAPI {

    /// These functions set the current state needed by Source() and Vertex() actions.
    ///
    /// These calls dictate the properties of particles to be created by Source() or Vertex().
    /// When particles are created within a NewActionList() / EndActionList() block, they will
    /// receive attributes from the state that was current when the action list was created (unlike OpenGL).
    /// When in immediate mode (not creating or calling an action list), particles are created
    /// with attributes from the current state.
    class pSourceState
    {
    public:
        pDomain Up_;
        pDomain Vel_;
        pDomain RotVel_;
        pDomain VertexB_;
        pDomain Size_;
        pDomain Color_;
        pDomain Alpha_;
        pdata_t Data_;
        float Age_;
        float AgeSigma_;
        float Mass_;
        bool vertexB_tracks_;

        PINLINE pSourceState() : Up_(PDPoint_(pVec(0,1,0))), Vel_(PDPoint_(pVec(0,0,0))), RotVel_(PDPoint_(pVec(0,0,0))),
            VertexB_(PDPoint_(pVec(0,0,0))), Size_(PDPoint_(pVec(1,1,1))), Color_(PDPoint_(pVec(1,1,1))), Alpha_(PDPoint_(pVec(1,1,1)))
        {
            Data_ = 0;
            Age_ = 0.0f;
            AgeSigma_ = 0.0f;
            Mass_ = 1.0f;
            vertexB_tracks_ = true;
        }

        /// Specify the color of particles to be created.
        ///
        /// This call is short-hand for Color(PDPoint(color), PDPoint(pVec(alpha)).
        ///
        /// The default color is 1,1,1,1 (opaque white if you interpret it as RGBA).
        PINLINE void Color(const pVec &color, const float alpha = 1.0f)
        {
            Color(PDPoint_(color), PDPoint_(pVec(alpha)));
        }

        /// Specify the color of particles to be created.
        ///
        /// This call is short-hand for Color(PDPoint(color), PDPoint(pVec(alpha)).
        ///
        /// The default color is 1,1,1,1 (opaque white if you interpret it as RGBA).
        PINLINE void Color(const float red, const float green, const float blue, const float alpha = 1.0f)
        {
            Color(pVec(red, green, blue), alpha);
        }

        /// Specify the domain for colors and alpha value of new particles.
        ///
        /// Your application can interpret the color triple in any color space you choose. RGB is the most common, with colors ranging on 0.0 -> 1.0.
        /// For example, the PDLine(pVec(1, 0, 0), pVec(1, 1, 0)) will choose points on a line between red and yellow. Points outside the 0.0 -> 1.0
        /// range will not be clamped by the Particle System API. Some renderers may use colors on the range 0 -> 255, so the domain used to choose
        /// the colors can be on that range. The alpha value is usually used for transparency.
        ///
        /// The particle color does not necessarily need to be used to represent color. It can be interpreted as an arbitrary three-vector.
        ///
        /// The default color is 1,1,1,1 (opaque white).
        PINLINE void Color(const pDomain &cdom) ///< The color domain.
        {
            Color_ = cdom;
            Alpha_ = PDPoint_(pVec(1));
        }

        /// Specify the domain for colors and alpha value of new particles.
        ///
        /// Your application can interpret the color triple in any color space you choose. RGB is the most common, with colors ranging on 0.0 -> 1.0.
        /// For example, the PDLine(pVec(1, 0, 0), pVec(1, 1, 0)) will choose points on a line between red and yellow. Points outside the 0.0 -> 1.0
        /// range will not be clamped by the Particle System API. Some renderers may use colors on the range 0 -> 255, so the domain used to choose
        /// the colors can be on that range. The alpha value is usually used for transparency.
        ///
        /// The particle color does not necessarily need to be used to represent color. It can be interpreted as an arbitrary three-vector.
        ///
        /// The default color is 1,1,1,1 (opaque white).
        PINLINE void Color(const pDomain &cdom, ///< The color domain.
            const pDomain &adom ///< The X dimension of the alpha domain is used for alpha.
            )
        {
            Color_ = cdom;
            Alpha_ = adom;
        }

        /// Specify the user data of particles to be created.
        ///
        /// All new particles will have the given user data value. This value could be cast from a pointer or could be any other useful value.
        ///
        /// The default user data is 0.
        PINLINE void Data(const pdata_t data)
        {
            Data_ = data;
        }

        /// Specify the size of particles to be created.
        ///
        /// This call is short-hand for Size(PDPoint(size)).
        ///
        /// The default size is 1,1,1.
        PINLINE void Size(const pVec &size)
        {
            Size_ = PDPoint_(size);
        }

        /// Specify the domain for the size of particles to be created.
        ///
        /// All new particles will have a size chosen randomly from within the specified domain. The size values may be negative.
        ///
        /// The size is not mass. It does not affect any particle dynamics, including acceleration and bouncing. It is merely a triple of rendering
        /// attributes, like color, and can be interpreted at the whim of the application programmer (that’s you). In particular, the three
        /// components do not need to be used together as three dimensions of the particle’s size. For example, one could be interpreted as radius,
        /// another as length, and another as density.
        ///
        /// The default size is 1,1,1.
        PINLINE void Size(const pDomain &dom)
        {
            Size_ = dom;
        }

        /// Specify the mass of particles to be created.
        ///
        /// The mass is used in the particle dynamics math, such as F=m*a. It doesn't affect size for bouncing, etc.
        ///
        /// The default mass is 1.
        PINLINE void Mass(const float mass)
        {
            Mass_ = mass;
        }


        /// Specify the initial rotational velocity vector of particles to be created.
        PINLINE void RotVelocity(const pVec &v)
        {
            RotVel_ = PDPoint_(v);
        }

        /// Specify the domain for the initial rotational velocity vector of particles to be created.
        ///
        /// For particles that will be rendered with complex shapes, like boulders, a rotation frame may be defined. The frame consists of the
        /// velocity vector, the Up vector, and the cross product of those, which you compute yourself.
        ///
        /// The default rotational velocity is 0,0,0.
        PINLINE void RotVelocity(const pDomain &dom)
        {
            RotVel_ = dom;
        }

        /// Specify the initial age of particles to be created.
        ///
        /// The age parameter can be positive, zero, or negative. Giving particles different starting ages allows KillOld() to distinguish between
        /// which to kill in interesting ways. Setting sigma to a non-zero value will give the particles an initial age with a normal distribution
        /// with mean age and standard deviation sigma. When many particles are created at once this allows a few particles to die at each time step,
        /// yielding a more natural effect.
        ///
        /// The default age is 0 and its sigma is 0.
        PINLINE void StartingAge(const float age, ///< mean starting age of particles
            const float sigma = 0.0f) ///< standard deviation of particle starting age
        {
            Age_ = age;
            AgeSigma_ = sigma;
        }

        /// Specify the initial up vector of particles to be created.
        ///
        /// This call is short-hand for UpVec(PDPoint(v)).
        ///
        /// The default Up vector is 0,1,0.
        PINLINE void UpVec(const pVec &up)
        {
            Up_ = PDPoint_(up);
        }

        /// Specify the domain for the initial up vector of particles to be created.
        ///
        /// For particles that will be rendered with complex shapes, like boulders, a rotation frame may be defined. The frame consists of the
        /// velocity vector, the Up vector, and the cross product of those, which you compute yourself.
        ///
        /// The default Up vector is 0,1,0.
        PINLINE void UpVec(const pDomain &dom)
        {
            Up_ = dom;
        }

        /// Specify the initial velocity vector of particles to be created.
        ///
        /// This call is short-hand for Velocity(PDPoint(vel)).
        ///
        /// The default Velocity vector is 0,0,0.
        PINLINE void Velocity(const pVec &vel)
        {
            Vel_ = PDPoint_(vel);
        }

        /// Specify the domain for the initial velocity vector of particles to be created.
        ///
        /// The default Velocity vector is 0,0,0.
        PINLINE void Velocity(const pDomain &dom)
        {
            Vel_ = dom;
        }

        /// Specify the initial secondary position of new particles.
        ///
        /// The PositionB attribute is used to store a destination position for the particle. This is designed for actions such as Restore().
        ///
        /// The default PositionB is 0,0,0.
        PINLINE void VertexB(const pVec &v)
        {
            VertexB_ = PDPoint_(v);
        }

        /// Specify the domain for the initial secondary position of new particles.
        ///
        /// The PositionB attribute is used to store a destination position for the particle. This is designed for actions such as Restore().
        ///
        /// The default PositionB is 0,0,0.
        PINLINE void VertexB(const pDomain &dom)
        {
            VertexB_ = dom;
        }

        /// Specify that the initial secondary position of new particles be the same as their position.
        ///
        /// If true, the PositionB attribute of new particles comes from their position, rather than from the VertexB domain.
        ///
        /// The default value of VertexBTracks is true.
        PINLINE void VertexBTracks(const bool track_vertex = true)
        {
            vertexB_tracks_ = track_vertex;
        }

        /// Reset all particle creation state to default values.
        ///
        /// All state set by the pSourceState functions will be reset.
        PINLINE void Reset()
        {
#ifndef __CUDACC__
            *this = pSourceState();
#endif
        }
    };

};

#endif
