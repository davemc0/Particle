// This file contains the code to emit the CUDA kernel for the action list.

// Handle the creation of a struct to pass all the parameters of the action list into the CUDA kernel.
// Most of these params will be ignored and their values hard coded at kernel emission time.
//
// Are there any varying parameters to worry about? If so, use something like this:
//
// Handle domains by separately defining their derived class as a simple struct with no virtual functions.
// Modify the action structs to specify by name the derived domain type.
// 
// nvcc -keep -use_fast_math --ptxas-options=-v -ccbin "c:\Program Files\Microsoft Visual Studio 9.0\VC\bin" -c -DWIN32 -D_CONSOLE -D_MBCS -Xcompiler /EHsc,/W3,/nologo,/O2,/Zi,/MT -I"C:\CUDA\include" -I./ -I../Particle -o test_cuda.obj test_cuda.cu

#include "CUDAGen.h"
#include "pAPI.h"

#include <sstream>

using namespace std;

namespace PAPI {

    namespace {

    string CatInt(const string &name, const int ac)
    {
        ostringstream S;
        S << name << ac;
        return S.str();
    }

        // We emit the action list two times:
        // 1) Create the kernel function. This emits literals for the non-varying vars and takes the varying vars from the cmd line and passes them to the action functions.
        // 2) We emit the host-side calling function, which copies the varying vars into a big array, then loads it into constant memory.

        // This macro calls the Emit3 function and passes in the name of the variable as well as its value.
#define EMIT(val, str) Emit3(val, str, #val, var_cnt, call)
#define EMITD(valhead, valtail, str) Emit3(valhead.valtail, str, #valtail, var_cnt, call)
#define NC(x) (!call ? x : "")

        inline string FloatLit(float v)
        {
            ostringstream S;
            float vr = float(int(v));
            S << v << (v==vr ? ".0" : "") << "f";

            return S.str();
        }

        string Emit3(float v, const string &pre, const string &vname, int &var_cnt, bool call)
        {
            ostringstream S;
            if(v == P_VARYING_FLOAT) // Emit the variable name since it's a varying parameter.
                if(call)
                    S << "    VD[" << var_cnt++ << "].f = " << pre << "." << vname << ";\n";
                else
                    S << "VD[" << var_cnt++ << "].f";
            else if(!call) {
                S << FloatLit(v);
            }

            return S.str();
        }

        string Emit3(int v, const string &pre, const string &vname, int &var_cnt, bool call)
        {
            ostringstream S;
            if(v == P_VARYING_INT) // Emit the variable name since it's a varying parameter.
                if(call)
                    S << "    VD[" << var_cnt++ << "].i = " << pre << "." << vname << ";\n";
                else
                    S << "VD[" << var_cnt++ << "].i";
            else if(!call)
                S << v;

            return S.str();
        }

        string Emit3(pdata_t v, const string &pre, const string &vname, int &var_cnt, bool call)
        {
            ostringstream S;
            if(v == P_VARYING_INT) // Emit the variable name since it's a varying parameter.
                if(call)
                    S << "    VD[" << var_cnt++ << "].d = " << pre << "." << vname << ";\n";
                else
                    S << "VD[" << var_cnt++ << "].d";
            else if(!call)
                S << v;

            return S.str();
        }

        string Emit3(bool v, const string &pre, const string &vname, int &var_cnt, bool call)
        {
            ostringstream S;
            // XXX Not sure how to have a magic number for bool.
            if(v == P_VARYING_BOOL) // Emit the variable name since it's a varying parameter.
                if(call)
                    S << "    VD[" << var_cnt++ << "].b = " << pre << "." << vname << ";\n";
                else
                    S << "VD[" << var_cnt++ << "].b";
            else if(!call)
                S << v;

            return S.str();
        }

