// opengl.cpp
//
// Copyright 1998 by David K. McAllister
//
// This file implements the API calls that draw particle groups in OpenGL.

#include "general.h"

#ifdef WIN32
// This is for something in gl.h.
#include <windows.h>
#endif

#include <GL/gl.h>

extern _ParticleState _ps;

// Emit OpenGL calls to draw the particles. These are drawn with
// whatever primitive type the user specified(GL_POINTS, for
// example). The color and radius are set per primitive, by default.
// For GL_LINES, the other vertex of the line is the velocity vector.
void pDrawGroupp(int primitive, bool const_size, bool const_color)
{
	// Get a pointer to the particles in gp memory
	ParticleGroup *pg = _ps.pgrp;
	if(pg == NULL)
		return; // ERROR
		
	if(pg->p_count < 1)
		return;

	//if(const_color)
	//	glColor4fv((GLfloat *)&pg->list[0].color);
	
	glBegin((GLenum)primitive);
	
	for(int i = 0; i < pg->p_count; i++)
	{
		Particle &m = pg->list[i];
		
		// Warning: this depends on alpha following color in the Particle struct.
		if(!const_color)
			glColor4fv((GLfloat *)&m.color);
		glVertex3fv((GLfloat *)&m.pos);
		
		// For lines, make a tail with the velocity vector's direction and
		// a length of radius.
		if(primitive == GL_LINES) {
			pVector tail(-m.vel.x, -m.vel.y, -m.vel.z);
			tail *= m.size;
			tail += m.pos;
			
			glVertex3fv((GLfloat *)&tail);
		}
	}
	
	glEnd();
}

void pDrawGroupl(int dlist, bool const_size, bool const_color, bool const_rotation)
{
	// Get a pointer to the particles in gp memory
	ParticleGroup *pg = _ps.pgrp;
	if(pg == NULL)
		return; // ERROR
	
	if(pg->p_count < 1)
		return;

	//if(const_color)
	//	glColor4fv((GLfloat *)&pg->list[0].color);

	for(int i = 0; i < pg->p_count; i++)
	{
		Particle &m = pg->list[i];

		glPushMatrix();
		glTranslatef(m.pos.x, m.pos.y, m.pos.z);

		if(!const_size)
			glScalef(m.size, m.size, m.size);

		// Expensive! A sqrt, cross prod and acos. Yow.
		if(!const_rotation)
		{
			pVector vN(m.vel);
			vN.normalize();
			pVector voN(m.velB);
			voN.normalize();

			pVector biN;
			if(voN.x == vN.x && voN.y == vN.y && voN.z == vN.z)
				biN = pVector(0, 1, 0);
			else
				biN = vN ^ voN;
			biN.normalize();

			pVector N(vN ^ biN);

			double M[16];
			M[0] = vN.x;  M[4] = biN.x;  M[8] = N.x;  M[12] = 0;
			M[1] = vN.y;  M[5] = biN.y;  M[9] = N.y;  M[13] = 0;
			M[2] = vN.z;  M[6] = biN.z;  M[10] = N.z; M[14] = 0;
			M[3] = 0;     M[7] = 0;      M[11] = 0;   M[15] = 1;
			glMultMatrixd(M);
		}

		// Warning: this depends on alpha following color in the Particle struct.
		if(!const_color)
			glColor4fv((GLfloat *)&m.color);

		glCallList(dlist);

		glPopMatrix();
	}
	
	glEnd();
}
