// CUDA_Example_Gen.cpp - An example of generating a kernel function for a Particle System action list.
//
// Copyright 2008 by David K. McAllister

#include "Particle/pAPI.h"

#include <string>
#include <fstream>

using namespace std;
using namespace PAPI;

ParticleContext_t P;

// A fountain spraying up in the middle of the screen
void ComputeParticles()
{
    // Set up the state.
    pSourceState S;
    S.Velocity(PDCylinder_(pVec_(0.0f, -0.01f, 0.25f), pVec_(0.0f, -0.01f, 0.27f), 0.021f, 0.019f));
    S.Color(PDLine_(pVec_(0.8f, 0.9f, 1.0f), pVec_(1.0f, 1.0f, 1.0f)));

    // Generate particles along a very small line in the nozzle.
    P.Source(P_VARYING_FLOAT, PDLine_(pVec_(0, 0, 0), pVec_(0, 0, 0.4f)), S);

    // Gravity.
    P.Gravity(pVec_(0, 0, -0.01f));

    // Bounce particles off a disc of radius 5.
    P.Bounce(-0.05f, 0.35f, 0, PDDisc_(pVec_(0, 0, 0), pVec_(0, 0, 1), 5));

    // Move particles to their new positions.
    P.Move(true, false);

    // Kill particles below Z=-3.
    P.Sink(false, PDPlane_(pVec_(0,0,-3), pVec_(0,0,1)));
}

// Create the action list and convert it to a string and save it.
// Save the string to a .cu file.
void EmitActionListKernel()
{
    int AL = P.GenActionLists();

    P.NewActionList(AL);
    ComputeParticles();
    P.EndActionList();

    string Kernel, KernelName("Gen_Example");
    
    // Emit for CUDA
    P.EmitActionList(AL, Kernel, KernelName, P_CUDA_CODE);

    string fname2 = KernelName + ".cu";
    ofstream OutFile2(fname2.c_str());

    OutFile2 << Kernel;

    OutFile2.close();

    cout << "Emitted " << fname2 << endl;
    
    // Emit for CPU C++
    P.EmitActionList(AL, Kernel, KernelName, P_CPU_CPP_CODE);

    string fname = KernelName + ".cpp";
    ofstream OutFile(fname.c_str());

    OutFile << Kernel;

    OutFile.close();

    cout << "Emitted " << fname << endl;
}

int main(int argc, char **argv)
{
    // cerr << "sizeof(pVec)=" << sizeof(pVec) << " sizeof(pDomain)=" << sizeof(pDomain) << " sizeof(pSourceState)=" << sizeof(pSourceState) << endl;

    EmitActionListKernel();

    return 0;
}
