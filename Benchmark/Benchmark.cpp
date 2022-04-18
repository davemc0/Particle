/// Benchmark.cpp
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// This application benchmarks particle system effects without doing graphics.

#include "../DemoShared/Effects.h"
#include "Particle/pAPI.h"

// The following header files are part of DMcTools.
#include "Math/Random.h"
#include "Util/StatTimer.h"
#include "Util/Timer.h"

#include <iostream>
#include <string>

namespace {
const int PRINT_PERIOD = 100, NUM_REPORTS = 5;
ExecMode_e ExecMode = Internal_Mode;
ParticleContext_t P;
EffectsManager Efx(P, 500000);
StatTimer FPSClock(PRINT_PERIOD);
bool SortParticles = false, ShowText = true;
int demoNum = -1;
} // namespace

void Report()
{
    static int FrameCountForClock = 0;
    if (++FrameCountForClock >= PRINT_PERIOD) {
        int cnt = (int)P.GetGroupCount();

        char exCh = (ExecMode == Immediate_Mode) ? 'I' : (ExecMode == Internal_Mode) ? 'N' : 'C';
        printf("%c%c n=%5d time=%02.4f %s\n", exCh, SortParticles ? 'S' : ' ', cnt, (float)FPSClock.GetMean(), Efx.GetCurEffectName().c_str());
        fflush(stdout);
        FrameCountForClock = 0;
    }
}

// Optimize the working set size
void RunBenchmarkCache()
{
    Efx.particleHandle = P.GenParticleGroups(1, Efx.maxParticles); // Make a particle group

    P.CurrentGroup(Efx.particleHandle);

    Efx.MakeActionLists(ExecMode);

    Efx.ChooseDemo(demoNum, ExecMode); // Prime it

    for (int CacheSize = 1024 * 16; CacheSize < 8 * 1024 * 1024; CacheSize += (16 * 1024)) {
        P.SetWorkingSetSize(CacheSize);
        for (int i = 0; i < 100; i++) {
            Efx.RunDemoFrame(ExecMode);
            if (SortParticles) P.Sort(pVec(0, -19, 15), pVec(0, 0, 3));
            FPSClock.Event();
            if (ShowText) Report();
        }
        printf("%d,%f\n", CacheSize, (float)FPSClock.GetMean());
    }
}

void RunBenchmark(int demoNum)
{
    Efx.particleHandle = P.GenParticleGroups(1, Efx.maxParticles); // Make a particle group

    P.CurrentGroup(Efx.particleHandle);

    Efx.MakeActionLists(ExecMode);

    Efx.ChooseDemo(demoNum, ExecMode);

    for (int i = 0; i < PRINT_PERIOD * NUM_REPORTS; i++) {
        Efx.RunDemoFrame(ExecMode);
        if (SortParticles) P.Sort(pVec(0, -19, 4), Efx.center);
        FPSClock.Event();
        Report();
    }

    P.DeleteParticleGroups(Efx.particleHandle);
}

// Test implementation of domains.
// Generate a particle and ensure that it is within the domain.
void TestOneDomain(pDomain& Dom)
{
    std::cerr << "TestOneDomain(" << typeid(Dom).name() << ")\n";
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

int findDemoByName(char* demoName)
{
    for (int d = 0; d < Efx.Effects.size(); d++) {
        if (Efx.Effects[d]->GetName() == demoName) return d;
    }
    EASSERT(0 && "Unknown demo name");
    return 0;
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
        } else if (starg == "-demo") {
            if (argc <= i + 1) Usage(program, "More args");
            if (strlen(argv[i + 1]) <= 2)
                demoNum = atoi(argv[i + 1]);
            else
                demoNum = findDemoByName(argv[i + 1]);

            RemoveArgs(argc, argv, i, 2);
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

    if (demoNum >= 0)
        RunBenchmark(demoNum);
    else
        for (demoNum = 0; demoNum < Efx.Effects.size(); demoNum++) { RunBenchmark(demoNum); }

    return 0;
}
