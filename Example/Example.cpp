// Example.cpp - An example of the Particle System API in OpenGL.
//
// Copyright 1999 by David K. McAllister

#include <particle/papi.h>

#include <GL/glut.h>

// A fountain spraying up in the middle of the screen
void ComputeParticles()
{
	// Set up the state.
	pVelocityD(PDCylinder, 0.01, 0.0, 0.35, 0.01, 0.0, 0.37, 0.021, 0.019);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);

	// Generate particles along a very small line in the nozzle.
	pSource(100, PDLine, 0.0, 0.0, 0.401, 0.0, 0.0, 0.405);

	// Gravity.
	pGravity(0.0, 0.0, -0.01);
	
	// Bounce particles off a disc of radius 5.
	pBounce(-0.05, 0.35, 0, PDDisc, 0, 0, 0,  0, 0, 1,  5);
	
	// Kill particles below Z=-3.
	pSink(false, PDPlane, 0,0,-3, 0,0,1);

	// Move particles to their new positions.
	pMove();
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
	pDrawGroupp(GL_LINES, true);
	
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
	int particle_handle = pGenParticleGroups(1, 4000);
	
	pCurrentGroup(particle_handle);
	
	glutMainLoop();
	
	return 0;
}
