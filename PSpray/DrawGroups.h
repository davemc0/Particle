#ifndef DRAWGROUPS_H
#define DRAWGROUPS_H

#include <particle/pVector.h>

// Emit OpenGL calls to draw the particles as GL_LINES.
// The color is set per primitive or is constant.
// The other vertex of the line is the velocity vector.
extern void DrawGroupAsLines(bool const_color);

// Draw each particle by translating, scaling, and rotating the display list
// to the position and orientation of the particle. Also sets the glColor
// before calling the display list for each particle.
extern void DrawGroupAsDisplayLists(int dlist, bool const_color, bool const_rotation);

// Draw each particle as a screen-aligned triangle with texture.
extern void DrawGroupAsTriSprites(const pVector &view, const pVector &up,
								  float size_scale = 1.0f, bool draw_tex=false,
								  bool const_size=false, bool const_color=false);

// Draw each particle as a screen-aligned quad with texture.
extern void DrawGroupAsQuadSprites(const pVector &view, const pVector &up,
								   float size_scale = 1.0f, bool draw_tex=false,
								   bool const_size=false, bool const_color=false);

// Draw as points using vertex arrays
// To draw as textured point sprites just call
// glEnable(GL_POINT_SPRITE_ARB) before calling this function.
extern void DrawGroupAsPoints(const bool const_color);

#endif
