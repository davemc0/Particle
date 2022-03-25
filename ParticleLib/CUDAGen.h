// This file contains the code to emit the CUDA kernel for the action list.

#ifndef CUDAGen_h
#define CUDAGen_h

#include "PInternalState.h"

#include <string>

namespace PAPI {

    // Create the string representing the CUDA kernel for an action list
    std::string EmitActionListFile(PAPI::ActionList &AList, const std::string &KernelName, const EmitCodeParams_e Params);

};

#endif