        string Emit3(const pVec &v, const string &pre, const string &vname, int &var_cnt, bool call)
        {
            ostringstream S;
            S << NC("pVec_(");
            if(v.x() == P_VARYING_FLOAT)
                if(call)
                    S << "    VD[" << var_cnt++ << "].f = " << pre << "." << vname << ".x();\n";
                else
                    S << "VD[" << var_cnt++ << "].f";
            else if(!call)
                S << FloatLit(v.x());
            S << NC(", ");

            if(v.y() == P_VARYING_FLOAT)
                if(call)
                    S << "    VD[" << var_cnt++ << "].f = " << pre << "." << vname << ".y();\n";
                else
                    S << "VD[" << var_cnt++ << "].f";
            else if(!call)
                S << FloatLit(v.y());
            S << NC(", ");

            if(v.z() == P_VARYING_FLOAT)
                if(call)
                    S << "    VD[" << var_cnt++ << "].f = " << pre << "." << vname << ".z();\n";
                else
                    S << "VD[" << var_cnt++ << "].f";
            else if(!call)
                S << FloatLit(v.z());
            S << NC(")");

            return S.str();
        }


        string Emit3(const pDomain &dom, const string &pre, const string &vname, int &var_cnt, bool call)
        {
            string nm = pre + "." + vname;
            ostringstream S;

            // If ANY variable in here is varying then we have to emit in a way that will call the constructor and compute the dependent variables.
            if(!dom.Varying) {
                if(!call) {
                    S << "PDData_(pDomainType_E(" << dom.Which << ")";
                    for(int i=0; i<P_N_FLOATS_IN_DOMAIN; i++)
                        S << ", " << dom.PDData_V[i];
                    S << ")";
                }
            } else {
                switch(dom.Which) {
                case PDVarying_e:
#if 0
                    // Receive a non-constructed domain and construct it here
                    S << NC("PDFromVarying_(pDomainType_E(") << EMITD(dom, Which, nm) << NC("), ")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v2, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f1, nm) << NC(")");
#else
                    // Receive a constructed domain.
                    S << NC("PDData_(pDomainType_E(") << EMITD(dom, Which, nm) << NC(")");
                    for(int i=0; i<P_N_FLOATS_IN_DOMAIN; i++)
                        S << NC(", ") << Emit3(P_VARYING_INT, nm, CatInt("PDData_V[", i) + "]", var_cnt, call);
                    S << NC(")");
#endif
                    break;
                case PDUnion_e:
                    S << NC("PDUnion_(")
                        << NC("/* NOT IMPLEMENTED */") << NC(")");
                    break;
                case PDPoint_e:
                    S << NC("PDPoint_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(")");
                    break;
                case PDLine_e:
                    S << NC("PDLine_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(")");
                    break;
                case PDTriangle_e:
                    S << NC("PDTriangle_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v2, nm) << NC(")");
                    break;
                case PDRectangle_e: 
                    S << NC("PDRectangle_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v2, nm) << NC(")");
                    break;
                case PDDisc_e: 
                    S << NC("PDDisc_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f1, nm) << NC(")");
                    break;
                case PDPlane_e: 
                    S << NC("PDPlane_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(")");
                    break;
                case PDBox_e: 
                    S << NC("PDBox_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(")");
                    break;
                case PDCylinder_e: 
                    S << NC("PDCylinder_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f1, nm) << NC(")");
                    break;
                case PDCone_e: 
                    S << NC("PDCone_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.v1, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f1, nm) << NC(")");
                    break;
                case PDSphere_e: 
                    S << NC("PDSphere_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f1, nm) << NC(")");
                    break;
                case PDBlob_e: 
                    S << NC("PDBlob_(")
                        << EMITD(dom, PDRaw_V.v0, nm) << NC(", ")
                        << EMITD(dom, PDRaw_V.f0, nm) << NC(")");
                    break;
                default: S << NC("UNK_DOMAIN");
                    break;
                }
            }

            return S.str();
        }

