/// Benchmark.cpp
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// This application benchmarks particle system effects without doing graphics.

#include "../DemoShared/Effects.h"
#include "Particle/pAPI.h"

// The following header files are part of DMcTools.
#include "Math/Random.h"
#include "Util/Timer.h"

#include <iostream>
#include <string>

static ExecMode_e ExecMode = Internal_Mode;
static bool SortParticles = false, ShowText = true;
static int DemoNum = 6;

static Timer FPSClock;
static ParticleContext_t P;
static ParticleEffects Efx(P, 60000);

void Report()
{
#define NUM_FRAMES_TO_AVG_FOR_CLOCK 100
    static double ClockTime = 1.0;
    static int FrameCountForClock = 0;
    FrameCountForClock++;
    if (FrameCountForClock >= NUM_FRAMES_TO_AVG_FOR_CLOCK) {
        ClockTime = FPSClock.Reset();
        float fps = float(NUM_FRAMES_TO_AVG_FOR_CLOCK) / ClockTime;
        int cnt = (int)P.GetGroupCount();

        printf("%c%c n=%5d fps=%02.2f %s\n", ExecMode == Immediate_Mode ? 'I' : (ExecMode == Internal_Mode ? 'L' : 'C'), SortParticles ? 'S' : ' ', cnt, fps,
               Efx.GetCurEffectName().c_str());
        FPSClock.Start();
        FrameCountForClock = 0;
    }
}

// Optimize the working set size
// 3 MB works for Q6300.
void RunBenchmarkCache()
{
    Efx.particleHandle = P.GenParticleGroups(1, Efx.maxParticles); // Make a particle group

    P.CurrentGroup(Efx.particleHandle);

    Efx.MakeActionLists(ExecMode);

    Efx.CallDemo(DemoNum, ExecMode); // Prime it

    FPSClock.Start();
    for (int CacheSize = 1024 * 16; CacheSize < 8 * 1024 * 1024; CacheSize += (16 * 1024)) {
        P.SetWorkingSetSize(CacheSize);
        FPSClock.Reset();
        for (int i = 0; i < 100; i++) {
            Efx.CallDemo(DemoNum, ExecMode);
            if (SortParticles) P.Sort(pVec(0, -19, 15), pVec(0, 0, 3));
            if (ShowText) Report();
        }
        double t = FPSClock.Read();
        printf("%d,%f\n", CacheSize, t);
    }
}

void RunBenchmark()
{
    Efx.particleHandle = P.GenParticleGroups(1, Efx.maxParticles); // Make a particle group

    P.CurrentGroup(Efx.particleHandle);

    Efx.MakeActionLists(ExecMode);

    Efx.CallDemo(DemoNum, ExecMode);

    if (1) {
        for (int i = 0; i < 1000; i++) {
            Efx.CallDemo(DemoNum, ExecMode);
            if (SortParticles) P.Sort(pVec(0, -19, 15), pVec(0, 0, 3));
            Report();
        }
    } else {
        FPSClock.Reset();
        while (1) {
            float t = 0.0f;
            FPSClock.Start();
            for (int i = 0; i < 10000000; i++) {
                float v = pNRandf();
                t += v;
            }
            printf("%f\n", FPSClock.Read());
            FPSClock.Reset();
        }
    }
}

// Test implementation of domains.
// Generate a particle and ensure that it is within the domain.
void TestOneDomain(pDomain& Dom)
{
    std::cerr << "TestOneDomain()\n";
    const int Loops = 1000000;
    const float EP = 0.0000001;
    int Bad = 0;

    for (int i = 0; i < Loops; i++) {
        pVec pt = Dom.Generate();
        // std::cerr << pt << std::endl;
        bool isin = Dom.Within(pt);
        if (!isin) {
            if (0) {
                std::cerr << "Bad: (" << i << ") " << pt << ": ";
                std::cerr << Dom.Within(pt + pVec(EP, 0, 0));
                std::cerr << Dom.Within(pt + pVec(-EP, 0, 0));
                std::cerr << Dom.Within(pt + pVec(0, EP, 0));
                std::cerr << Dom.Within(pt + pVec(0, -EP, 0));
                std::cerr << Dom.Within(pt + pVec(0, 0, EP));
                std::cerr << Dom.Within(pt + pVec(0, 0, -EP));
                std::cerr << std::endl;
            }
            Bad++;
        }
    }
    std::cerr << Bad << " / " << Loops << " are bad.\n";
}

inline float RN() { return frand(-100, 100); }

void TestDomains()
{
    std::cerr << "Testing domains\n";

    const int OuterLoops = 1000;

    std::cerr << "PDPoint\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDPoint(pVec(RN(), RN(), RN())));
    std::cerr << "PDLine\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDLine(pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN())));
    std::cerr << "PDTriangle\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDTriangle(pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN())));
    std::cerr << "PDRectangle\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDRectangle(pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN())));
    std::cerr << "PDPlane\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDPlane(pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN())));
    std::cerr << "PDBox\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDBox(pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN())));
    std::cerr << "PDCylinder\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDCylinder(pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN()), fabs(RN()), fabs(RN())));
    std::cerr << "PDCone\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDCone(pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN()), fabs(RN()), fabs(RN())));
    std::cerr << "PDSphere\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDSphere(pVec(RN(), RN(), RN()), fabs(RN()), fabs(RN())));
    std::cerr << "PDBlob\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDBlob(pVec(RN(), RN(), RN()), fabs(RN())));
    std::cerr << "PDDisc\n";
    for (int i = 0; i < OuterLoops; i++) TestOneDomain(PDDisc(pVec(RN(), RN(), RN()), pVec(RN(), RN(), RN()), fabs(RN()), fabs(RN())));

    std::cerr << "Done testing domains.\n";
}

static void Usage(char* program_name, char* message)
{
    if (message) std::cerr << message << std::endl;

    std::cerr << "Usage: " << program_name << std::endl;
    exit(1);
}

static void Args(int argc, char** argv)
{
    char* program = argv[0];

    for (int i = 1; i < argc; i++) {
        std::string starg(argv[i]);

        if (starg == "-h" || starg == "-help") {
            Usage(program, "Help:");
        } else if (starg == "-cache") {
            RunBenchmarkCache();
            RemoveArgs(argc, argv, i);
        } else if (starg == "-test") {
            TestDomains();
            RemoveArgs(argc, argv, i);
        } else if (starg == "-immed") {
            ExecMode = Immediate_Mode;
            RemoveArgs(argc, argv, i);
        } else if (starg == "-compiled") {
            ExecMode = Compiled_Mode;
            RemoveArgs(argc, argv, i);
        } else if (starg == "-sort") {
            SortParticles = true;
            RemoveArgs(argc, argv, i);
        } else if (starg == "-print") {
            ShowText = true;
            RemoveArgs(argc, argv, i);
        } else {
            Usage(program, "Invalid option!");
        }
    }
}

int main(int argc, char** argv)
{
    Args(argc, argv);

    RunBenchmark();

    return 0;
}
