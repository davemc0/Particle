// CUDA_ExampleRun.cpp - An example of running a kernel function for a Particle System action list.
//
// Copyright 2008 by David K. McAllister

#include "Particle/pAPI.h"
using namespace PAPI;

#include "GL/glut.h"

ParticleContext_t P;
int ActionListNum;

// A fountain spraying up in the middle of the screen
void ComputeParticles()
{
    // Set up the state.
    pSourceState S;
    S.Velocity(PDCylinder_(pVec_(0.0f, -0.01f, 0.25f), pVec_(0.0f, -0.01f, 0.27f), 0.021f, 0.019f));
    S.Color(PDLine_(pVec_(0.8f, 0.9f, 1.0f), pVec_(1.0f, 1.0f, 1.0f)));

    // Generate particles along a very small line in the nozzle.
    P.Source(100, PDLine_(pVec_(0, 0, 0), pVec_(0, 0, 0.4f)), S);

    // Gravity.
    P.Gravity(pVec_(0, 0, -0.01f));

    // Bounce particles off a disc of radius 5.
    P.Bounce(-0.05f, 0.35f, 0, PDDisc_(pVec_(0, 0, 0), pVec_(0, 0, 1), 5));

    // Move particles to their new positions.
    P.Move(true, false);

    // Kill particles below Z=-3.
    P.Sink(false, PDPlane_(pVec_(0,0,-3), pVec_(0,0,1)));
}

// Create the action list and convert it to a string.
void PrepareActionListKernel()
{
    ActionListNum = P.GenActionLists();

    P.NewActionList(ActionListNum);
    ComputeParticles();
    P.EndActionList();

    // extern P_PARTICLE_EMITTED_ACTION_LIST Gen_Example;
    extern void Gen_Example(const void *A, void *P, const float dt, const int block_size);

    P.BindEmittedActionList(ActionListNum, Gen_Example, P_CPU_CPP_CODE);
}

// Draw as points using vertex arrays
// To draw as textured point sprites just call
// glEnable(GL_POINT_SPRITE_ARB) before calling this function.
void DrawGroupAsPoints()
{
    size_t cnt = P.GetGroupCount();
    if(cnt < 1) return;

    float *ptr;
    size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;

    cnt = P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
        size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
        up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);
    if(cnt < 1) return;

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
    P.CallActionList(ActionListNum);

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
    glutCreateWindow("CUDA Particle Example");

    glutDisplayFunc(Draw);
    glutIdleFunc(Draw);
    glutReshapeFunc(Reshape);

    // We want depth buffering, etc.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Get the action list ready
    PrepareActionListKernel();

    // Make a particle group
    int particle_handle = P.GenParticleGroups(1, 10000);

    P.CurrentGroup(particle_handle);

    glutMainLoop();

    return 0;
}