        string Emit3(const pSourceState &SrcSt, const string &pre, const string &vname, int &var_cnt, bool call)
        {
            string nm = pre + "." + vname;
            ostringstream S;

            S << NC("    pSourceState SrcStt;\n")
                << NC("    SrcStt.Up_ = ") << EMITD(SrcSt, Up_, nm) << NC(";\n")
                << NC("    SrcStt.Vel_ = ") << EMITD(SrcSt, Vel_, nm) << NC(";\n")
                << NC("    SrcStt.RotVel_ = ") << EMITD(SrcSt, RotVel_, nm) << NC(";\n")
                << NC("    SrcStt.VertexB_ = ") << EMITD(SrcSt, VertexB_, nm) << NC(";\n")
                << NC("    SrcStt.Size_ = ") << EMITD(SrcSt, Size_, nm) << NC(";\n")
                << NC("    SrcStt.Color_ = ") << EMITD(SrcSt, Color_, nm) << NC(";\n")
                << NC("    SrcStt.Alpha_ = ") << EMITD(SrcSt, Alpha_, nm) << NC(";\n")
                << NC("    SrcStt.Data_ = ") << EMITD(SrcSt, Data_, nm) << NC(";\n")
                << NC("    SrcStt.Age_ = ") << EMITD(SrcSt, Age_, nm) << NC(";\n")
                << NC("    SrcStt.AgeSigma_ = ") << EMITD(SrcSt, AgeSigma_, nm) << NC(";\n")
                << NC("    SrcStt.Mass_ = ") << EMITD(SrcSt, Mass_, nm) << NC(";\n")
                << NC("    SrcStt.vertexB_tracks_ = ") << EMITD(SrcSt, vertexB_tracks_, nm) << NC(";\n");

            return S.str();
        }
    };

    // Create a name for this action list kernel by concatenating the abbreviatons of its actions.
    string GetKernelName(ActionList &AList)
    {
        string kname = "AList";
        for(ActionList::iterator it = AList.begin(); it != AList.end(); it++) {
            kname += '_' + (*it)->GetAbrv();
        }

        return kname;
    }

    // Create a struct that defines the parameters passed into this 
    string EmitActionListCaller(const ActionList &AList, const string &KernelName, const EmitCodeParams_e Params)
    {
        ostringstream caller0, caller1;

        caller0 << "///////////////////////////////////////////////////\n"
            << "// Host code for calling the kernels\n\n";
        caller0 << ((Params & P_CUDA_CODE) ? "__host__ " : "")
            << "static void " << KernelName << "Intl(const ActionList &AList, ParticleGroup &PG, const float dt, const int block_size)\n{\n";

        caller0 << "    // Fill in the constant array\n";
        int var_cnt = 0;
        for(ActionList::const_iterator it = AList.begin(); it != AList.end(); it++) {
            // caller << "    " << typeid(**it).name() << " *AP = dynamic_cast<" << typeid(**it).name() << " *>(AList[" <<  << "]);\n";
            caller1 << (*it)->EmitCall("AList[" + CatInt("", (int)(it - AList.begin())) + "]", var_cnt, true);
        }

        if(var_cnt > 0) {
            caller0 << "    Dword_t VD[" << var_cnt << "];\n";
            caller0 << caller1.str() << endl;
        }

        if(Params & P_CUDA_CODE) {
            // Copy the constant array to the device
            // Both arrays are called VD since the same code emits the code to both read the constant array and write the host array.
            caller0 << "    // Copy constants to device\n";
            caller0 << "    cudaMemcpyToSymbol(::VD, VD, sizeof(Dword_t) * " << var_cnt << ");\n";

            caller0 << "    PG.update_host_from_device = true;\n\n";
        }

        caller0 << "    // Call the kernels\n";

        return caller0.str();
    }

    // Create the string representing the call to one kernel for a segment of an action list
    string EmitHostCaller(const ActionList &AList, const ActionList::iterator abeg, const ActionList::iterator aend,
        int ac, const string &KernelName, const int var_cnt_in, const EmitCodeParams_e Params)
    {
        ostringstream kernel;
        // kernel << "    // EmitHostCaller\n";

        if((*abeg)->GetName() == "PASource") {
            PASSERT(abeg + 1 == aend, "Only one PAsource per group");

            PASource *PAS = dynamic_cast<PASource*>(AList[ac]);
            int var_cnt = var_cnt_in - 1; // Either this variable gets ignored or it emits particle_rate, which was the most recent one emitted so subtract one to hit it again.

            kernel << "    {\n";
            kernel << "        float prate = " << Emit3(PAS->particle_rate, "", "blech", var_cnt, false) << ";\n";
            kernel << "        int nparticles = PG.GetList().size();\n";
            kernel << "        int nthreads = SourceQuantity(prate, dt, nparticles, PG.GetMaxParticles());\n\n";

            if(Params & P_CUDA_CODE) {
                kernel << "        int ngrids = (nthreads + block_size - 1) / block_size;\n";
                kernel << "        " << KernelName << "<<<ngrids, block_size>>>(PG.GetDevicePtr(), dt, nparticles);\n\n";

                // The particle array on the host is garbage but is the right size.
                kernel << "        PG.GetList().resize(nparticles + nthreads);\n";
            } else {
                kernel << "        PG.GetList().resize(nparticles + nthreads);\n";
                kernel << "        Particle_t *PartP = nthreads ? &(PG.GetList()[0]) : NULL;\n";
                // For CPU code let the convention be to pass an additional parameter with the particle index
                kernel << "        for(int i=0; i<nthreads; i++)\n";
                kernel << "            " << KernelName << "(PartP, i, VD, dt, nparticles);\n";
            }
            kernel << "    }\n";
        } else if((*abeg)->GetName() == "PASort") {
        } else if((*abeg)->GetName() == "PAFollow") {
        } else if((*abeg)->GetName() == "PAGravitate" || (*abeg)->GetName() == "PAMatchVelocity" || (*abeg)->GetName() == "PAMatchRotVelocity") {
            // XXX These really need a separate target buffer.
        } else {
            kernel << "    {\n";
            kernel << "        int nparticles = PG.GetList().size();\n";
            kernel << "        int nthreads = nparticles;\n";
            if(Params & P_CUDA_CODE) {
                kernel << "        int ngrids = (nthreads + block_size - 1) / block_size;\n";
                kernel << "        " << KernelName << "<<<ngrids, block_size>>>(PG.GetDevicePtr(), dt, nparticles);\n";
            } else {
                kernel << "        Particle_t *PartP = nthreads ? &(PG.GetList()[0]) : NULL;\n";
                kernel << "        for(int i=0; i<nthreads; i++)\n";
                kernel << "            " << KernelName << "(PartP, i, VD, dt, nparticles);\n";
            }

            // Scan the list to see if any actions kill particles
            bool SegmentHasKillers = false;
            for(ActionList::iterator it = abeg; it != aend; it++) {
                SegmentHasKillers = SegmentHasKillers || (*it)->GetKillsParticles();
            }

            if(SegmentHasKillers) {
                // XXX Emit code to fill in bubbles made by killed particles and to get the number of particles killed from the device.
                kernel << "        // Handle killed particles\n";
                kernel << "        CullKilled(PG);\n";
            }

            kernel << "    }\n";
        }

        return kernel.str();
    }

    // Create the string representing the CUDA kernel for a segment of an action list
    string EmitActionListKernel(const ActionList &AList, const ActionList::iterator abeg, const ActionList::iterator aend,
        const string &KernelName, int &var_cnt, const EmitCodeParams_e Params)
    {
        ostringstream kernel;

        // This is the preamble of the kernel
        if(Params & P_CUDA_CODE) {
            kernel << "__global__ void " << KernelName << "(Particle_t *P, const float dt, const int n_particles)\n{\n";
            kernel << "    int t = blockIdx.x * blockDim.x + threadIdx.x;\n";
            kernel << "    if(t >= n_particles) return;\n\n";
        } else {
            kernel << "static inline void " << KernelName << "(Particle_t *P, const int t, const Dword_t *VD, const float dt, const int n_particles)\n{\n";
        }

        if((*abeg)->GetName() == "PASource") {
            PASSERT(abeg + 1 == aend, "Only one PAsource per group");
            PASource *PAS = dynamic_cast<PASource*>(*abeg);

            kernel << "    Particle_t &m = P[n_particles + t];\n\n";

            kernel << (*abeg)->EmitCall("", var_cnt, false) << endl;
        } else if((*abeg)->GetName() == "PASort") {
        } else if((*abeg)->GetName() == "PAFollow" || (*abeg)->GetName() == "PAGravitate" || (*abeg)->GetName() == "PAMatchVelocity" || (*abeg)->GetName() == "PAMatchRotVelocity") {
            // XXX These really need a separate target buffer.
            kernel << "    // " << (*abeg)->GetName() << " not implemented yet.\n";
        } else {
            kernel << "    Particle_t &m = P[t];\n\n";

            int ac=0;
            bool SegmentHasKillers = false;
            for(ActionList::iterator it = abeg; it != aend; it++, ac++) {
                SegmentHasKillers = SegmentHasKillers || (*it)->GetKillsParticles();

                kernel << (*it)->EmitCall("", var_cnt, false) << endl;
            }

            if(SegmentHasKillers) {
                // XXX Emit code to fill in bubbles made by killed particles and to return the number of particles killed to the host.
            }
        }

        kernel << "}\n\n";

        return kernel.str();
    }

    // Create the string representing the CUDA kernel for an action list
    string EmitActionListFile(ActionList &AList, const string &KernelName, const EmitCodeParams_e Params)
    {
        int var_cnt = 0;
        ostringstream kernel, caller;

        caller << EmitActionListCaller(AList, KernelName, Params);

        // Emit the callback functions, which are just pointers to strings.
        for(ActionList::iterator it = AList.begin(); it != AList.end(); it++) {
            if(PACallback *PACB = dynamic_cast<PACallback *>(*it)) {
                PASSERT(PACB->callbackStr == "", "Empty callback source code string");
                kernel << endl << ((Params & P_CUDA_CODE) ? "__device__ inline " : "static inline ") << PACB->callbackStr << endl;
            }
        }

        // Loop over the actions, emitting kernels for them, combining the ones that can be combined.
        ActionList::iterator it = AList.begin();
        while(it != AList.end()) {
            // Make an action segment
            ActionList::iterator abeg = it;
            ActionList::iterator aend = it+1;

            // If the first one is connectable, try to connect some more.
            bool SegmentHasKillers = false;
            while(!(*abeg)->GetDoNotSegment()
                && aend != AList.end()
                && !(*aend)->GetDoNotSegment()
                // Killers can only reside at the end of a segment, but there can be more than one of them.
                && !(SegmentHasKillers && !(*aend)->GetKillsParticles())) {
                    SegmentHasKillers = SegmentHasKillers || (*aend)->GetKillsParticles();
                    aend++;
            }

            // Found a sub-list that can be in one kernel together. Now emit them.
            int KCall = (int)(it - AList.begin());
            string KName = CatInt(KernelName, KCall);
            kernel << EmitActionListKernel(AList, abeg, aend, KName, var_cnt, Params);

            caller << EmitHostCaller(AList, abeg, aend, (int)(it - AList.begin()), KName, var_cnt, Params);
            it = aend;
        }

        caller << "}\n\n";

        caller << ((Params & P_CUDA_CODE) ? "__host__ " : "");
        caller << "extern void " << KernelName << "(const void *A, void *P, const float dt, const int block_size); // GREP0\n";
        caller << "//Efx.Effects[]->BindEmitted(Efx, " << KernelName << ", BIND_KIND); // GREP1\n";

        caller << "void " << KernelName << "(const void *A, void *P, const float dt, const int block_size)\n{\n";
        caller << "    const ActionList &AList = *(ActionList *)A;\n";
        caller << "    ParticleGroup &PG = *(ParticleGroup *)P;\n";
        caller << "    " << KernelName << "Intl(AList, PG, dt, block_size);\n";
        caller << "}\n\n";

        // This is the preamble of the file.
        // It gets added last so that we know how many varying values there are.
        string prestr = "// THIS IS A GENERATED FILE, created by calling EmitActionList(AList, \"" + KernelName + "\", " + CatInt("", (int)Params) + ").\n\n"
            "#include \"CUDAActions.cuh\"\n"
            "#include \"CUDADomains.cuh\"\n"
            "#include \"PInternalState.h\"\n"
            "using namespace PAPI;\n\n";

        if(Params & P_CUDA_CODE) {
            prestr += "__constant__ Dword_t VD[" + CatInt("", var_cnt) + "];\n\n";
        }

        prestr += string((Params & P_CUDA_CODE) ? "__host__ inline " : "static inline ") +
            "void CullKilled(ParticleGroup &PG)\n"
            "{\n"
            "    // Must traverse list carefully so Remove will work\n"
            "    for (ParticleList::iterator it = PG.begin(); it != PG.end(); ) {\n"
            "        Particle_t &m = (*it);\n"
            "        // tmp0 == 0 means the particle is alive. Non-zero means it's dead.\n"
            "        if(m.tmp0)\n"
            "            it = PG.Remove(it);\n"
            "        else\n"
            "            it++;\n"
            "    }\n"
            "}\n\n";

        return prestr + kernel.str() + caller.str();
    }

    // This virtual function of each derived action class emits the kernel code to call the _Impl function.

    string PAAvoid::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(position, nm)
                << EMIT(look_ahead, nm)
                << EMIT(magnitude, nm)
                << EMIT(epsilon, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(position, nm)
                << ", " << EMIT(look_ahead, nm)
                << ", " << EMIT(magnitude, nm)
                << ", " << EMIT(epsilon, nm) << ");\n";
        }

        return S.str();
    }

    string PABounce::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(position, nm)
                << EMIT(oneMinusFriction, nm)
                << EMIT(resilience, nm)
                << EMIT(cutoffSqr, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(position, nm)
                << ", " << EMIT(oneMinusFriction, nm)
                << ", " << EMIT(resilience, nm)
                << ", " << EMIT(cutoffSqr, nm) << ");\n";
        }

        return S.str();
    }

    string PACallback::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;

        // Get the callback function's name from the string.
        PASSERT(callbackStr.substr(0, 5) == "void ", "Malformed callback string: String must begin with \"void FuncName(Particle_t & ...");
        size_t fname_start = callbackStr.find_first_not_of(" \t\n\r", 4);
        size_t fname_end = callbackStr.find_first_of("( \t\n\r", fname_start);
        string FuncName = callbackStr.substr(fname_start, fname_end - fname_start);

        cerr << "Found callback named '" << FuncName << "'.\n";

        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << "    " << FuncName << "(m, "
                << EMIT(Data, nm)
                << ", dt);\n";
        } else {
            string nm = name_in + name;
            S << "    " << FuncName << "(m, "
                << ", " << EMIT(Data, nm)
                << ", dt);\n";
        }

        return S.str();
    }

    string PACallActionList::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << "// NOT IMPLEMENTED:";
              // Could implement this by deriving the kernel name from the action_list_num and calling it.
                //<< ", " << EMIT(action_list_num, nm)
        } else {
            string nm = name_in + name;
            S << "// NOT IMPLEMENTED:";
            S << "    " << name << "_Impl(m, dt"
                // Could implement this by deriving the kernel name from the action_list_num and calling it.
                //<< ", " << EMIT(action_list_num, nm)
                << ");\n";
        }

        return S.str();
    }

    string PACopyVertexB::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(copy_pos, nm)
                << EMIT(copy_vel, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(copy_pos, nm)
                << ", " << EMIT(copy_vel, nm) << ");\n";
        }

        return S.str();
    }

    string PADamping::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(damping, nm)
                << EMIT(vlowSqr, nm)
                << EMIT(vhighSqr, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(damping, nm)
                << ", " << EMIT(vlowSqr, nm)
                << ", " << EMIT(vhighSqr, nm) << ");\n";
        }

        return S.str();
    }

    string PARotDamping::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(damping, nm)
                << EMIT(vlowSqr, nm)
                << EMIT(vhighSqr, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(damping, nm)
                << ", " << EMIT(vlowSqr, nm)
                << ", " << EMIT(vhighSqr, nm) << ");\n";
        }

        return S.str();
    }

    string PAExplosion::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(center, nm)
                << EMIT(radius, nm)
                << EMIT(magnitude, nm)
                << EMIT(stdev, nm)
                << EMIT(epsilon, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(center, nm)
                << ", " << EMIT(radius, nm)
                << ", " << EMIT(magnitude, nm)
                << ", " << EMIT(stdev, nm)
                << ", " << EMIT(epsilon, nm) << ");\n";
        }

        return S.str();
    }

    string PAFollow::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(magnitude, nm)
                << EMIT(epsilon, nm)
                << EMIT(max_radius, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(magnitude, nm)
                << ", " << EMIT(epsilon, nm)
                << ", " << EMIT(max_radius, nm) << ");\n";
        }

        return S.str();
    }

    string PAGravitate::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(magnitude, nm)
                << EMIT(epsilon, nm)
                << EMIT(max_radius, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(magnitude, nm)
                << ", " << EMIT(epsilon, nm)
                << ", " << EMIT(max_radius, nm) << ");\n";
        }

        return S.str();
    }

    string PAGravity::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(direction, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(direction, nm) << ");\n";
        }

        return S.str();
    }

    string PAJet::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(dom, nm)
                << EMIT(acc, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(dom, nm)
                << ", " << EMIT(acc, nm) << ");\n";
        }

        return S.str();
    }

    string PAKillOld::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(age_limit, nm)
                << EMIT(kill_less_than, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(age_limit, nm)
                << ", " << EMIT(kill_less_than, nm) << ");\n";
        }

        return S.str();
    }

    string PAMatchVelocity::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(magnitude, nm)
                << EMIT(epsilon, nm)
                << EMIT(max_radius, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(magnitude, nm)
                << ", " << EMIT(epsilon, nm)
                << ", " << EMIT(max_radius, nm) << ");\n";
        }

        return S.str();
    }

    string PAMatchRotVelocity::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(magnitude, nm)
                << EMIT(epsilon, nm)
                << EMIT(max_radius, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(magnitude, nm)
                << ", " << EMIT(epsilon, nm)
                << ", " << EMIT(max_radius, nm) << ");\n";
        }

        return S.str();
    }

    string PAMove::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(move_velocity, nm)
                << EMIT(move_rotational_velocity, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(move_velocity, nm)
                << ", " << EMIT(move_rotational_velocity, nm) << ");\n";
        }

        return S.str();
    }

    string PAOrbitLine::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(p, nm)
                << EMIT(axis, nm)
                << EMIT(magnitude, nm)
                << EMIT(epsilon, nm)
                << EMIT(max_radius, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(p, nm)
                << ", " << EMIT(axis, nm)
                << ", " << EMIT(magnitude, nm)
                << ", " << EMIT(epsilon, nm)
                << ", " << EMIT(max_radius, nm) << ");\n";
        }

        return S.str();
    }

    string PAOrbitPoint::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(center, nm)
                << EMIT(magnitude, nm)
                << EMIT(epsilon, nm)
                << EMIT(max_radius, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(center, nm)
                << ", " << EMIT(magnitude, nm)
                << ", " << EMIT(epsilon, nm)
                << ", " << EMIT(max_radius, nm) << ");\n";
        }

        return S.str();
    }

    string PARandomAccel::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(gen_acc, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(gen_acc, nm) << ");\n";
        }

        return S.str();
    }

    string PARandomDisplace::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(gen_disp, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(gen_disp, nm) << ");\n";
        }

        return S.str();
    }

    string PARandomVelocity::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(gen_vel, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(gen_vel, nm) << ");\n";
        }

        return S.str();
    }

    string PARandomRotVelocity::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(gen_vel, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(gen_vel, nm) << ");\n";
        }

        return S.str();
    }

    string PARestore::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(time_left, nm)
                << EMIT(restore_velocity, nm)
                << EMIT(restore_rvelocity, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(time_left, nm)
                << ", " << EMIT(restore_velocity, nm)
                << ", " << EMIT(restore_rvelocity, nm) << ");\n";
        }

        return S.str();
    }

    string PASink::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(kill_inside, nm)
                << EMIT(position, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(kill_inside, nm)
                << ", " << EMIT(position, nm) << ");\n";
        }

        return S.str();
    }

    string PASinkVelocity::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(kill_inside, nm)
                << EMIT(velocity, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(kill_inside, nm)
                << ", " << EMIT(velocity, nm) << ");\n";
        }

        return S.str();
    }

    string PASort::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(Eye, nm)
                << EMIT(Look, nm)
                << EMIT(front_to_back, nm)
                << EMIT(clamp_negative, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(Eye, nm)
                << ", " << EMIT(Look, nm)
                << ", " << EMIT(front_to_back, nm)
                << ", " << EMIT(clamp_negative, nm) << ");\n";
        }

        return S.str();
    }

    string PASource::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(SrcSt, nm);
            S << EMIT(position, nm);
            S << EMIT(particle_rate, nm);
        } else {
            string nm = name_in + name;
            S << EMIT(SrcSt, nm) << endl;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(position, nm)
                << ", SrcStt"
                << ");\n";
            EMIT(particle_rate, nm); // Just to properly count if it's VARYING.
        }

        return S.str();
    }

    string PASpeedLimit::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(min_speed, nm)
                << EMIT(max_speed, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(min_speed, nm)
                << ", " << EMIT(max_speed, nm) << ");\n";
        }

        return S.str();
    }

    string PATargetColor::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(color, nm)
                << EMIT(alpha, nm)
                << EMIT(scale, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(color, nm)
                << ", " << EMIT(alpha, nm)
                << ", " << EMIT(scale, nm) << ");\n";
        }

        return S.str();
    }

    string PATargetSize::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(size, nm)
                << EMIT(scale, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(size, nm)
                << ", " << EMIT(scale, nm) << ");\n";
        }

        return S.str();
    }

    string PATargetVelocity::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(velocity, nm)
                << EMIT(scale, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(velocity, nm)
                << ", " << EMIT(scale, nm) << ");\n";
        }

        return S.str();
    }

    string PATargetRotVelocity::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(velocity, nm)
                << EMIT(scale, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(velocity, nm)
                << ", " << EMIT(scale, nm) << ");\n";
        }

        return S.str();
    }

    string PAVortex::EmitCall(const string &name_in, int &var_cnt, bool call)
    {
        ostringstream S;
        if(call) {
            string nm = "(*dynamic_cast<" + name + " *>(" + name_in + "))";
            S << EMIT(tip, nm)
                << EMIT(axis, nm)
                << EMIT(tightnessExponent, nm)
                << EMIT(max_radius, nm)
                << EMIT(inSpeed, nm)
                << EMIT(upSpeed, nm)
                << EMIT(aroundSpeed, nm);
        } else {
            string nm = name_in + name;
            S << "    " << name << "_Impl(m, dt"
                << ", " << EMIT(tip, nm)
                << ", " << EMIT(axis, nm)
                << ", " << EMIT(tightnessExponent, nm)
                << ", " << EMIT(max_radius, nm)
                << ", " << EMIT(inSpeed, nm)
                << ", " << EMIT(upSpeed, nm)
                << ", " << EMIT(aroundSpeed, nm) << ");\n";
        }

        return S.str();
    }

};
