/// ParBench.cpp
///
/// Copyright 2006-2007 by David K. McAllister
/// http://www.ParticleSystems.org
///
/// This application benchmarks particle system effects without doing graphics.

#include "../PSpray/Effects.h"

#include <Particle/pAPI.h>

// The following header files are part of DMcTools.
// DMcTools is part of the same source distribution as the Particle API.
#include <Util/Timer.h>
#include <Util/Utils.h>
#include <Util/Assert.h>

#include <iostream>
#include <string>
using namespace std;

#include <math.h>
#include <stdio.h>
#include <string.h>

static bool SortParticles = false, Immediate = false, ShowText = true;
static int DemoNum = 6;

static Timer Clock;
static ParticleContext_t P;
static ParticleEffects Efx(P, 30000);

void Report()
{
#define NUM_FRAMES_TO_AVG_FOR_CLOCK 30
    static double ClockTime = 1.0;
    static int FrameCountForClock = 0;
    FrameCountForClock++;
    if(FrameCountForClock >= NUM_FRAMES_TO_AVG_FOR_CLOCK)
    {
        ClockTime = Clock.Reset();
        float fps = float(NUM_FRAMES_TO_AVG_FOR_CLOCK) / ClockTime;
        int cnt = (int)P.GetGroupCount();

        printf("%c%c n=%5d fps=%02.2f %s\n",
            Immediate ? 'I':' ',
            SortParticles ? 'S':' ',
            cnt, fps, Efx.GetCurEffectName());
        Clock.Start();
        FrameCountForClock = 0;
    }
}

// Optimize the working set size
void RunBenchmarkCache()
{
    Efx.particle_handle = P.GenParticleGroups(1, Efx.maxParticles); // Make a particle group

    P.CurrentGroup(Efx.particle_handle);

    Efx.CallDemo(DemoNum, true, Immediate);

    Clock.Start();
    for(int CacheSize = 1024 * 8; CacheSize < 2 * 1024 * 1024; CacheSize += (8 * 1024)) {
        P.SetWorkingSetSize(CacheSize);
        Clock.Reset();
        for(int i=0; i<200; i++) {
            Efx.CallDemo(DemoNum, false, Immediate);
            if(SortParticles)
                P.Sort(pVec(0,-19,15), pVec(0,0,3));
            if(ShowText)
                Report();
        }
        double t = Clock.Read();
        printf("%d,%f\n", CacheSize, t);
    }
}

void RunBenchmark()
{
    Efx.particle_handle = P.GenParticleGroups(1, Efx.maxParticles); // Make a particle group

    P.CurrentGroup(Efx.particle_handle);

    Efx.CallDemo(DemoNum, true, Immediate);

#if 1
    for(int i=0; i<300; i++) {
        Efx.CallDemo(DemoNum, false, Immediate);
        if(SortParticles)
            P.Sort(pVec(0,-19,15), pVec(0,0,3));
        Report();
    }
#else
    Clock.Reset();
    while(1) {
        float t = 0.0f;
        Clock.Start();
        for(int i=0; i<10000000; i++) {
            float v = pNRandf();
            t += v;
        }
        printf("%f\n", Clock.Read());
        Clock.Reset();
    }
#endif
}

void TestOneDomain(pDomain &Dom)
{
    const int Loops = 1000000;
    const float EP = 0.0000001;
    int Bad = 0;

    for(int i=0; i<Loops; i++) {
        pVec pt = Dom.Generate();
        //cerr << pt << endl;
        bool isin = Dom.Within(pt);
        // ASSERT_R(isin);
        if(!isin) {
#if 0
            cerr << "Bad: (" << i << ") " << pt << ": ";
            cerr << Dom.Within(pt + pVec(EP,0,0));
            cerr << Dom.Within(pt + pVec(-EP,0,0));
            cerr << Dom.Within(pt + pVec(0,EP,0));
            cerr << Dom.Within(pt + pVec(0,-EP,0));
            cerr << Dom.Within(pt + pVec(0,0,EP));
            cerr << Dom.Within(pt + pVec(0,0,-EP));
            cerr << endl;
#endif
            Bad++;
        }
    }
    cerr << Bad << " / " << Loops << " are bad.\n";
}

float RN()
{
    return DRand(-100, 100);
}

void TestDomains()
{
    const int OuterLoops = 1000;

    cerr << "PDPoint\n";      for(int i=0; i<OuterLoops; i++) TestOneDomain(PDPoint(pVec(RN(),RN(),RN())));
    cerr << "PDLine\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDLine(pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN())));
    cerr << "PDTriangle\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDTriangle(pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN())));
    cerr << "PDRectangle\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDRectangle(pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN())));
    cerr << "PDPlane\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDPlane(pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN())));
    cerr << "PDBox\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDBox(pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN())));
    cerr << "PDCylinder\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDCylinder(pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN()), fabs(RN()), fabs(RN())));
    cerr << "PDCone\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDCone(pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN()), fabs(RN()), fabs(RN())));
    cerr << "PDSphere\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDSphere(pVec(RN(),RN(),RN()), fabs(RN()), fabs(RN())));
    cerr << "PDBlob\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDBlob(pVec(RN(),RN(),RN()), fabs(RN())));
    cerr << "PDDisc\n"; for(int i=0; i<OuterLoops; i++) TestOneDomain(PDDisc(pVec(RN(),RN(),RN()), pVec(RN(),RN(),RN()), fabs(RN()), fabs(RN())));

    cerr << "Done testing domains.\n";
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
        } else if(string(argv[i]) == "-list") {
            Immediate = false;
            RemoveArgs(argc, argv, i);
        } else if(string(argv[i]) == "-sort") {
            SortParticles = true;
            RemoveArgs(argc, argv, i);
        } else if(string(argv[i]) == "-print") {
            ShowText = true;
            RemoveArgs(argc, argv, i);
        } else {
            Usage(program, "Invalid option!");
        }
    }
}

int main(int argc, char **argv)
{
    Args(argc, argv);

    try {
        RunBenchmark();
        TestDomains();
    }
    catch (PError_t &Er) {
        cerr << "Particle API exception: " << Er.ErrMsg << endl;
        throw Er;
    }
    catch (...) {
        cerr << "Non-Particle-API exception caught. Bye.\n";
        throw;
    }

    return 0;
}
