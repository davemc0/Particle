// Example.cpp - An example of the Particle System API in OpenGL
//
// Copyright 1999-2006, 2022 by David K. McAllister

#include "CudaExample.h"

#include "Particle/pAPI.h"
using namespace PAPI;

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"

// For C++17 execution policy to get parallelism of particle actions
#include <execution>

ParticleContext_t P;

// Draw each particle as a point using vertex arrays
// To draw as textured point sprites just call glEnable(GL_POINT_SPRITE) before calling this function.
void DrawGroupAsPoints()
{
    size_t cnt = P.GetGroupCount();
    if (cnt < 1) return;

    const float* ptr;
    size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;

    cnt = P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs,
                               mass1Ofs, data1Ofs);
    if (cnt < 1) return;

    glEnable(GL_POINT_SMOOTH);
    glPointSize(4);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, int(flstride) * sizeof(float), ptr + color3Ofs);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, int(flstride) * sizeof(float), ptr + pos3Ofs);

    glDrawArrays(GL_POINTS, 0, (GLsizei)cnt);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void Draw()
{
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //
    // // Set up the view
    // glLoadIdentity();
    // gluLookAt(0, -12, 3, 0, 0, 0, 0, 0, 1);
    //
    // // Draw the ground
    // glColor3ub(0, 115, 0);
    // glPushMatrix();
    // glTranslatef(0, 0, -1);
    // glutSolidCylinder(5, 1, 20, 20);
    // glPopMatrix();
    //
    // // Do what the particles do
    // ComputeParticles();

    // Draw the particles
    // DrawGroupAsPoints();
    static int ii = 0;
    printf("yo %d\n", ii++);

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

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    // Make a standard 3D window
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Particle Example");

    glutDisplayFunc(Draw);
    glutIdleFunc(Draw);
    glutReshapeFunc(Reshape);

    // We want depth buffering, etc.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Make a particle group
    int particleHandle = P.GenParticleGroups(1, 50000);

    P.CurrentGroup(particleHandle);
    P.TimeStep(0.1f);

    try {
        glutMainLoop();
    }
    catch (PError_t& Er) {
        std::cerr << "Particle API exception: " << Er.ErrMsg << std::endl;
        throw Er;
    }

    return 0;
}
