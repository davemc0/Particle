#ifndef DRAWGROUPS_H
#define DRAWGROUPS_H

#include "Particle/pAPI.h"

// Emit OpenGL calls to draw the particles as GL_LINES.
// The color is set per primitive or is constant.
// The other vertex of the line is the velocity vector.
// If len_scale > 0 then velocity vectors are scaled by len_scale.
// If len_scale < 0 then velocity vectors are normalized and then scaled by -len_scale.
// Setting len_scale to dt should give lines that roughly move by one line length per frame.
extern void DrawGroupAsLines(PAPI::ParticleContext_t& P, const bool const_color, const float len_scale = 1.0f);

// Draw each particle by translating, scaling, and rotating the display list
// to the position and orientation of the particle. Also sets the glColor
// before calling the display list for each particle.
extern void DrawGroupAsDisplayLists(PAPI::ParticleContext_t& P, int dlist, bool const_color, bool const_rotation);

// Draw each particle as a screen-aligned triangle with texture.
extern void DrawGroupAsTriSprites(PAPI::ParticleContext_t& P, const PAPI::pVec& view, const PAPI::pVec& up, float size_scale = 1.0f, bool draw_tex = false,
                                  bool const_size = false, bool const_color = false);

// Draw each particle as a screen-aligned quad with texture.
extern void DrawGroupAsQuadSprites(PAPI::ParticleContext_t& P, const PAPI::pVec& view, const PAPI::pVec& up, float size_scale = 1.0f, bool draw_tex = false,
                                   bool const_size = false, bool const_color = false);

// Draw as points using vertex arrays
// To draw as textured point sprites just call
// glEnable(GL_POINT_SPRITE) before calling this function.
extern void DrawGroupAsPoints(PAPI::ParticleContext_t& P, const bool const_color);

#endif
