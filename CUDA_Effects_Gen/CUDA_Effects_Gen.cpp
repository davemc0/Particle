/// CUDA_Effects_Gen.cpp
///
/// Copyright 2009 by David K. McAllister
/// http://www.ParticleSystems.org
///
/// This application emits particle system effect kernels.

#include "../Effects/Effects.h"

#include "Particle/pAPI.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

inline string toStr(const int x)
{
    ostringstream S;
    S << x;
    return S.str();
}

void EmitEffectKernels()
{
    ParticleContext_t P;
    ParticleEffects Efx(P, 60000);
    Efx.particle_handle = P.GenParticleGroups(1, Efx.maxParticles);
    Efx.MakeEffects();

    for(int i=0; i<Efx.NumEffects; i++) {
        Efx.Effects[i]->EmitList(Efx);

        string EffectName = Efx.Effects[i]->GetName();
        string KernelName = "E" + toStr(i) + "_" + EffectName;
        string Kernel;

        {
            // Emit for CUDA
            P.EmitActionList(Efx.Effects[i]->AList, Kernel, KernelName, P_CUDA_CODE);
            string fname = KernelName + ".cu";
            ofstream OutFile(fname.c_str());
            OutFile << Kernel;
            OutFile.close();
            cout << "Emitted " << fname << endl;
        }

        {
            // Emit for CPU C++
            P.EmitActionList(Efx.Effects[i]->AList, Kernel, KernelName, P_CPU_CPP_CODE);
            string fname = KernelName + ".cpp";
            ofstream OutFile(fname.c_str());
            OutFile << Kernel;
            OutFile.close();
            cout << "Emitted " << fname << endl;
        }
    }
}

static void Usage(char *program_name, char *message)
{
    if (message)
        cerr << message << endl;

    cerr << "Usage: " << program_name << endl;
    exit(1);
}

static void Args(int argc, char **argv)
{
    char *program = argv[0];

    for(int i=1; i<argc; i++) {
        if(string(argv[i]) == "-h" || string(argv[i]) == "-help") {
            Usage(program, "Help:");
        } else {
            Usage(program, "Invalid option!");
        }
    }
}

int main(int argc, char **argv)
{
    Args(argc, argv);

    EmitEffectKernels();

    return 0;
}
