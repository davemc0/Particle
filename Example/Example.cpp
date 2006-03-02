// Example.cpp - An example of the Particle System API in OpenGL.
//
// Copyright 1999 by David K. McAllister

#include <particle/papi.h>

#include <GL/glut.h>

// A fountain spraying up in the middle of the screen
void ComputeParticles()
{
	// Set up the state.
	pVelocityD(PDCylinder(pVec(0.0, -0.01, 0.25), pVec(0.0, -0.01, 0.27), 0.021, 0.019));
	pColorD(PDLine(pVec(0.8, 0.9, 1.0), pVec(1.0, 1.0, 1.0)));
	pSize(1.5);
	pStartingAge(0);

	// Generate particles along a very small line in the nozzle.
	pSource(100, PDLine(pVec(0.0, 0.0, 0.0), pVec(0.0, 0.0, 0.405)));

	// Gravity.
	pGravity(pVec(0.0, 0.0, -0.01));
	
	// Bounce particles off a disc of radius 5.
	pBounce(-0.05, 0.35, 0, PDDisc(pVec(0, 0, 0), pVec(0, 0, 1), 5));
	
	// Kill particles below Z=-3.
	pSink(false, PDPlane(pVec(0,0,-3), pVec(0,0,1)));

	// Move particles to their new positions.
	pMove();
}

// Draw as points using vertex arrays
// To draw as textured point sprites just call
// glEnable(GL_POINT_SPRITE_ARB) before calling this function.
void DrawGroupAsPoints()
{
	int cnt = (int)pGetGroupCount();
	if(cnt < 1) return;

	float *ptr;
    size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs;

    cnt = (int)pGetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
        size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs);
    if(cnt < 1) return;

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, int(flstride) * sizeof(float), ptr + color3Ofs);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, int(flstride) * sizeof(float), ptr + pos3Ofs);

    glDrawArrays(GL_POINTS, 0, cnt);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the view.
	glLoadIdentity();
	gluLookAt(0, -8, 3, 0, 0, 0, 0, 0, 1);
	
	// Draw the ground.
	glBegin(GL_QUADS);
	glColor3ub(0, 115, 0);
	glVertex3f(-3.5,-3.5,0);
	glColor3ub(0, 5, 140);
	glVertex3f(-3.5,3.5,0);
	glColor3ub(0, 5, 140);
	glVertex3f(3.5,3.5,0);
	glColor3ub(0, 115, 0);
	glVertex3f(3.5,-3.5,0);
	glEnd();
	
	// Do what the particles do.
	ComputeParticles();
	
	// Draw the particles.
	DrawGroupAsPoints();
	
	glutSwapBuffers();
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, w / double(h), 1, 100);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
	// Initialize GLUT.
	glutInit(&argc, argv);
	
	// Make a normal 3D window.
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Particle Example");
	
	glutDisplayFunc(Draw);
	glutIdleFunc(Draw);
	glutReshapeFunc(Reshape);
	
	// We want depth buffering, etc.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Make a particle group
	int particle_handle = pGenParticleGroups(1, 10000);
	
	pCurrentGroup(particle_handle);
	
	glutMainLoop();
	
	return 0;
}
