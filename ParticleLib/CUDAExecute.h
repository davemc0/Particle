// This file contains the wrapper code to call generated action list kernels.
// It also contains the device-side memory management.
// ALL CUDA runtime calls must be in this file so that it's easy to disable CUDA support in the API.

#include "pVec.h"
